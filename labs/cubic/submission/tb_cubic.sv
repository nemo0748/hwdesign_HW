`timescale 1ns/1ps

module tb_cubic;

    // Parameters
    localparam int WID = 16;
    localparam int FBITS = 8;
    localparam time CLK_PERIOD = 10ns;  // 100 MHz clock

    // DUT signals
    logic clk;
    logic rst;
    logic signed [WID-1:0] x;
    logic signed [WID-1:0] a0, a1, a2;
    logic signed [WID-1:0] y;

    // Instantiate DUT
    cubic_fixed #(
        .WID(WID),
        .FBITS(FBITS)
    ) dut (
        .clk(clk),
        .rst(rst),
        .x(x),
        .a0(a0),
        .a1(a1),
        .a2(a2),
        .y(y)
    );

    // Clock generator
    initial clk = 0;
    always #(CLK_PERIOD/2) clk = ~clk;

    // CSV file names
    string fn;
    string fn_out;
    
    // CSV reading variables
    integer file_handle;
    integer out_file_handle;
    integer scan_result;
    string header_line;
    integer line_num;
    
    // Test vector variables
    logic signed [WID-1:0] xint;
    logic signed [WID-1:0] aint0, aint1, aint2;
    real y_float;
    logic signed [WID-1:0] yint_expected;
    real yfix_expected;
    
    // Statistics
    integer num_passed, num_failed;
    integer num_tests;

    // DUT internal signals for debugging
    logic signed [WID-1:0] x_dut;

    initial begin
        // Initialize
        rst = 0;
        x = 0;
        a0 = 0;
        a1 = 0;
        a2 = 0;
        num_passed = 0;
        num_failed = 0;
        num_tests = 0;

        // Assert reset for one clock cycle
        rst = 0;
        @(posedge clk);
        @(posedge clk)
        rst = 1;
        @(posedge clk);
        rst = 0;

        // Construct filenames
        // Note that we use relative path assuming running from demos/fixp/sim
        fn = $sformatf("../test_outputs/tv_w%0d_f%0d.csv", WID, FBITS);
        fn_out = $sformatf("../test_outputs/tv_w%0d_f%0d_sv.csv", WID, FBITS);

        // Open input CSV file
        file_handle = $fopen(fn, "r");
        if (file_handle == 0) begin
            $display("ERROR: Could not open input file %s", fn);
            $display("Please run the Python notebook to generate test vectors first.");
            $finish;
        end

        // Open output CSV file
        out_file_handle = $fopen(fn_out, "w");
        if (out_file_handle == 0) begin
            $display("ERROR: Could not open output file %s", fn_out);
            $fclose(file_handle);
            $finish;
        end

        $display("=== Cubic Fixed Point Testbench ===");
        $display("Parameters: WID=%0d, FBITS=%0d", WID, FBITS);
        $display("Reading test vectors from: %s", fn);
        $display("Writing results to: %s", fn_out);

        // Read and skip header line from input
        scan_result = $fgets(header_line, file_handle);
        $display("CSV Header: %s", header_line);
        
        // Write header to output file
        $fdisplay(out_file_handle, "xint,aint0,aint1,aint2,y,yint,yfix,y_dut");

        // Wait for a few clock cycles before starting
        repeat (3) @(posedge clk);

        line_num = 0;
        
        // Read test vectors from CSV file
        while (!$feof(file_handle)) begin
            // Read CSV line: xint,aint0,aint1,aint2,y,yint,yfix
            scan_result = $fscanf(file_handle, "%d,%d,%d,%d,%f,%d,%f\n", 
                                  xint, aint0, aint1, aint2, y_float, yint_expected, yfix_expected);

            if (scan_result != 7) begin
                // End of file or incomplete line
                break;
            end
            
            line_num++;
            num_tests++;
            
            // Drive inputs
            #(0.1*CLK_PERIOD);  // Small delay before changing inputs
            x  = xint;
            a0 = aint0;
            a1 = aint1;
            a2 = aint2;
            
            // Wait for output 
            repeat (3) @(posedge clk); 
 
            // Check result - compare DUT output with expected fixed-point output
            if (y == yint_expected) begin
                num_passed++;
            end else begin
                num_failed++;
                $display("Line %0d FAILED: x=%0d, a0=%0d, a1=%0d, a2=%0d", 
                         line_num, xint, aint0, aint1, aint2);
                $display("  Expected: %0d (%.6f), Got: %0d", 
                         yint_expected, yfix_expected, y);
            end
            
            // Write result to output CSV file
            // Include all input values, expected outputs, and DUT output
            $fdisplay(out_file_handle, "%0d,%0d,%0d,%0d,%f,%0d,%f,%0d",
                     xint, aint0, aint1, aint2, y_float, yint_expected, yfix_expected, y);
        end

        $fclose(file_handle);
        $fclose(out_file_handle);

        // Print summary
        $display("\n=== Test Summary ===");
        $display("Total tests: %0d", num_tests);
        $display("Passed: %0d", num_passed);
        $display("Failed: %0d", num_failed);
        if (num_failed == 0) begin
            $display("*** ALL TESTS PASSED ***");
        end else begin
            $display("*** SOME TESTS FAILED ***");
        end
        $display("Test results stored in %s", fn_out);

        // Wait a few cycles and finish
        repeat (5) @(posedge clk);
        $finish;
    end

endmodule