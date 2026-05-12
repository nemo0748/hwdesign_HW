`timescale 1ns/1ps

module piecewise_fixed #(
    parameter int WID      = 16,   // total bit width
    parameter int FBITS    = 8,    // fractional bits
    parameter bit SATURATE = 0     // compile-time saturation flag
)(
    input  logic                        clk,

    // Inputs in Q(WID, FBITS)
    input  logic signed [WID-1:0]       x,
    input  logic signed [WID-1:0]       a0,
    input  logic signed [WID-1:0]       a1,
    input  logic signed [WID-1:0]       a2,
    input  logic signed [WID-1:0]       a3,

    // Output in Q(WID, FBITS)
    output logic signed [WID-1:0]       y
);

    // Register inputs to align with output timing
    logic signed [WID-1:0] x_r, a0_r, a1_r, a2_r, a3_r;

    always_ff @(posedge clk) begin
        x_r  <= x;
        a0_r <= a0;
        a1_r <= a1;
        a2_r <= a2;
        a3_r <= a3;
    end

    // Set full precision width for intermediate calculations
    localparam int WID_FULL = 2*WID - FBITS + 1;
    logic signed [WID_FULL-1:0] y0_full, y1_full;
    logic signed [WID-1:0] y0, y1;

    // Max/min representable values in Q(WID, FBITS)
    localparam logic signed [WID_FULL-1:0] MAXV =  (1 <<< (WID-1)) - 1;
    localparam logic signed [WID_FULL-1:0] MINV = -(1 <<< (WID-1));

    // Compute full precision results
    always_comb begin
        y0_full = ((a0_r * x_r) >>> FBITS) + a1_r;
        y1_full = ((a2_r * x_r) >>> FBITS) + a3_r;
    end

    // Apply saturation or wrapping
    // Note that generate keyword cannot be used inside
    // always_comb, so we use a separate always_comb_block
    generate
        if (SATURATE) begin : gen_sat
            always_comb begin
                if (y0_full > MAXV)       y0 = MAXV;
                else if (y0_full < MINV)  y0 = MINV;
                else                      y0 = y0_full;

                if (y1_full > MAXV)       y1 = MAXV;
                else if (y1_full < MINV)  y1 = MINV;
                else                      y1 = y1_full;
            end
        end else begin : gen_wrap
            always_comb begin
                y0 = y0_full;
                y1 = y1_full;
            end
        end
    endgenerate

    // Final max
    always_comb begin
        y = (y0 > y1) ? y0 : y1;
    end
    
endmodule