`timescale 1ns/1ps

module tb_vac_csv;

    // Parameters
    localparam int DATA_W = 32;
    localparam int ADDR_W = 5;
    localparam time CLK_PERIOD = 10ns;
    localparam int PIPE_LAT = 4;

    // --- Signals declared to match DUT ports ---
    logic clk = 0, rst;
    logic [ADDR_W-1:0] s_axil_awaddr;
    logic s_axil_awvalid, s_axil_awready;
    logic [DATA_W-1:0] s_axil_wdata;
    logic s_axil_wvalid, s_axil_wready;
    logic [1:0] s_axil_bresp;    // <--- MISSING SIGNAL 1
    logic s_axil_bvalid;         // <--- MISSING SIGNAL 2
    logic s_axil_bready;
    
    logic signed [31:0] s_axis_tdata;
    logic s_axis_tvalid, s_axis_tready;
    
    logic [63:0] m_axis_tdata;   // Added to match common VAC outputs
    logic m_axis_tvalid, m_axis_tready;
    
    logic signed [31:0] result_sat, result_ovf;

    // Clock Generation
    always #(CLK_PERIOD/2) clk = ~clk;

    // --- DUT Instantiation (Explicit mapping to avoid .* errors) ---
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
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .result_sat     (result_sat),
        .result_ovf     (result_ovf)
        // Add m_axis ports here if your DUT requires them
    );

    // Default Ties
    assign s_axil_bready = 1'b1;
    assign m_axis_tready = 1'b1; 

    // --- NON-HANGING WRITE TASK ---
    task automatic safe_write(input [ADDR_W-1:0] addr, input [DATA_W-1:0] data);
        @(posedge clk);
        s_axil_awaddr  <= addr;
        s_axil_awvalid <= 1'b1;
        s_axil_wdata   <= data;
        s_axil_wvalid  <= 1'b1;

        fork
            begin
                // Wait for the write to be accepted
                wait (s_axil_awready && s_axil_wready);
            end
            begin
                repeat(100) @(posedge clk);
                $display("ERROR: AXI Write Timeout at addr %h", addr);
            end
        join_any
        disable fork;

        @(posedge clk);
        s_axil_awvalid <= 1'b0;
        s_axil_wvalid  <= 1'b0;
    endtask

    // --- MAIN SIMULATION ---
    integer fin, scan_ok;
    string header_line; 
    logic signed [31:0] tv_A, tv_B, tv_C, tv_D, tv_x, tv_exp_sat, tv_exp_ovf;

    initial begin
        rst = 1;
        s_axil_awvalid = 0; s_axil_wvalid = 0; s_axis_tvalid = 0;
        
        repeat(5) @(posedge clk);
        rst = 0;
        repeat(5) @(posedge clk);

        fin = $fopen("../test_vectors.csv", "r");
        if (!fin) begin
            $display("ERROR: CSV file not found!");
            $finish;
        end

        // Skip Header
        void'($fgets(header_line, fin));

        while (!$feof(fin)) begin
            scan_ok = $fscanf(fin, "%d,%d,%d,%d,%d,%d,%d\n", 
                             tv_A, tv_B, tv_C, tv_D, tv_x, tv_exp_sat, tv_exp_ovf);
            
            if (scan_ok == 7) begin
                safe_write(5'h00, tv_A);
                safe_write(5'h04, tv_B);
                safe_write(5'h08, tv_C);
                safe_write(5'h0C, tv_D);

                @(posedge clk);
                s_axis_tdata  <= tv_x;
                s_axis_tvalid <= 1'b1;
                @(posedge clk);
                s_axis_tvalid <= 1'b0;

                repeat(PIPE_LAT) @(posedge clk);

                if (result_sat !== tv_exp_sat || result_ovf !== tv_exp_ovf)
                    $display("[FAIL] x=%0d | Got: Sat=%0d Ovf=%0d | Exp: Sat=%0d Ovf=%0d", 
                             tv_x, result_sat, result_ovf, tv_exp_sat, tv_exp_ovf);
                else
                    $display("[PASS] x=%0d", tv_x);
            end
        end

        $fclose(fin);
        $finish;
    end

endmodule