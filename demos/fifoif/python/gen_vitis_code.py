# generate Vitis HLS code for data structures
import datastructs
from xilinxutils.vitisstructs import VitisStruct, VitisCodeGen

# Get include directory
import os
include_dir = os.path.join(os.path.dirname(__file__), 
                           '..', 'fifo_fun_vitis', 'src' )
include_dir = os.path.abspath(include_dir)
print(include_dir)

# Bus widths to support
stream_bus_widths = [32, 64]

# Generate commmand structure include file
cmd_struct = VitisStruct("Cmd", datastructs.cmd_fields) 
vg = VitisCodeGen(cmd_struct)
cmd_file = os.path.join(include_dir, "cmd.h")
vg.gen_include(include_file=cmd_file, bus_widths=stream_bus_widths)