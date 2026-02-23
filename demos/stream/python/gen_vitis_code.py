# generate Vitis HLS code for data structures
import datastructs
from xilinxutils.vitisstructs import VitisStruct, VitisCodeGen

# Get include directory
import os
include_dir = os.path.join(os.path.dirname(__file__), 
                           '..', 'stream_vitis' )
include_dir = os.path.abspath(include_dir)
print(include_dir)

# Bus widths to support
stream_bus_widths = [32, 64]

# Generate command header include file
cmd_struct = VitisStruct("CmdHdr", datastructs.cmd_hdr_fields) 
vg = VitisCodeGen(cmd_struct)
cmd_file = os.path.join(include_dir, "cmd_hdr.h")
vg.gen_include(include_file=cmd_file, bus_widths=stream_bus_widths,
               copy_stream_utils=True)

# Generate response header include file
resp_struct = VitisStruct("RespHdr", datastructs.resp_hdr_fields) 
vg = VitisCodeGen(resp_struct)
resp_file = os.path.join(include_dir, "resp_hdr.h")
vg.gen_include(include_file=resp_file, bus_widths=stream_bus_widths)

# Generate response footer header include file
resp_footer_struct = VitisStruct("RespFooter", datastructs.resp_footer_fields) 
vg = VitisCodeGen(resp_footer_struct)
resp_footer_file = os.path.join(include_dir, "resp_footer.h")
vg.gen_include(include_file=resp_footer_file, bus_widths=stream_bus_widths)