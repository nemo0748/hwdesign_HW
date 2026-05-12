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

    logic [5:0]  nbits_reg; // I added this

    typedef enum logic [1:0] {
        IDLE,
        RUN,
        DONE
    } state_t;
    

    // TODO:  Complete the code for the divide operation


    state_t state;

    always_ff @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            a_reg <= 32'b0;
            b_reg <= 32'b0;
            z_reg <= 32'b0;
            count <= 6'b0;
            nbits_reg <= 6'b0;
        end else begin
            case (state)
                IDLE: begin
                    // Wait until inready=1 and invalid=1 to start
                    if (invalid) begin
                        a_reg     <= a;     // Register inputs
                        b_reg     <= b;
                        nbits_reg <= nbits;
                        z_reg     <= 32'b0;
                        count     <= 6'b0;
                        state     <= RUN;   // Move to RUN
                    end
                end

                RUN: begin
                    // Module performs division over multiple iterations
                    if (count == nbits_reg) begin
                        state <= DONE;      // After nbits iterations, move to DONE
                    end else begin
                        // Hardware equivalent of: remain = remain << 1
                        // then: if remain >= b: remain = remain - b, z = (z<<1)|1
                        if ({a_reg[30:0], 1'b0} >= b_reg) begin
                            a_reg <= {a_reg[30:0], 1'b0} - b_reg;
                            z_reg <= {z_reg[30:0], 1'b1};
                        end else begin
                            a_reg <= {a_reg[30:0], 1'b0};
                            z_reg <= {z_reg[30:0], 1'b0};
                        end
                        count <= count + 1'b1; // Increment iteration count
                    end
                end

                DONE: begin
                    // Move back to IDLE when outready=1
                    if (outready) begin
                        state <= IDLE;
                    end
                end

                default: state <= IDLE;
            endcase
        end
    end

    // Combinational Output Handshaking
    // IDLE: inready=1, outvalid=0
    // RUN:  inready=0, outvalid=0
    // DONE: inready=0, outvalid=1
    assign inready  = (state == IDLE);
    assign outvalid = (state == DONE);
    assign z        = z_reg;
    

endmodule