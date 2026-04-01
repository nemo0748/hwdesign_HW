from __future__ import annotations

import argparse
import subprocess
import sys
from enum import IntEnum
from pathlib import Path

import numpy as np

from xilinxutils.codegen.build import CodeGenConfig
from xilinxutils.codegen.streamutils import copy_streamutils
from xilinxutils.hw.dataschema import DataArray, DataList, EnumField, FloatField, IntField
from xilinxutils.hw import toolchain





"""
Define the data schemas for the polynomial accelerator
"""

INCLUDE_DIR = "include"
WORD_BW_SUPPORTED = [32, 64]
MaxNsamp = 128

Float32 = FloatField.specialize(bitwidth=32)

class PolyError(IntEnum):
    NO_ERROR = 0
    WRONG_NSAMP = 1

PolyErrorField = EnumField.specialize(enum_type=PolyError, include_dir=INCLUDE_DIR)


class CoeffArray(DataArray):
    ncoeffs = 4
    element_type = Float32
    static = True
    max_shape = (ncoeffs,)
    include_dir = INCLUDE_DIR


TransId = IntField.specialize(bitwidth=16, signed=False)
Nsamp = IntField.specialize(bitwidth=16, signed=False)
class PolyCmdHdr(DataList):
    elements = {
        "tx_id": {
            "schema": TransId,
            "description": "Transaction ID",
        },
        "coeffs": {
            "schema": CoeffArray,
            "description": "Polynomial coefficients",
        },
        "nsamp": {
            "schema": Nsamp,
            "description": "Number of samples",
        },
    }
    include_dir = INCLUDE_DIR


class PolyRespHdr(DataList):
    elements = {
        "tx_id": {
            "schema": TransId,
            "description": "Echo of the transaction ID sent in the command",
        },
    }
    include_dir = INCLUDE_DIR


class PolyRespFtr(DataList):
    elements = {
        "nsamp_read": {
            "schema": Nsamp,
            "description": "Number of samples returned in the response",
        },
        "error": {
            "schema": PolyErrorField,
            "description": "Error code indicating success or type of failure",
        },
    }
    include_dir = INCLUDE_DIR


class SampDataIn(DataArray):
    element_type = Float32
    static = False
    max_shape = (MaxNsamp,)
    include_dir = INCLUDE_DIR


class SampDataOut(DataArray):
    element_type = Float32
    static = False
    max_shape = (MaxNsamp,)
    include_dir = INCLUDE_DIR


SCHEMA_CLASSES = [
    PolyErrorField,
    CoeffArray,
    PolyCmdHdr,
    PolyRespHdr,
    PolyRespFtr,
    SampDataIn,
    SampDataOut,
]

"""
Python Golden model for polynomial evaluation
"""
def polynomial_eval(
    cmd_hdr: PolyCmdHdr,
    samp_in: SampDataIn,
) -> tuple[PolyRespHdr, SampDataOut, PolyRespFtr]:
    resp_hdr = PolyRespHdr()
    resp_hdr.tx_id = cmd_hdr.tx_id

    coeffs = np.asarray(cmd_hdr.coeffs, dtype=np.float32)
    x = np.asarray(samp_in.val, dtype=np.float32)
    y = np.zeros_like(x)
    power = np.ones_like(x)
    for coeff in coeffs:
        y += coeff * power
        power *= x

    samp_out = SampDataOut()
    samp_out.val = y

    resp_ftr = PolyRespFtr()
    resp_ftr.nsamp_read = len(x)
    resp_ftr.error = PolyError.NO_ERROR if len(x) == int(cmd_hdr.nsamp) else PolyError.WRONG_NSAMP

    return resp_hdr, samp_out, resp_ftr


"""
Building inputs, generating headers, writing vectors, running Vitis, and validating outputs
"""
def build_demo_inputs(nsamp: int = 100) -> tuple[PolyCmdHdr, SampDataIn]:
    coeffs = CoeffArray()
    coeffs.val = np.array([1.0, -2.0, -3.0, 4.0], dtype=np.float32)

    cmd_hdr = PolyCmdHdr()
    cmd_hdr.tx_id = 42
    cmd_hdr.coeffs = coeffs.val
    cmd_hdr.nsamp = nsamp

    samp_in = SampDataIn()
    samp_in.val = np.linspace(0.0, 1.0, nsamp, dtype=np.float32)
    return cmd_hdr, samp_in


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the polynomial accelerator example.")
    parser.add_argument("--nsamp", type=int, default=100, help="Number of input samples to generate.")
    parser.add_argument("--skip-vitis", action="store_true", help="Only run the Python side of the example.")
    parser.add_argument("--plot", action="store_true", help="Plot the Python golden-model output.")

    # Interactive kernels add their own argv entries such as -f <kernel.json>.
    if "ipykernel" in sys.modules or "-f" in sys.argv[1:]:
        args, _ = parser.parse_known_args()
        return args

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    
    # Create a data directory under the current working directory
    data_dir = Path.cwd() / "data"
    data_dir.mkdir(exist_ok=True)

    #generate_headers(EXAMPLE_DIR)

    cmd_hdr, samp_in = build_demo_inputs(nsamp=args.nsamp)
    resp_hdr, samp_out, resp_ftr = polynomial_eval(cmd_hdr, samp_in)

    for s in samp_in.val:
        print(s)


    #data_dir = write_vectors(EXAMPLE_DIR, cmd_hdr, samp_in)