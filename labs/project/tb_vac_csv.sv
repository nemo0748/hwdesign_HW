`timescale 1ns/1ps

module tb_vac_csv;

    // Parameters
    localparam int DATA_W = 32;
    localparam int ADDR_W = 5;
    localparam time CLK_PERIOD = 10ns;
    localparam int PIPE_LAT = 4;

    // Signals
    logic clk = 0, rst;
    logic [ADDR_W-1:0] s_axil_awaddr;
    logic s_axil_awvalid, s_axil_awready;
    logic [DATA_W-1:0] s_axil_wdata;
    logic s_axil_wvalid, s_axil_wready;
    logic [1:0] s_axil_bresp; 
    logic s_axil_bvalid, s_axil_bready;
    
    logic signed [31:0] s_axis_tdata;
    logic s_axis_tvalid, s_axis_tready;
    logic signed [31:0] result_sat, result_ovf;

    // Clock
    always #(CLK_PERIOD/2) clk = ~clk;

    // DUT Instantiation
    vac #(.DATA_W(DATA_W), .ADDR_W(ADDR_W)) dut (
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
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .result_sat     (result_sat),
        .result_ovf     (result_ovf)
    );

    assign s_axil_bready = 1'b1;

    // Non-hanging AXI Write — flattened (no fork/join_any) for portability across
    // simulators that don't fully support `disable fork` inside a join_any.
    integer write_timeout;
    task automatic safe_write(input [ADDR_W-1:0] addr, input [DATA_W-1:0] data);
        @(posedge clk);
        s_axil_awaddr  <= addr; s_axil_awvalid <= 1'b1;
        s_axil_wdata   <= data; s_axil_wvalid  <= 1'b1;
        // Wait until the slave samples both VALIDs while READY is asserted.
        // With the fixed regbank, AWREADY/WREADY drop only after a successful
        // accept (because BVALID then rises), so a single posedge after the
        // ready check guarantees the write has committed.
        write_timeout = 0;
        while (!(s_axil_awready && s_axil_wready) && write_timeout < 100) begin
            @(posedge clk);
            write_timeout = write_timeout + 1;
        end
        if (write_timeout >= 100) $display("ERROR: AXI Write Timeout at %h", addr);
        @(posedge clk);
        s_axil_awvalid <= 1'b0; s_axil_wvalid  <= 1'b0;
    endtask

    // File handles and variables
    integer fin, fout, scan_ok;
    reg [8*256-1:0] header_line;
    logic signed [31:0] tv_A, tv_B, tv_C, tv_D, tv_x, tv_exp_sat, tv_exp_ovf;

    initial begin
        // Reset sequence
        rst = 1; repeat(5) @(posedge clk); rst = 0; repeat(5) @(posedge clk);

        // Open Files
        fin  = $fopen("../test_vectors.csv", "r");
        fout = $fopen("results.csv", "w");

        if (!fin)  $fatal(1, "Could not open input CSV");
        if (!fout) $fatal(1, "Could not create output CSV");

        // Skip input header and write output header
        scan_ok = $fgets(header_line, fin);
        $fwrite(fout, "A,B,C,D,x,got_sat,got_ovf,exp_sat,exp_ovf,status\n");

        while (!$feof(fin)) begin
            scan_ok = $fscanf(fin, "%d,%d,%d,%d,%d,%d,%d\n", 
                             tv_A, tv_B, tv_C, tv_D, tv_x, tv_exp_sat, tv_exp_ovf);
            
            if (scan_ok == 7) begin
                // 1. Write coefficients
                safe_write(5'h00, tv_A); 
                safe_write(5'h04, tv_B);
                safe_write(5'h08, tv_C); 
                safe_write(5'h0C, tv_D);

                // 2. NEW: Wait for AXI writes to fully settle and clear the pipe
                repeat(10) @(posedge clk); 

                // 3. Send X
                @(posedge clk);
                s_axis_tdata  <= tv_x;
                s_axis_tvalid <= 1'b1;
                @(posedge clk);
                s_axis_tvalid <= 1'b0;

                // 4. Wait for the specific pipeline latency
                repeat(PIPE_LAT) @(posedge clk);

                // Write results to CSV
                if (result_sat === tv_exp_sat && result_ovf === tv_exp_ovf) begin
                    $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,PASS\n",
                            tv_A, tv_B, tv_C, tv_D, tv_x, result_sat, result_ovf, tv_exp_sat, tv_exp_ovf);
                end else begin
                    $fwrite(fout, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,FAIL\n",
                            tv_A, tv_B, tv_C, tv_D, tv_x, result_sat, result_ovf, tv_exp_sat, tv_exp_ovf);
                end
            end
        end

        $fclose(fin);
        $fclose(fout);
        $display("Done! Results saved to results.csv");
        $finish;
    end
endmodule