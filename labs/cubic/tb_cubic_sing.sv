`timescale 1ns/1ps

module tb_cubic_sing;

    // Parameters
    localparam int WID = 16;
    localparam int FBITS = 8;
    localparam time CLK_PERIOD = 10ns;  // 100 MHz clock

    // Test value parameters (real)
    localparam real xr_test  = 2.5;
    localparam real a0r_test = 1.0;
    localparam real a1r_test = -0.5;
    localparam real a2r_test = 0.25;
    
    // Test value parameters (fixed-point integer)
    localparam int x_test  = int'(xr_test * (1 << FBITS));
    localparam int a0_test = int'(a0r_test * (1 << FBITS));
    localparam int a1_test = int'(a1r_test * (1 << FBITS));
    localparam int a2_test = int'(a2r_test * (1 << FBITS));
    
    // TODO
    
    localparam int x2_long = (64'(x_test) * x_test) >>> FBITS;
    localparam int x2_test = (x2_long > 32767) ? 32767 : (x2_long < -32768) ? -32768 : x2_long;
    localparam int x3_long = (64'(x_test) * x2_test) >>> FBITS;
    localparam int x3_test = (x3_long > 32767) ? 32767 : (x3_long < -32768) ? -32768 : x3_long;
    localparam int a2x2_long = (64'(a2_test) * x2_test) >>> FBITS;
    localparam int a2x2_test = (a2x2_long > 32767) ? 32767 : (a2x2_long < -32768) ? -32768 : a2x2_long;
    localparam int a1x1_long = (64'(a1_test) * x_test) >>> FBITS;
    localparam int a1x1_test = (a1x1_long > 32767) ? 32767 : (a1x1_long < -32768) ? -32768 : a1x1_long;
    localparam int sum1 = a0_test + a1x1_test;
    localparam int sat1 = (sum1 > 32767) ? 32767 : (sum1 < -32768) ? -32768 : sum1;
    localparam int sum2 = sat1 + a2x2_test;
    localparam int sat2 = (sum2 > 32767) ? 32767 : (sum2 < -32768) ? -32768 : sum2;
    localparam int sum3 = sat2 + x3_test;
    localparam int y_test = (sum3 > 32767) ? 32767 : (sum3 < -32768) ? -32768 : sum3;

    //yr_test
    localparam real yr_test = xr_test**3 + a2r_test*xr_test**2 + a1r_test*xr_test + a0r_test;

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

    // Main test stimulus
    initial begin
        // Initialize
        rst = 0;
        x = 0;
        a0 = 0;
        a1 = 0;
        a2 = 0;

        $display("=== Cubic Fixed Point Testbench ===");
        $display("Parameters: WID=%0d, FBITS=%0d", WID, FBITS);
        $display("\nTest Inputs:");
        $display("  x  = %0d (%.4f)", x, xr_test);
        $display("  a0 = %0d (%.4f)", a0, a0r_test);
        $display("  a1 = %0d (%.4f)", a1, a1r_test);
        $display("  a2 = %0d (%.4f)", a2, a2r_test);
        $display("\nExpected output:");
        $display("  y  = %0d (%.4f)", y_test, yr_test);

        // Assert reset for two clock cycles
        rst = 1;
        @(posedge clk);
        @(posedge clk);
        rst = 0;

        // Wait one cycle after reset
        @(posedge clk);

        // Set test inputs using localparams
        x  = x_test;
        a0 = a0_test;
        a1 = a1_test;
        a2 = a2_test;

        // Wait for pipeline to complete (3 stages)
        repeat (5) @(posedge clk);

    end

    // Monitor block - runs in parallel, prints internal signals each cycle
    initial begin
        integer cycle;
        
        $display("\n=== Monitoring Internal Signals ===");
        $display("Cycle | x_s0  | x2_s1  | ax1_s1 | x_s1  | y     ");
        $display("------|-------|--------|--------|-------|-------");
        
        for (cycle = 0; cycle < 10; cycle++) begin
            @(posedge clk);
            #1; // Small delay to sample after clock edge
            $display("%5d | %5d | %6d | %6d | %5d | %5d", 
                     cycle,
                     dut.x_s0,
                     dut.x2_s1,
                     dut.ax1_s1,
                     dut.x_s1,
                     y);
        end

        // Check if output matches expected value
        if (y === y_test) begin
            $display("\nTEST PASSED: Output y matches expected value %0d", y_test);
        end else begin
            $display("\nTEST FAILED: Output y = %0d does not match expected value %0d", y, y_test);
        end
        
        $display("\n=== Simulation Complete ===");
        $finish;
    end

endmodule