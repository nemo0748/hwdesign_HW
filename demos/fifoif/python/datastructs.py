# datastructs.py:  Generates the include files for the data structures

from xilinxutils.vitisstructs import VitisStruct, FieldInfo, IntType, EnumType
from enum import Enum

# Stream bit widths to support
stream_bus_widths = [32, 64]

# Command structure
cmd_fields = [
    FieldInfo("trans_id", IntType(16), descr="Transaction ID"),
    FieldInfo("a", IntType(32), descr="Operand A"),
    FieldInfo("b", IntType(32), descr="Operand B")]


# Response structure
class ErrCodes(Enum):
    NO_ERR = 0
    SYNC_ERR = 1
resp_fields = [
    FieldInfo("trans_id", IntType(16), descr="Transaction ID"),
    FieldInfo("c", IntType(32), descr="Operand C"),
    FieldInfo("d", IntType(32), descr="Operand D"),
    FieldInfo("err_code", EnumType("ErrCodes", ErrCodes, 8), descr="Error Code")]
