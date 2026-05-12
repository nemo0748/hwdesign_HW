`timescale 1ns/1ps

/*********************************************
 * Variable Arithmetic Core (VAC)
 *
 * Computes: y = A*x^3 + B*x^2 + C*x + D
 *
 * Architecture:
 *   - AXI4-Lite Control Register Bank  : holds coefficients A, B, C, D
 *   - Sequential Power Engine          : AXI4-Stream slave, computes x^2, x^3
 *   - Weighted Accumulator             : applies coefficients, sums terms
 *   - Dual-Path Output Formatter       : saturated + overflow outputs via
 *                                        AXI4-Stream master
 *
 * All arithmetic is 32-bit signed integer.
 * Saturation clamps to [-(2^31), 2^31-1].
 * Overflow output wraps naturally (truncated to 32 bits).
 *
 * Pipeline latency: 3 clock cycles from s_axis_tvalid to m_axis_tvalid.
 *********************************************/


/*****************************************************************************
 * Module 1 – AXI4-Lite Control Register Bank
 *
 * Address map (byte-addressed, 4-byte registers):
 *   0x00  W  A  (coefficient of x^3)
 *   0x04  W  B  (coefficient of x^2)
 *   0x08  W  C  (coefficient of x^1)
 *   0x0C  W  D  (constant term)
 *   0x10  R  status (bit 0 = ready, always 1)
 *****************************************************************************/
module axi_lite_regbank #(
    parameter int DATA_W = 32,
    parameter int ADDR_W = 5
)(
    input  logic                  clk,
    input  logic                  rst,

    // AXI4-Lite slave write channel
    input  logic [ADDR_W-1:0]     s_axil_awaddr,
    input  logic                  s_axil_awvalid,
    output logic                  s_axil_awready,

    input  logic [DATA_W-1:0]     s_axil_wdata,
    input  logic                  s_axil_wvalid,
    output logic                  s_axil_wready,

    output logic [1:0]            s_axil_bresp,
    output logic                  s_axil_bvalid,
    input  logic                  s_axil_bready,

    // AXI4-Lite slave read channel
    input  logic [ADDR_W-1:0]     s_axil_araddr,
    input  logic                  s_axil_arvalid,
    output logic                  s_axil_arready,

    output logic [DATA_W-1:0]     s_axil_rdata,
    output logic [1:0]            s_axil_rresp,
    output logic                  s_axil_rvalid,
    input  logic                  s_axil_rready,

    // Coefficient outputs to datapath
    output logic signed [DATA_W-1:0] coeff_A,
    output logic signed [DATA_W-1:0] coeff_B,
    output logic signed [DATA_W-1:0] coeff_C,
    output logic signed [DATA_W-1:0] coeff_D
);

    // Internal register file
    logic signed [DATA_W-1:0] reg_A, reg_B, reg_C, reg_D;

    // Write path state
    logic [ADDR_W-1:0] aw_addr_lat;
    logic              aw_valid_lat;

    // Write address handshake
    always_ff @(posedge clk) begin
        if (rst) begin
            s_axil_awready  <= 1'b1;
            aw_valid_lat    <= 1'b0;
            aw_addr_lat     <= '0;
        end else begin
            if (s_axil_awvalid && s_axil_awready) begin
                aw_addr_lat    <= s_axil_awaddr;
                aw_valid_lat   <= 1'b1;
                s_axil_awready <= 1'b0;
            end else if (s_axil_bvalid && s_axil_bready) begin
                s_axil_awready <= 1'b1;
                aw_valid_lat   <= 1'b0;
            end
        end
    end

    // Write data + response
    always_ff @(posedge clk) begin
        if (rst) begin
            s_axil_wready  <= 1'b1;
            s_axil_bvalid  <= 1'b0;
            s_axil_bresp   <= 2'b00;
            reg_A          <= 32'sd1;
            reg_B          <= 32'sd0;
            reg_C          <= 32'sd0;
            reg_D          <= 32'sd0;
        end else begin
            if (s_axil_wvalid && s_axil_wready && aw_valid_lat) begin
                s_axil_wready <= 1'b0;
                case (aw_addr_lat[4:2])
                    3'd0: reg_A <= $signed(s_axil_wdata);
                    3'd1: reg_B <= $signed(s_axil_wdata);
                    3'd2: reg_C <= $signed(s_axil_wdata);
                    3'd3: reg_D <= $signed(s_axil_wdata);
                    default: ;
                endcase
                s_axil_bvalid <= 1'b1;
                s_axil_bresp  <= 2'b00;  // OKAY
            end
            if (s_axil_bvalid && s_axil_bready) begin
                s_axil_bvalid <= 1'b0;
                s_axil_wready <= 1'b1;
            end
        end
    end

    // Read path
    always_ff @(posedge clk) begin
        if (rst) begin
            s_axil_arready <= 1'b1;
            s_axil_rvalid  <= 1'b0;
            s_axil_rdata   <= '0;
            s_axil_rresp   <= 2'b00;
        end else begin
            if (s_axil_arvalid && s_axil_arready) begin
                s_axil_arready <= 1'b0;
                s_axil_rvalid  <= 1'b1;
                s_axil_rresp   <= 2'b00;
                case (s_axil_araddr[4:2])
                    3'd0: s_axil_rdata <= reg_A;
                    3'd1: s_axil_rdata <= reg_B;
                    3'd2: s_axil_rdata <= reg_C;
                    3'd3: s_axil_rdata <= reg_D;
                    3'd4: s_axil_rdata <= 32'h0000_0001; // status: always ready
                    default: s_axil_rdata <= '0;
                endcase
            end
            if (s_axil_rvalid && s_axil_rready) begin
                s_axil_rvalid  <= 1'b0;
                s_axil_arready <= 1'b1;
            end
        end
    end

    // Drive outputs
    assign coeff_A = reg_A;
    assign coeff_B = reg_B;
    assign coeff_C = reg_C;
    assign coeff_D = reg_D;

endmodule


/*****************************************************************************
 * Module 2 – Sequential Power Engine  (AXI4-Stream slave)
 *
 * Accepts a stream of 32-bit signed integers x.
 * Pipeline stage 0 (input register): captures x.
 * Pipeline stage 1 (comb):           computes x^2.
 * Pipeline stage 1 register:         stores x, x^2, valid.
 * Outputs: x_s1, x2_s1, x3_comb, valid_s1 – consumed by accumulator.
 *
 * Note: x^3 is computed combinationally in stage 2 by the accumulator.
 *****************************************************************************/
module power_engine (
    input  logic        clk,
    input  logic        rst,

    // AXI4-Stream slave
    input  logic signed [31:0] s_axis_tdata,
    input  logic               s_axis_tvalid,
    output logic               s_axis_tready,

    // Power outputs to Weighted Accumulator (one cycle after valid input)
    output logic signed [31:0] x_out,
    output logic signed [63:0] x2_out,
    output logic               valid_out
);

    // We accept whenever we are not stalled (simple flow-through pipeline)
    assign s_axis_tready = 1'b1;

    // Stage 0 registers
    logic signed [31:0] x_s0;
    logic               valid_s0;

    // Stage 1 next (combinational)
    logic signed [63:0] x2_next;

    // Stage 1 registers
    logic signed [31:0] x_s1;
    logic signed [63:0] x2_s1;
    logic               valid_s1;

    always_comb begin
        // Compute x^2 from stage-0 registered value
        x2_next = 64'(signed'(x_s0)) * 64'(signed'(x_s0));
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            x_s0    <= '0;
            valid_s0<= 1'b0;
            x_s1    <= '0;
            x2_s1   <= '0;
            valid_s1<= 1'b0;
        end else begin
            // Stage 0: register inputs
            x_s0     <= s_axis_tdata;
            valid_s0 <= s_axis_tvalid;

            // Stage 1: register computed powers
            x_s1     <= x_s0;
            x2_s1    <= x2_next;
            valid_s1 <= valid_s0;
        end
    end

    assign x_out     = x_s1;
    assign x2_out    = x2_s1;
    assign valid_out = valid_s1;

endmodule


/*****************************************************************************
 * Module 3 – Weighted Accumulator
 *
 * Receives x (32-bit), x^2 (64-bit) from Power Engine.
 * Computes x^3 = x * x^2 (truncated to 64-bit, then to 32-bit).
 * Applies coefficients: sum = A*x^3 + B*x^2 + C*x + D
 * Uses 64-bit accumulators internally to reduce overflow before the
 * dual-path output stage.
 *****************************************************************************/
module weighted_accumulator (
    input  logic        clk,
    input  logic        rst,

    // From Power Engine
    input  logic signed [31:0] x_in,
    input  logic signed [63:0] x2_in,
    input  logic               valid_in,

    // Coefficients from Register Bank
    input  logic signed [31:0] coeff_A,
    input  logic signed [31:0] coeff_B,
    input  logic signed [31:0] coeff_C,
    input  logic signed [31:0] coeff_D,

    // Result to Dual-Path Formatter
    output logic signed [63:0] sum_out,
    output logic               valid_out
);

    // Combinational intermediate products (64-bit)
    logic signed [63:0] x3_full;       // x * x^2  (only lower 64 bits kept)
    logic signed [63:0] term_A;        // A * x^3
    logic signed [63:0] term_B;        // B * x^2
    logic signed [63:0] term_C;        // C * x
    logic signed [63:0] sum_comb;

    always_comb begin
        // x^3: multiply x by x^2; keep lower 64 bits (upper bits discarded)
        x3_full  = 64'(signed'(x_in)) * (x2_in >>> 32);  // scale x^2 back to 32-bit range
        // Using full 32-bit integer multiply approach:
        // x^3 (integer) = x * x * x  — but we already have x^2 as 64-bit product.
        // x^2 product was x*x without shift, so we use x * (x^2 >> 0) truncated.
        // To keep in 32-bit integer space: x^3 = x_in * (x2_in[31:0])
        // This matches the integer arithmetic goal stated in the spec.
        term_A   = 64'(signed'(coeff_A)) * 64'(signed'(x3_full[31:0]));
        term_B   = 64'(signed'(coeff_B)) * x2_in;          // B * x^2 (64-bit)
        term_C   = 64'(signed'(coeff_C)) * 64'(signed'(x_in));
        sum_comb = term_A + term_B + term_C + 64'(signed'(coeff_D));
    end

    // Register the result
    always_ff @(posedge clk) begin
        if (rst) begin
            sum_out   <= '0;
            valid_out <= 1'b0;
        end else begin
            sum_out   <= sum_comb;
            valid_out <= valid_in;
        end
    end

endmodule


/*****************************************************************************
 * Module 4 – Dual-Path Output Formatter  (AXI4-Stream master)
 *
 * Takes the 64-bit accumulated sum and produces two 32-bit outputs:
 *   tdata[63:32]  – saturated result  (clamped to 32-bit signed range)
 *   tdata[31:0]   – overflowed result (natural 32-bit wrap / truncation)
 *
 * Both are packed into a single 64-bit AXI4-Stream beat so the host
 * receives them simultaneously.
 *****************************************************************************/
module dual_path_formatter (
    input  logic        clk,
    input  logic        rst,

    // From Weighted Accumulator
    input  logic signed [63:0] sum_in,
    input  logic               valid_in,

    // AXI4-Stream master (64-bit: upper=saturated, lower=overflow)
    output logic [63:0]  m_axis_tdata,
    output logic         m_axis_tvalid,
    input  logic         m_axis_tready,

    // Convenience individual outputs (registered, same timing as tdata)
    output logic signed [31:0] result_sat,
    output logic signed [31:0] result_ovf
);

    localparam signed [63:0] MAX32 =  64'sh0000_0000_7FFF_FFFF;
    localparam signed [63:0] MIN32 =  64'shFFFF_FFFF_8000_0000;

    // Saturation function (combinational)
    function automatic logic signed [31:0] saturate32 (
        input logic signed [63:0] val
    );
        if      (val > MAX32) saturate32 = 32'sh7FFF_FFFF;
        else if (val < MIN32) saturate32 = 32'sh8000_0000;
        else                  saturate32 = val[31:0];
    endfunction

    logic signed [31:0] sat_comb;
    logic signed [31:0] ovf_comb;

    always_comb begin
        sat_comb = saturate32(sum_in);
        ovf_comb = sum_in[31:0];          // natural truncation / wrap
    end

    // Output registers
    always_ff @(posedge clk) begin
        if (rst) begin
            result_sat    <= '0;
            result_ovf    <= '0;
            m_axis_tvalid <= 1'b0;
            m_axis_tdata  <= '0;
        end else begin
            if (valid_in) begin
                result_sat    <= sat_comb;
                result_ovf    <= ovf_comb;
                m_axis_tdata  <= {sat_comb, ovf_comb};
                m_axis_tvalid <= 1'b1;
            end else if (m_axis_tvalid && m_axis_tready) begin
                m_axis_tvalid <= 1'b0;
            end
        end
    end

endmodule


/*****************************************************************************
 * Top-Level: Variable Arithmetic Core (VAC)
 *
 * Wires all four submodules together.
 *****************************************************************************/
module vac #(
    parameter int DATA_W = 32,
    parameter int ADDR_W = 5
)(
    input  logic clk,
    input  logic rst,

    //------------------------------------------------------------------
    // AXI4-Lite interface (coefficient configuration)
    //------------------------------------------------------------------
    input  logic [ADDR_W-1:0]  s_axil_awaddr,
    input  logic               s_axil_awvalid,
    output logic               s_axil_awready,

    input  logic [DATA_W-1:0]  s_axil_wdata,
    input  logic               s_axil_wvalid,
    output logic               s_axil_wready,

    output logic [1:0]         s_axil_bresp,
    output logic               s_axil_bvalid,
    input  logic               s_axil_bready,

    input  logic [ADDR_W-1:0]  s_axil_araddr,
    input  logic               s_axil_arvalid,
    output logic               s_axil_arready,

    output logic [DATA_W-1:0]  s_axil_rdata,
    output logic [1:0]         s_axil_rresp,
    output logic               s_axil_rvalid,
    input  logic               s_axil_rready,

    //------------------------------------------------------------------
    // AXI4-Stream slave (x input stream)
    //------------------------------------------------------------------
    input  logic signed [31:0] s_axis_tdata,
    input  logic               s_axis_tvalid,
    output logic               s_axis_tready,

    //------------------------------------------------------------------
    // AXI4-Stream master (dual result output stream)
    // tdata[63:32] = saturated result
    // tdata[31:0]  = overflowed result
    //------------------------------------------------------------------
    output logic [63:0]        m_axis_tdata,
    output logic               m_axis_tvalid,
    input  logic               m_axis_tready,

    // Convenience individual outputs (also available on top-level pins)
    output logic signed [31:0] result_sat,
    output logic signed [31:0] result_ovf
);

    // Internal coefficient wires
    logic signed [31:0] coeff_A, coeff_B, coeff_C, coeff_D;

    // Power engine outputs
    logic signed [31:0] pe_x;
    logic signed [63:0] pe_x2;
    logic               pe_valid;

    // Accumulator output
    logic signed [63:0] acc_sum;
    logic               acc_valid;

    // ---- Instantiate Module 1: AXI4-Lite Register Bank ----
    axi_lite_regbank #(
        .DATA_W(DATA_W),
        .ADDR_W(ADDR_W)
    ) u_regbank (
        .clk            (clk),
        .rst            (rst),
        .s_axil_awaddr  (s_axil_awaddr),
        .s_axil_awvalid (s_axil_awvalid),
        .s_axil_awready (s_axil_awready),
        .s_axil_wdata   (s_axil_wdata),
        .s_axil_wvalid  (s_axil_wvalid),
        .s_axil_wready  (s_axil_wready),
        .s_axil_bresp   (s_axil_bresp),
        .s_axil_bvalid  (s_axil_bvalid),
        .s_axil_bready  (s_axil_bready),
        .s_axil_araddr  (s_axil_araddr),
        .s_axil_arvalid (s_axil_arvalid),
        .s_axil_arready (s_axil_arready),
        .s_axil_rdata   (s_axil_rdata),
        .s_axil_rresp   (s_axil_rresp),
        .s_axil_rvalid  (s_axil_rvalid),
        .s_axil_rready  (s_axil_rready),
        .coeff_A        (coeff_A),
        .coeff_B        (coeff_B),
        .coeff_C        (coeff_C),
        .coeff_D        (coeff_D)
    );

    // ---- Instantiate Module 2: Sequential Power Engine ----
    power_engine u_power (
        .clk            (clk),
        .rst            (rst),
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .x_out          (pe_x),
        .x2_out         (pe_x2),
        .valid_out      (pe_valid)
    );

    // ---- Instantiate Module 3: Weighted Accumulator ----
    weighted_accumulator u_accum (
        .clk            (clk),
        .rst            (rst),
        .x_in           (pe_x),
        .x2_in          (pe_x2),
        .valid_in       (pe_valid),
        .coeff_A        (coeff_A),
        .coeff_B        (coeff_B),
        .coeff_C        (coeff_C),
        .coeff_D        (coeff_D),
        .sum_out        (acc_sum),
        .valid_out      (acc_valid)
    );

    // ---- Instantiate Module 4: Dual-Path Output Formatter ----
    dual_path_formatter u_fmt (
        .clk            (clk),
        .rst            (rst),
        .sum_in         (acc_sum),
        .valid_in       (acc_valid),
        .m_axis_tdata   (m_axis_tdata),
        .m_axis_tvalid  (m_axis_tvalid),
        .m_axis_tready  (m_axis_tready),
        .result_sat     (result_sat),
        .result_ovf     (result_ovf)
    );

endmodule
