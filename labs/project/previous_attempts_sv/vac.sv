`timescale 1ns/1ps

/*********************************************
 * Variable Arithmetic Core (VAC)
 *
 * Computes: y = A*x^3 + B*x^2 + C*x + D
 *
 * Pipeline latency: 4 clock cycles total.
 *********************************************/

/*****************************************************************************
 * Module 1 – AXI4-Lite Control Register Bank
 *****************************************************************************/
module axi_lite_regbank #(
    parameter int DATA_W = 32,
    parameter int ADDR_W = 5
)(
    input  logic                   clk,
    input  logic                   rst,

    // AXI4-Lite slave write channel
    input  logic [ADDR_W-1:0]      s_axil_awaddr,
    input  logic                   s_axil_awvalid,
    output logic                   s_axil_awready,

    input  logic [DATA_W-1:0]      s_axil_wdata,
    input  logic                   s_axil_wvalid,
    output logic                   s_axil_wready,

    output logic [1:0]             s_axil_bresp,
    output logic                   s_axil_bvalid,
    input  logic                   s_axil_bready,

    // AXI4-Lite slave read channel
    input  logic [ADDR_W-1:0]      s_axil_araddr,
    input  logic                   s_axil_arvalid,
    output logic                   s_axil_arready,

    output logic [DATA_W-1:0]      s_axil_rdata,
    output logic [1:0]             s_axil_rresp,
    output logic                   s_axil_rvalid,
    input  logic                   s_axil_rready,

    // Coefficient outputs
    output logic signed [DATA_W-1:0] coeff_A,
    output logic signed [DATA_W-1:0] coeff_B,
    output logic signed [DATA_W-1:0] coeff_C,
    output logic signed [DATA_W-1:0] coeff_D
);

    logic signed [DATA_W-1:0] reg_A, reg_B, reg_C, reg_D;
    logic [ADDR_W-1:0] aw_addr_lat;
    logic              aw_valid_lat;

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

    always_ff @(posedge clk) begin
        if (rst) begin
            s_axil_wready  <= 1'b1;
            s_axil_bvalid  <= 1'b0;
            s_axil_bresp   <= 2'b00;
            reg_A          <= 32'sd0;
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
            end
            if (s_axil_bvalid && s_axil_bready) begin
                s_axil_bvalid <= 1'b0;
                s_axil_wready <= 1'b1;
            end
        end
    end

    // Read Logic
    always_ff @(posedge clk) begin
        if (rst) begin
            s_axil_arready <= 1'b1;
            s_axil_rvalid  <= 1'b0;
            s_axil_rdata   <= '0;
        end else begin
            if (s_axil_arvalid && s_axil_arready) begin
                s_axil_arready <= 1'b0;
                s_axil_rvalid  <= 1'b1;
                case (s_axil_araddr[4:2])
                    3'd0: s_axil_rdata <= reg_A;
                    3'd1: s_axil_rdata <= reg_B;
                    3'd2: s_axil_rdata <= reg_C;
                    3'd3: s_axil_rdata <= reg_D;
                    3'd4: s_axil_rdata <= 32'h1; // Status
                    default: s_axil_rdata <= '0;
                endcase
            end
            if (s_axil_rvalid && s_axil_rready) begin
                s_axil_rvalid  <= 1'b0;
                s_axil_arready <= 1'b1;
            end
        end
    end

    assign coeff_A = reg_A;
    assign coeff_B = reg_B;
    assign coeff_C = reg_C;
    assign coeff_D = reg_D;
endmodule

/*****************************************************************************
 * Module 2 – Sequential Power Engine
 *****************************************************************************/
module power_engine (
    input  logic clk,
    input  logic rst,
    input  logic signed [31:0] s_axis_tdata,
    input  logic               s_axis_tvalid,
    output logic               s_axis_tready,
    output logic signed [31:0] x_out,
    output logic signed [63:0] x2_out,
    output logic               valid_out
);
    assign s_axis_tready = 1'b1;

    logic signed [31:0] x_s0;
    logic               valid_s0;
    logic signed [63:0] x2_next;
    logic signed [31:0] x_s1;
    logic signed [63:0] x2_s1;
    logic               valid_s1;

    always_comb x2_next = 64'(signed'(x_s0)) * 64'(signed'(x_s0));

    always_ff @(posedge clk) begin
        if (rst) begin
            valid_s0 <= 1'b0;
            valid_s1 <= 1'b0;
        end else begin
            x_s0     <= s_axis_tdata;
            valid_s0 <= s_axis_tvalid;
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
 *****************************************************************************/
module weighted_accumulator (
    input  logic clk,
    input  logic rst,
    input  logic signed [31:0] x_in,
    input  logic signed [63:0] x2_in,
    input  logic               valid_in,
    input  logic signed [31:0] coeff_A,
    input  logic signed [31:0] coeff_B,
    input  logic signed [31:0] coeff_C,
    input  logic signed [31:0] coeff_D,
    output logic signed [63:0] sum_out,
    output logic               valid_out
);
    logic signed [63:0] term_A, term_B, term_C, term_D;
    logic signed [63:0] x3_val;

    always_comb begin
        x3_val   = 64'(signed'(x_in)) * 64'(signed'(x2_in[31:0]));
        term_A   = 64'(signed'(coeff_A)) * 64'(signed'(x3_val[31:0]));
        term_B   = 64'(signed'(coeff_B)) * x2_in;
        term_C   = 64'(signed'(coeff_C)) * 64'(signed'(x_in));
        term_D   = 64'(signed'(coeff_D));
        
        // Registering logic follows in FF block
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            sum_out   <= '0;
            valid_out <= 1'b0;
        end else begin
            sum_out   <= term_A + term_B + term_C + term_D;
            valid_out <= valid_in;
        end
    end
endmodule

/*****************************************************************************
 * Module 4 – Dual-Path Output Formatter
 *****************************************************************************/
module dual_path_formatter (
    input  logic clk,
    input  logic rst,
    input  logic signed [63:0] sum_in,
    input  logic               valid_in,
    output logic [63:0]        m_axis_tdata,
    output logic               m_axis_tvalid,
    input  logic               m_axis_tready,
    output logic signed [31:0] result_sat,
    output logic signed [31:0] result_ovf
);
    localparam signed [63:0] MAX32 = 64'sh0000_0000_7FFF_FFFF;
    localparam signed [63:0] MIN32 = 64'shFFFF_FFFF_8000_0000;

    logic signed [31:0] sat_comb;
    always_comb begin
        if      (sum_in > MAX32) sat_comb = 32'sh7FFF_FFFF;
        else if (sum_in < MIN32) sat_comb = 32'sh8000_0000;
        else                     sat_comb = sum_in[31:0];
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            m_axis_tvalid <= 1'b0;
            {result_sat, result_ovf} <= '0;
        end else begin
            if (valid_in) begin
                result_sat    <= sat_comb;
                result_ovf    <= sum_in[31:0];
                m_axis_tdata  <= {sat_comb, sum_in[31:0]};
                m_axis_tvalid <= 1'b1;
            end else if (m_axis_tready) begin
                m_axis_tvalid <= 1'b0;
            end
        end
    end
endmodule

/*****************************************************************************
 * Top-Level: Variable Arithmetic Core (VAC)
 *****************************************************************************/
module vac #(
    parameter int DATA_W = 32,
    parameter int ADDR_W = 5
)(
    input  logic clk,
    input  logic rst,
    // AXI-Lite
    input  logic [ADDR_W-1:0]  s_axil_awaddr, s_axil_awvalid,
    output logic               s_axil_awready,
    input  logic [DATA_W-1:0]  s_axil_wdata, s_axil_wvalid,
    output logic               s_axil_wready,
    output logic [1:0]         s_axil_bresp,
    output logic               s_axil_bvalid,
    input  logic               s_axil_bready,
    input  logic [ADDR_W-1:0]  s_axil_araddr, s_axil_arvalid,
    output logic               s_axil_arready,
    output logic [DATA_W-1:0]  s_axil_rdata,
    output logic [1:0]         s_axil_rresp,
    output logic               s_axil_rvalid,
    input  logic               s_axil_rready,
    // AXI-Stream Slave
    input  logic signed [31:0] s_axis_tdata,
    input  logic               s_axis_tvalid,
    output logic               s_axis_tready,
    // AXI-Stream Master
    output logic [63:0]        m_axis_tdata,
    output logic               m_axis_tvalid,
    input  logic               m_axis_tready,
    // Direct Result Outputs
    output logic signed [31:0] result_sat, result_ovf
);

    logic signed [31:0] cA, cB, cC, cD;
    logic signed [31:0] cA_q1, cA_q2, cB_q1, cB_q2, cC_q1, cC_q2, cD_q1, cD_q2;
    logic signed [31:0] pe_x;
    logic signed [63:0] pe_x2;
    logic pe_v, acc_v;
    logic signed [63:0] acc_sum;

    // Shift registers to delay coefficients to match x/x^2 timing
    always_ff @(posedge clk) begin
        if (rst) begin
            {cA_q1, cA_q2, cB_q1, cB_q2, cC_q1, cC_q2, cD_q1, cD_q2} <= '0;
        end else begin
            cA_q1 <= cA; cA_q2 <= cA_q1;
            cB_q1 <= cB; cB_q2 <= cB_q1;
            cC_q1 <= cC; cC_q2 <= cC_q1;
            cD_q1 <= cD; cD_q2 <= cD_q1;
        end
    end

    axi_lite_regbank u_reg (.*, .coeff_A(cA), .coeff_B(cB), .coeff_C(cC), .coeff_D(cD));
    
    power_engine u_pe (
        .clk(clk), .rst(rst),
        .s_axis_tdata(s_axis_tdata), .s_axis_tvalid(s_axis_tvalid), .s_axis_tready(s_axis_tready),
        .x_out(pe_x), .x2_out(pe_x2), .valid_out(pe_v)
    );

    weighted_accumulator u_acc (
        .clk(clk), .rst(rst),
        .x_in(pe_x), .x2_in(pe_x2), .valid_in(pe_v),
        .coeff_A(cA_q2), .coeff_B(cB_q2), .coeff_C(cC_q2), .coeff_D(cD_q2),
        .sum_out(acc_sum), .valid_out(acc_v)
    );

    dual_path_formatter u_fmt (
        .clk(clk), .rst(rst), .sum_in(acc_sum), .valid_in(acc_v),
        .m_axis_tdata(m_axis_tdata), .m_axis_tvalid(m_axis_tvalid), .m_axis_tready(m_axis_tready),
        .result_sat(result_sat), .result_ovf(result_ovf)
    );

endmodule