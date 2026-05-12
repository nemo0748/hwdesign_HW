# Synthesizes and builds the IP for import into Vivado

# Set the desired unroll factor
set uf 1
set sol_name "solution_1"

# Create and configure the HLS project
open_project vmult_hls -reset
set_top vec_mult

# Add source and testbench files with macro definitions
add_files -cflags "-Iinclude -DPIPELINE_EN=1 -DUNROLL_FACTOR=$uf" src/vmult.cpp
add_files -tb testbench/tb_vmult.cpp

# Create solution and set target part and clock
open_solution $sol_name -reset

# Set the part number for Pynq-Z2
set_part {xc7z020clg400-1}
create_clock -period 4 -name default

# Set the part number for the RFSoC 4x2
#set_part {xczu48dr-ffvg1517-2-e}
#create_clock -period 3.33 -name default

# Run synthesis
# csynth_design

# Optional: run co-simulation
# cosim_design

# Export IP for Vivado
# export_design -format ip_catalog

# Close the project
close_project

# Exit Vitis
exit