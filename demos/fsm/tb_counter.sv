`timescale 1ns/1ps

module tb_counter;

    // Clock and reset signals
    logic clk;
    logic rst;
    
    // Testbench signals
    logic [31:0] cnt_init;
    logic start;
    logic ready;
    logic [31:0] cnt;
    
    // Clock generation - 10ns period (100 MHz)
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end
    
    // Instantiate the device under test (dut)
    counter dut (
        .clk(clk),
        .rst(rst),
        .cnt_init(cnt_init),
        .start(start),
        .cnt(cnt)
    );
    
    // Test sequence
    initial begin
        // Initialize signals
        rst = 1;
        start = 0;
        cnt_init = 0;
        
        // Wait for a few clock cycles
        repeat(2) @(posedge clk);
        
        // Release reset
        rst = 0;
        @(posedge clk);
        
        // Set cnt_init = 10 and start the counter
        cnt_init = 10;
        start = 1;
        @(posedge clk);
        
        // Deassert start
        start = 0;
        
        // Run for 15 more clock cycles
        repeat(15) @(posedge clk);
        
        // End simulation
        $finish;
    end
    
    // Monitor and display cnt on each clock cycle (runs in parallel)
    int cycle_count1;
    int file;
    initial begin
        cycle_count = 0;
        
        // Open CSV file and write header
        file = $fopen("counter_output.csv", "w");
        $fdisplay(file, "cycle,start,cnt_init,cnt");
        
        forever begin
            @(posedge clk);
            #2; // small delay to allow cnt to update
            $display("Cycle=%3d start=%3d cnt_init=%3d cnt=%3d",
             cycle_count, start, cnt_init,  cnt);
            $fdisplay(file, "%0d,%0d,%0d,%0d", cycle_count, start, cnt_init, cnt);
            cycle_count++;
        end
    end

endmodule

