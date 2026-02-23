# ============================================================
# Vitis HLS TCL script for the block polynomial demo
# ============================================================

# Create or reset the project
open_project hls_component -reset

# Set the top-level function
set_top poly_block

# ------------------------------------------------------------
# Add design and testbench files
# ------------------------------------------------------------
# Adjust paths if your directory structure differs
add_files poly_block.cpp
add_files -tb tb_poly_block.cpp

# ------------------------------------------------------------
# Create solution and configure hardware part + clock
# ------------------------------------------------------------
open_solution "solution1" -reset

# FPGA part (Pynq-Z2)
set_part {xc7z020clg400-1}

# Clock: 100 MHz = 10 ns
create_clock -period 10 -name default

# ------------------------------------------------------------
# C Simulation
# ------------------------------------------------------------
csim_design

# ------------------------------------------------------------
# C Synthesis.  Comment out if you want to run C simulation only.
# ------------------------------------------------------------
csynth_design

# ------------------------------------------------------------
# RTL Co-Simulation
# Pass argument "rtl" to testbench via cosim.argv
# Comment out if you want to run C synthesis only.
# ------------------------------------------------------------
cosim_design -trace_level all