# datastructs.py:  Generates the include files for the data structures

from xilinxutils.vitisstructs import FieldInfo, FloatType, IntType, EnumType, FloatType
from enum import Enum

# Stream bit widths to support
stream_bus_widths = [32, 64]

# Command structure
cmd_hdr_fields = [
    FieldInfo("trans_id", IntType(16), descr="Transaction ID"),
    FieldInfo("a0", FloatType(), descr="Coefficient a0"),
    FieldInfo("a1", FloatType(), descr="Coefficient a1"),
    FieldInfo("a2", FloatType(), descr="Coefficient a2"),
    FieldInfo("n", IntType(32, signed=False), descr="Number of data points")]


# Response structure
class ErrCodes(Enum):
    NO_ERR = 0
    SYNC_ERR = 1
resp_hdr_fields = [
    FieldInfo("trans_id", IntType(16), descr="Transaction ID")
]
resp_footer_fields = [
    FieldInfo("nread", IntType(32, signed=False), 
              descr="Number of data points read before TLAST"),
    FieldInfo("err_code", EnumType("ErrCodes", ErrCodes, 8), 
              descr="Error Code")]
