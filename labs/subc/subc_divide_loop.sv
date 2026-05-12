`timescale 1ns/1ps

module subc_divide(
    input  logic         clk,
    input  logic         rst,

    // Handshake for inputs
    input  logic         invalid,
    output logic         inready,

    // Inputs
    input  logic [31:0]  a,
    input  logic [31:0]  b,
    input  logic [5:0]   nbits,     // runtime number of iterations (0–32)

    // Handshake for outputs
    input  logic         outready,
    output logic         outvalid,

    // Output
    output logic [31:0]  z
);

    // Internal registers
    logic [31:0] a_reg, b_reg;
    logic [31:0] z_reg;
    logic [5:0]  count;

    typedef enum logic [1:0] {
        IDLE,
        RUN,
        DONE
    } state_t;

    // TODO:  Complete the code for the divide operation

    

endmodule