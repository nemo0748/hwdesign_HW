`timescale 1ns/1ps

/*********************************************
 * tb_vac_csv – CSV-driven Testbench for Variable Arithmetic Core
 *
 * Input  CSV : test_vectors.csv
 *              Columns: A, B, C, D, x, expected_sat, expected_ovf
 *
 * Output CSV : results.csv
 *              Columns: A, B, C, D, x,
 *                       got_sat, got_ovf,
 *                       expected_sat, expected_ovf,
 *                       pass
 *
 * Each row sets the four AXI4-Lite coefficients, streams x through
 * the DUT, waits PIPE_LAT cycles, then captures and logs result_sat
 * and result_ovf.
 *
 * Pipeline latency: PIPE_LAT = 4 cycles (matches original tb_vac).
 *********************************************/

module tb_vac_csv;

    // ------------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------------
    localparam int  DATA_W     = 32;
    localparam int  ADDR_W     = 5;
    localparam time CLK_PERIOD = 10ns;
    localparam int  PIPE_LAT   = 4;

    localparam string INPUT_CSV  = "../test_vectors.csv";
    localparam string OUTPUT_CSV = "results.csv";

    // ------------------------------------------------------------------
    // DUT signals
    // ------------------------------------------------------------------
    logic clk, rst;

    // AXI4-Lite
    logic [ADDR_W-1:0]  s_axil_awaddr;
    logic               s_axil_awvalid;
    logic               s_axil_awready;
    logic [DATA_W-1:0]  s_axil_wdata;
    logic               s_axil_wvalid;
    logic               s_axil_wready;
    logic [1:0]         s_axil_bresp;
    logic               s_axil_bvalid;
    logic               s_axil_bready;
    logic [ADDR_W-1:0]  s_axil_araddr;
    logic               s_axil_arvalid;
    logic               s_axil_arready;
    logic [DATA_W-1:0]  s_axil_rdata;
    logic [1:0]         s_axil_rresp;
    logic               s_axil_rvalid;
    logic               s_axil_rready;

    // AXI4-Stream slave (x input)
    logic signed [31:0] s_axis_tdata;
    logic               s_axis_tvalid;
    logic               s_axis_tready;

    // AXI4-Stream master (dual output)
    logic [63:0]        m_axis_tdata;
    logic               m_axis_tvalid;
    logic               m_axis_tready;
    logic signed [31:0] result_sat;
    logic signed [31:0] result_ovf;

    // ------------------------------------------------------------------
    // DUT Instantiation
    // ------------------------------------------------------------------
    vac #(
        .DATA_W(DATA_W),
        .ADDR_W(ADDR_W)
    ) dut (
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
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .m_axis_tdata   (m_axis_tdata),
        .m_axis_tvalid  (m_axis_tvalid),
        .m_axis_tready  (m_axis_tready),
        .result_sat     (result_sat),
        .result_ovf     (result_ovf)
    );

    // ------------------------------------------------------------------
    // Clock
    // ------------------------------------------------------------------
    initial clk = 0;
    always #(CLK_PERIOD/2) clk = ~clk;

    // Always accept output
    assign m_axis_tready = 1'b1;

    // ------------------------------------------------------------------
    // AXI4-Lite write task
    // ------------------------------------------------------------------
    task automatic axil_write(
        input logic [ADDR_W-1:0] addr,
        input logic [DATA_W-1:0] data
    );
        @(posedge clk);
        s_axil_awaddr  <= addr;
        s_axil_awvalid <= 1'b1;
        s_axil_wdata   <= data;
        s_axil_wvalid  <= 1'b1;

        @(posedge clk);
        while (!s_axil_awready) @(posedge clk);
        s_axil_awvalid <= 1'b0;

        while (!s_axil_wready) @(posedge clk);
        s_axil_wvalid  <= 1'b0;

        s_axil_bready  <= 1'b1;
        while (!s_axil_bvalid) @(posedge clk);
        @(posedge clk);
        s_axil_bready  <= 1'b0;
    endtask

    // ------------------------------------------------------------------
    // Write coefficients via AXI4-Lite
    // ------------------------------------------------------------------
    task automatic write_coeffs(
        input logic signed [31:0] A,
        input logic signed [31:0] B,
        input logic signed [31:0] C,
        input logic signed [31:0] D
    );
        axil_write(5'h00, 32'(A));
        axil_write(5'h04, 32'(B));
        axil_write(5'h08, 32'(C));
        axil_write(5'h0C, 32'(D));
        repeat (2) @(posedge clk);  // settle
    endtask

    // ------------------------------------------------------------------
    // Stream a single x and capture output after pipeline
    // ------------------------------------------------------------------
    task automatic stream_x(
        input  logic signed [31:0] x_val,
        output logic signed [31:0] sat_result,
        output logic signed [31:0] ovf_result
    );
        @(posedge clk);
        s_axis_tdata  <= x_val;
        s_axis_tvalid <= 1'b1;
        @(posedge clk);
        s_axis_tvalid <= 1'b0;

        repeat (PIPE_LAT) @(posedge clk);

        sat_result = result_sat;
        ovf_result = result_ovf;
    endtask

    // ------------------------------------------------------------------
    // Main
    // ------------------------------------------------------------------
    // CSV file handles
    integer fin, fout;
    // Scanned fields from each CSV row
    integer scan_ok;
    // Per-test variables
    logic signed [31:0] tv_A, tv_B, tv_C, tv_D, tv_x;
    logic signed [31:0] tv_exp_sat, tv_exp_ovf;
    logic signed [31:0] got_sat, got_ovf;
    integer num_pass, num_fail, num_total;
    string  header_line;  // consumed but not used

    // Current coefficients (track to avoid redundant AXI writes)
    logic signed [31:0] cur_A, cur_B, cur_C, cur_D;

    initial begin
        // ---- Initialise bus ----
        rst            = 0;
        s_axil_awaddr  = '0;  s_axil_awvalid = 0;
        s_axil_wdata   = '0;  s_axil_wvalid  = 0;
        s_axil_bready  = 0;
        s_axil_araddr  = '0;  s_axil_arvalid = 0;
        s_axil_rready  = 0;
        s_axis_tdata   = '0;  s_axis_tvalid  = 0;
        num_pass  = 0;  num_fail  = 0;  num_total = 0;
        cur_A = 32'shDEAD_BEEF;  // sentinel – force first write

        // ---- Reset ----
        @(posedge clk); @(posedge clk);
        rst = 1;
        @(posedge clk);
        rst = 0;
        repeat (3) @(posedge clk);

        // ---- Open input CSV ----
        fin = $fopen(INPUT_CSV, "r");
        if (fin == 0) begin
            $fatal(1, "ERROR: Cannot open input CSV '%s'", INPUT_CSV);
        end

        // ---- Open output CSV ----
        fout = $fopen(OUTPUT_CSV, "w");
        if (fout == 0) begin
            $fatal(1, "ERROR: Cannot open output CSV '%s'", OUTPUT_CSV);
        end

        // Write output header
        $fwrite(fout, "A,B,C,D,x,got_sat,got_ovf,expected_sat,expected_ovf,pass\n");

        // Skip input header line
        scan_ok = $fgets(header_line, fin);

        $display("============================================================");
        $display("  VAC CSV Testbench  –  y = A*x^3 + B*x^2 + C*x + D");
        $display("  Input : %s", INPUT_CSV);
        $display("  Output: %s", OUTPUT_CSV);
        $display("============================================================");
        $display("  %-4s %-4s %-4s %-4s %-12s %-14s %-14s %-14s %-14s %s",
                 "A","B","C","D","x",
                 "got_sat","got_ovf","exp_sat","exp_ovf","PASS");
        $display("  %s", {110{"-"}});

        // ---- Iterate over CSV rows ----
        while (!$feof(fin)) begin
            // $fscanf returns number of items matched; 7 expected
            scan_ok = $fscanf(fin, "%d,%d,%d,%d,%d,%d,%d\n",
                              tv_A, tv_B, tv_C, tv_D, tv_x,
                              tv_exp_sat, tv_exp_ovf);
            if (scan_ok != 7) break;   // end of valid data

            num_total++;

            // Only re-write coefficients when they change
            if (tv_A !== cur_A || tv_B !== cur_B ||
                tv_C !== cur_C || tv_D !== cur_D) begin
                write_coeffs(tv_A, tv_B, tv_C, tv_D);
                cur_A = tv_A;  cur_B = tv_B;
                cur_C = tv_C;  cur_D = tv_D;
            end

            // Drive x and capture result
            stream_x(tv_x, got_sat, got_ovf);

            // Compare
            if (got_sat === tv_exp_sat && got_ovf === tv_exp_ovf) begin
                $display("  %-4d %-4d %-4d %-4d %-12d %-14d %-14d %-14d %-14d PASS",
                         tv_A, tv_B, tv_C, tv_D, tv_x,
                         got_sat, got_ovf, tv_exp_sat, tv_exp_ovf);
                $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,PASS\n",
                        tv_A, tv_B, tv_C, tv_D, tv_x,
                        got_sat, got_ovf, tv_exp_sat, tv_exp_ovf);
                num_pass++;
            end else begin
                $display("  %-4d %-4d %-4d %-4d %-12d %-14d %-14d %-14d %-14d FAIL",
                         tv_A, tv_B, tv_C, tv_D, tv_x,
                         got_sat, got_ovf, tv_exp_sat, tv_exp_ovf);
                if (got_sat !== tv_exp_sat)
                    $display("    ^ SAT mismatch: expected %0d, got %0d", tv_exp_sat, got_sat);
                if (got_ovf !== tv_exp_ovf)
                    $display("    ^ OVF mismatch: expected %0d, got %0d", tv_exp_ovf, got_ovf);
                $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,FAIL\n",
                        tv_A, tv_B, tv_C, tv_D, tv_x,
                        got_sat, got_ovf, tv_exp_sat, tv_exp_ovf);
                num_fail++;
            end
        end

        $fclose(fin);
        $fclose(fout);

        $display("============================================================");
        $display("  SUMMARY  Total=%0d  Pass=%0d  Fail=%0d", num_total, num_pass, num_fail);
        if (num_fail == 0)
            $display("  *** ALL %0d TESTS PASSED ***", num_total);
        else
            $display("  *** %0d TEST(S) FAILED ***", num_fail);
        $display("  Results written to %s", OUTPUT_CSV);
        $display("============================================================");

        repeat (5) @(posedge clk);
        $finish;
    end

endmodule
