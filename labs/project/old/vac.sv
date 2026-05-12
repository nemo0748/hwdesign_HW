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

    // Combinational handshake. AWREADY/WREADY are high whenever we are not
    // currently holding a response (i.e., BVALID is low). When AWVALID,
    // WVALID, AWREADY, and WREADY are all high, the write is accepted and
    // committed on the same clock edge, and BVALID is asserted.
    assign s_axil_awready = !s_axil_bvalid;
    assign s_axil_wready  = !s_axil_bvalid;

    wire write_fire = s_axil_awvalid & s_axil_awready &
                      s_axil_wvalid  & s_axil_wready;

    always_ff @(posedge clk) begin
        if (rst) begin
            reg_A        <= 32'sd0;
            reg_B        <= 32'sd0;
            reg_C        <= 32'sd0;
            reg_D        <= 32'sd0;
            s_axil_bvalid <= 1'b0;
            s_axil_bresp  <= 2'b00;
        end else begin
            // Commit the write
            if (write_fire) begin
                case (s_axil_awaddr[4:2])
                    3'd0: reg_A <= $signed(s_axil_wdata);
                    3'd1: reg_B <= $signed(s_axil_wdata);
                    3'd2: reg_C <= $signed(s_axil_wdata);
                    3'd3: reg_D <= $signed(s_axil_wdata);
                    default: ;
                endcase
                s_axil_bvalid <= 1'b1;
                s_axil_bresp  <= 2'b00;  // OKAY
            end else if (s_axil_bvalid && s_axil_bready) begin
                s_axil_bvalid <= 1'b0;
            end
        end
    end

    // Read Logic — kept structurally similar to original, but simplified.
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
 *
 * Produces x and x^2 aligned in time, two pipeline stages after ingest.
 * x^2 is kept at full 64-bit precision so downstream stages can compute
 * x^3 without intermediate truncation.
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

    // Full-width square; sign-extended multiplication.
    logic signed [63:0] x_s0_se;
    assign x_s0_se = {{32{x_s0[31]}}, x_s0};
    always_comb x2_next = x_s0_se * x_s0_se;

    always_ff @(posedge clk) begin
        if (rst) begin
            valid_s0 <= 1'b0;
            valid_s1 <= 1'b0;
            x_s0     <= 32'sd0;
            x_s1     <= 32'sd0;
            x2_s1    <= 64'sd0;
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
 *
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

    // Sign-extended 64-bit operand replicas (portable across SV simulators).
    logic signed [63:0] x_in_se, coeff_A_se, coeff_B_se, coeff_C_se;
    assign x_in_se     = {{32{x_in[31]}},     x_in};
    assign coeff_A_se  = {{32{coeff_A[31]}},  coeff_A};
    assign coeff_B_se  = {{32{coeff_B[31]}},  coeff_B};
    assign coeff_C_se  = {{32{coeff_C[31]}},  coeff_C};

    always_comb begin
        // x^3 at full 64-bit precision (no intermediate truncation).
        x3_val = x_in_se * x2_in;
        // term_A uses the full 64-bit x^3 so large cubes can saturate later.
        term_A = coeff_A_se * x3_val;
        term_B = coeff_B_se * x2_in;
        term_C = coeff_C_se * x_in_se;
        term_D = {{32{coeff_D[31]}}, coeff_D};
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
 *
 * Saturated path clamps to [INT32_MIN, INT32_MAX]; overflowed path returns
 * the low 32 bits of the sum (wrap-around behavior) for diagnostics.
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
    localparam logic signed [63:0] MAX32 = 64'sh0000_0000_7FFF_FFFF;
    localparam logic signed [63:0] MIN32 = 64'shFFFF_FFFF_8000_0000;

    logic signed [31:0] sat_comb;
    always_comb begin
        if      (sum_in > MAX32) sat_comb = 32'sh7FFF_FFFF;
        else if (sum_in < MIN32) sat_comb = 32'sh8000_0000;
        else                     sat_comb = sum_in[31:0];
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            m_axis_tvalid <= 1'b0;
            m_axis_tdata  <= '0;
            result_sat    <= 32'sd0;
            result_ovf    <= 32'sd0;
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
 *
 * Pipeline alignment of coefficients vs. x/x^2:
 *   - power_engine introduces 2 cycles of latency before x_out/x2_out.
 *   - We therefore delay the coefficients by 2 cycles (cX_q1 -> cX_q2)
 *     so each datum meets its intended coefficients in the accumulator.
 *****************************************************************************/
module vac #(
    parameter int DATA_W = 32,
    parameter int ADDR_W = 5
)(
    input  logic clk,
    input  logic rst,
    // AXI-Lite
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
    // AXI-Stream Slave
    input  logic signed [31:0] s_axis_tdata,
    input  logic               s_axis_tvalid,
    output logic               s_axis_tready,
    // AXI-Stream Master
    output logic [63:0]        m_axis_tdata,
    output logic               m_axis_tvalid,
    input  logic               m_axis_tready,
    // Direct Result Outputs
    output logic signed [31:0] result_sat,
    output logic signed [31:0] result_ovf
);

    logic signed [31:0] cA, cB, cC, cD;
    logic signed [31:0] cA_q1, cA_q2, cB_q1, cB_q2, cC_q1, cC_q2, cD_q1, cD_q2;
    logic signed [31:0] pe_x;
    logic signed [63:0] pe_x2;
    logic               pe_v, acc_v;
    logic signed [63:0] acc_sum;

    // Shift registers to delay coefficients to match x/x^2 timing.
    always_ff @(posedge clk) begin
        if (rst) begin
            cA_q1 <= 32'sd0; cA_q2 <= 32'sd0;
            cB_q1 <= 32'sd0; cB_q2 <= 32'sd0;
            cC_q1 <= 32'sd0; cC_q2 <= 32'sd0;
            cD_q1 <= 32'sd0; cD_q2 <= 32'sd0;
        end else begin
            cA_q1 <= cA; cA_q2 <= cA_q1;
            cB_q1 <= cB; cB_q2 <= cB_q1;
            cC_q1 <= cC; cC_q2 <= cC_q1;
            cD_q1 <= cD; cD_q2 <= cD_q1;
        end
    end

    axi_lite_regbank #(.DATA_W(DATA_W), .ADDR_W(ADDR_W)) u_reg (
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
        .coeff_A        (cA),
        .coeff_B        (cB),
        .coeff_C        (cC),
        .coeff_D        (cD)
    );

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
