`timescale 1ns/1ps

module tb_piecewise;

    // Parameters
    localparam int WID = 16;
    localparam int FBITS = 8;
    localparam bit SATURATE = 1;
    localparam time CLK_PERIOD = 10ns;  // 100 MHz clock

    // DUT signals
    logic clk;
    logic signed [WID-1:0] x;
    logic signed [WID-1:0] a0, a1, a2, a3;
    logic signed [WID-1:0] y;

    // Instantiate DUT
    piecewise_fixed #(
        .WID(WID),
        .FBITS(FBITS),
        .SATURATE(SATURATE)
    ) dut (
        .clk(clk),
        .x(x),
        .a0(a0),
        .a1(a1),
        .a2(a2),
        .a3(a3),
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
    logic signed [WID-1:0] aint0, aint1, aint2, aint3;
    real y_float;
    logic signed [WID-1:0] yint_expected;
    logic signed [WID-1:0] yint_trunc_expected;
    real yhat_trunc, yhat_sat;
    
    // Statistics
    integer num_passed, num_failed;
    integer num_tests;

    initial begin
        // Initialize
        x = 0;
        a0 = 0;
        a1 = 0;
        a2 = 0;
        a3 = 0;
        num_passed = 0;
        num_failed = 0;
        num_tests = 0;

        // Construct filenames
        // Note that we use relative path assuming running from demos/fixp/sim
        fn = $sformatf("../test_vectors/tv_w%0d_f%0d.csv", WID, FBITS);
        fn_out = $sformatf("../test_vectors/tv_w%0d_f%0d_%s_sv.csv", WID, FBITS, SATURATE ? "sat" : "trunc");

        // Open input CSV file
        file_handle = $fopen(fn, "r");
        if (file_handle == 0) begin
            $display("ERROR: Could not open file %s", fn);
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

        $display("=== Piecewise Fixed Point Testbench ===");
        $display("Parameters: WID=%0d, FBITS=%0d, SATURATE=%0d", WID, FBITS, SATURATE);
        $display("Reading test vectors from: %s", fn);
        $display("Writing results to: %s", fn_out);

        // Read and skip header line from input
        scan_result = $fgets(header_line, file_handle);
        $display("CSV Header: %s", header_line);
        
        // Write header to output file
        $fdisplay(out_file_handle, "xint,aint0,aint1,aint2,aint3,y,yint_trunc,yint_sat,yhat_trunc,yhat_sat,y_dut");

        // Wait for a few clock cycles before starting
        repeat (3) @(posedge clk);

        line_num = 0;
        
        // Read test vectors from CSV file
        while (!$feof(file_handle)) begin
            // Read CSV line: xint,aint0,aint1,aint2,aint3,y,yint_trunc,yint_sat,yhat_trunc,yhat_sat
            scan_result = $fscanf(file_handle, "%d,%d,%d,%d,%d,%f,%d,%d,%f,%f\n", 
                                  xint, aint0, aint1, aint2, aint3,
                                  y_float, yint_trunc_expected, yint_expected,
                                  yhat_trunc, yhat_sat);

            if (scan_result != 10) begin
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
            a3 = aint3;
            
            // Wait for output (module has 1 clock cycle latency due to input registers)
            @(posedge clk);
            @(posedge clk);  // Wait one more cycle for output
            
            // Check result based on SATURATE setting
            if (SATURATE) begin
                // Compare with saturation result
                if (y === yint_expected) begin
                    num_passed++;
                    if (line_num <= 10) begin  // Print first 10 tests
                        $display("Test %0d: PASS - x=%0d, y=%0d (expected=%0d)", 
                                line_num, xint, y, yint_expected);
                    end
                end else begin
                    num_failed++;
                    $display("Test %0d: FAIL - x=%0d, a=[%0d,%0d,%0d,%0d], y=%0d (expected=%0d)", 
                            line_num, xint, aint0, aint1, aint2, aint3, y, yint_expected);
                end
            end else begin
                // Compare with truncation result
                if (y === yint_trunc_expected) begin
                    num_passed++;
                    if (line_num <= 10) begin
                        $display("Test %0d: PASS - x=%0d, y=%0d (expected=%0d)", 
                                line_num, xint, y, yint_trunc_expected);
                    end
                end else begin
                    num_failed++;
                    $display("Test %0d: FAIL - x=%0d, a=[%0d,%0d,%0d,%0d], y=%0d (expected=%0d)", 
                            line_num, xint, aint0, aint1, aint2, aint3, y, yint_trunc_expected);
                end
            end
            
            // Write result to output CSV file
            // Format: xint,aint0,aint1,aint2,aint3,y,yint_trunc,yint_sat,yhat_trunc,yhat_sat,y_dut
            $fdisplay(out_file_handle, "%0d,%0d,%0d,%0d,%0d,%f,%0d,%0d,%f,%f,%0d",
                      xint, aint0, aint1, aint2, aint3,
                      y_float, yint_trunc_expected, yint_expected,
                      yhat_trunc, yhat_sat, y);
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
