# Variable Arithmetic Core (VAC)
Team members: Neha Ghosh 
Note - I could not find a teammate. I tried asking in class but classmates said they already had a teammate/would not reply back when messaged

The Variable Arithmetic Core (VAC) is a programmable hardware accelerator designed to compute cubic polynomials (Ax^3 + Bx^2 + Cx + D). The IP allows for dynamic, user-defined coefficients and features a Dual-Output Architecture: a primary saturated result to ensure system stability, and a secondary overflowed result for diagnostic and error-tracking purposes. The VAC is well-suited for hardware acceleration because it leverages a multi-stage sequential pipeline to achieve high-throughput processing, delivering a new result on every clock cycle. Unlike software-based execution, the VAC performs fixed-point scaling and dual-path error handling (saturation vs. overflow) in parallel combinational logic. This eliminates the need for conditional branching, significantly reducing latency and ensuring deterministic performance for real-time applications like signal processing or control systems. 


The VAC utilizes a hybrid interface strategy, employing AXI4-Lite for low-bandwidth coefficient configuration and AXI4-Stream for high-speed data flow. AXI4-Stream is used for the high-throughput data path. This ensures the host computer can update the polynomial "recipe" without interrupting the flow of data. 

# Modules 
The first module is the AXI-Lite **Control Register Bank**, which acts as the "memory" for the system. It holds the user-defined integer coefficients (A, B, C, D) and allows them to be updated dynamically by the host computer. 

The second module is the **Sequential Power Engine**, which serves as the entry point for the user’s data stream x. It utilizes an AXI4-Stream Slave interface to ingest integer values and calculates the powers across a multi-stage sequential pipeline. 

The third module is the **Weighted Accumulator**, which pulls the stored coefficients from the register bank and applies them to the incoming powers of x. Since the core operates strictly on whole numbers, this module performs standard 32-bit integer multiplication and summation without the need for fractional scaling. 

Finally, the **Dual-Path Output Formatter** takes the final sum and splits it into a saturated output—and a overflowed output. Both results are sent back to the host via an AXI4-Stream Master interface.
