`timescale 1ns/1ps

module counter (
    input  logic clk,
    input  logic rst,   // synchronous reset
    input  logic [31:0] cnt_init,
    input  logic start,
    output logic [31:0] cnt
);

    // Combinational outputs
    logic [31:0] cnt_next;

    // Combinational logic
    always_comb begin
        cnt_next = cnt;  // default.  Good practice.
        if ((start) && (cnt == 0))  begin
            cnt_next = cnt_init;
        end else if (cnt > 0) begin
            cnt_next = cnt -1;
        end else begin
            cnt_next = 0;
        end        
    end

    // Register the inputs
    always_ff @(posedge clk) begin
        if (rst) begin
            cnt <= 0;
        end else begin
            cnt <= cnt_next;    
        end
    end

    
endmodule