# xilinxutils/vitisstructs.py:  Data structure code generation for Vitis HLS

import numpy as np
import fixedpoint
from enum import Enum
from typing import Type
from numpy.typing import NDArray

def float_to_uint(
    fval: NDArray[np.floating]) -> NDArray[np.uint32]:
    """
    Converts a float value to its unsigned integer (bit representation).
    
    Parameters
    ----------
    fval : nd.array
        Float value to convert to bit representation.
        
    Returns
    -------
    uint_val : nd.array of np.uint32
        Unsigned integer representation of the float's bit pattern.
    """
    # Convert to np.float32 array to ensure 32-bit float representation
    f32 = np.asarray(fval, dtype=np.float32)
    # Use view to get the bit pattern as uint32
    return f32.view(np.uint32)


def uint_to_float(
    uval: NDArray[np.unsignedinteger]) -> NDArray[np.float32]:
    """
    Converts an unsigned integer bit pattern to float32 values.

    Parameters
    ----------
    uval : nd.array
        Unsigned integer array containing IEEE-754 float bit patterns.

    Returns
    -------
    fval : nd.array of np.float32
        Float values reconstructed from the input bit patterns.
    """
    # Convert to np.uint32 array to ensure 32-bit word representation
    u32 = np.asarray(uval, dtype=np.uint32)
    # Use view to interpret bits as float32
    return u32.view(np.float32)


class BaseType:
    """
    Abstract base for all types.

    Parameters
    ----------
    width : int
        Bitwidth of the type.
    """
    def __init__(
            self, 
            width: int):
        self.width = width
   
    def cpp_repr(self) -> str:
        raise NotImplementedError
    
    def read_expr(self,
                  word_name: str,
                  word_width: int = 32,
                  ind0 : int = 0) -> str:
        """
        Generate C++ expression to read this type from a word of type ap_unit<word_width>.

        Parameters
        ----------
        word_name : str
            Name of the variable representing the word to read from.
        word_width : int
            Bitwidth of the word type.
        ind0 : int, optional
            Starting bit index within the word to read from.
        """
        if self.width + ind0 > word_width:
            raise ValueError("Type bitwidth exceeds word bitwidth at given index.")
        return self.read_expr_impl(word_name, word_width, ind0)
    
    def read_expr_impl(self,
                       word_name: str,
                       word_width: int,
                       ind0 : int) -> str:
        raise NotImplementedError   
    
    def write_expr(self,
                   var_name: str,
                   word_name: str,
                   word_width: int = 32,
                   ind0: int = 0) -> str:
        """
        Generate C++ expression to write this type into a word of type ap_uint<word_width>.

        Parameters
        ----------
        var_name : str
            Name of the variable holding the field value.
        word_name : str
            Name of the variable representing the destination word.
        word_width : int
            Bitwidth of the word type.
        ind0 : int, optional
            Starting bit index within the word to write into.
        """
        if self.width + ind0 > word_width:
            raise ValueError("Type bitwidth exceeds word bitwidth at given index.")
        return self.write_expr_impl(var_name, word_name, word_width, ind0)

    def write_expr_impl(self,
                        var_name: str,
                        word_name: str,
                        word_width: int,
                        ind0: int) -> str:
        raise NotImplementedError
    
    def preamble(self) -> str:
        """
        Returns any necessary C++ preamble code for this type.  This code will be added
        at the top of the class definition.

        Returns
        -------
        str | None
            C++ code snippet.  If None or empty, no preamble is added.
        """
        return None
    
    def init_python_value(self):
        """
        Returns an initial value for a python field of this type.

        Returns
        -------
        any
            Initial value for a python field of this type.
        """
        return 0
    def write_uint(self, value) -> np.uint32:
        """
        Write value as an unsigned integer of this type.
        """
        raise NotImplementedError
    def read_uint(self, uint_value) -> np.uint32:
        """
        Read value from an unsigned integer of this type.
        """
        raise NotImplementedError

    
class IntType(BaseType):
    """
    This class is realized as arbitrary precision integer 
    type (ap_int/ap_uint) in Vitis and either np.int32/np.uint32
    or fixedpoint.FixedPoint in Python.

    Parameters
    ----------
    width : int
        Bitwidth of the integer.
    signed : bool, optional
        Whether the integer is signed (default True).
    use_np : bool, optional
        Use nunmpy integer types for widths <= 32 (default True).
        Otherwise, use fixedpoint.FixedPoint for all widths.
    """

    def __init__(
            self, 
            width: int, 
            signed: bool = True,
            use_np: bool = True):
        super().__init__(width)
        if width <= 0:
            raise ValueError("Width must be positive")
        self.signed = signed
        self.use_np = use_np
        if use_np and width > 32:
            raise ValueError("Numpy integer types only supported for widths <= 32")

    def cpp_repr(self) -> str:
        """
        Returns
        -------
        str
            C++ type string for this integer.
        """
        base = "ap_int" if self.signed else "ap_uint"
        return f"{base}<{self.width}>"

    def read_expr_impl(self,
                       word_name: str,
                       word_width: int,
                       ind0: int) -> str:
        """
        Generate C++ expression to read this integer from a word.

        Returns
        -------
        str
            C++ code snippet (expression only).
        """
        if self.width == word_width and ind0 == 0:
            # Direct assignment if the field fills the whole word
            return f"{word_name}"
        else:
            # Slice assignment
            high = ind0 + self.width - 1
            return f"{word_name}.range({high}, {ind0})"

    def write_expr_impl(self,
                        var_name: str,
                        word_name: str,
                        word_width: int,
                        ind0: int) -> str:
        """
        Generate C++ expression to write this integer into a word.

        Returns
        -------
        str
            C++ code snippet (statement).
        """
        if self.width == word_width and ind0 == 0:
            # Direct assignment if the field fills the whole word
            return f"{word_name} = {var_name};"
        else:
            # Slice assignment
            high = ind0 + self.width - 1
            return f"{word_name}.range({high}, {ind0}) = {var_name};"
        
    def init_python_value(self):
        """
        Returns an initial value for a python field of this type.

        Returns
        -------
        np.int32 | np.uint32
            Initial value for a python field of this type.
        """
        if self.use_np:
            if self.signed:
                val = np.int32(0)
            else:
                val = np.uint32(0)
        else:
            val = fixedpoint.FixedPoint(0, signed=self.signed, m=self.width, n=0)
        return val
    
    def write_uint(self, val) -> np.uint32:
        """
        Write value as an unsigned integer of this type.
        """
        if self.use_np:
            mask = (1 << self.width) - 1
            return val & mask
        else:
            if not isinstance(val, fixedpoint.FixedPoint):
                raise ValueError("Value must be a fixedpoint.FixedPoint")
            return np.uint32(val.bits[ self.width -1 : 0 ])
        
    def read_uint(self, uint_val):
        """
        Reads value from unsigned integer
        """
        if self.use_np:
            mask = (1 << self.width) - 1
            uint_val = uint_val & mask
            if self.signed:
                # Check if sign bit is set
                if uint_val & (1 << (self.width - 1)):
                    # Convert from unsigned to signed (two's complement)
                    uint_val = int(uint_val) - (1 << self.width)
                return np.int32(uint_val)
            else:
                return np.uint32(uint_val)
        else:
            return fixedpoint.FixedPoint(uint_val, signed=self.signed, m=self.width, n=0)
        
class FloatType(BaseType):
    """
    32-bit IEEE-754 floating point type.

    Notes
    -----
    - Always uses 32 bits.
    - Conversion between ap_uint<32> and float is done via a union
      to preserve bit patterns safely in HLS.
    """

    def __init__(self):
        super().__init__(32)

    def cpp_repr(self) -> str:
        """
        Returns
        -------
        str
            C++ type string for this float.
        """
        return "float"

    def read_expr_impl(self,
                       word_name: str,
                       word_width: int,
                       ind0: int) -> str:
        """
        Generate C++ code to read a float from a word.

        Returns
        -------
        str
            C++ code snippet (statement).
        """
        if ind0 + self.width > word_width:
            raise ValueError("Float field does not fit in word")

        # Slice bits if not aligned to full word
        if self.width == word_width and ind0 == 0:
            src_expr = word_name
        else:
            high = ind0 + self.width - 1
            src_expr = f"{word_name}.range({high}, {ind0})"

        # Use class utility generated in gen_include()
        return f"uint_to_float({src_expr})"

    def write_expr_impl(self,
                        var_name: str,
                        word_name: str,
                        word_width: int,
                        ind0: int) -> str:
        """
        Generate C++ code to write a float into a word.

        Returns
        -------
        str
            C++ code snippet (statement).
        """
        if ind0 + self.width > word_width:
            raise ValueError("Float field does not fit in word")

        high = ind0 + self.width - 1
        return f"{word_name}.range({high}, {ind0}) = float_to_uint({var_name});"
    
    def init_python_value(self):
        """
        Returns an initial value for a python field of this type.

        Returns
        -------
        np.float32
            Initial value for a python field of this type.
        """
        return np.float32(0.0)
    
    def write_uint(self, val) -> np.uint32:
        """
        Write float value as an unsigned integer (bit representation).
        
        Parameters
        ----------
        val : float or np.float32
            Float value to convert to bit representation.
            
        Returns
        -------
        np.uint32
            Unsigned integer representation of the float's bit pattern.
        """
        # Convert to np.float32 to ensure it's 32-bit
        f32 = np.float32(val)
        # Use view to get the bit pattern as uint32
        return f32.view(np.uint32)
    
    def read_uint(self, uint_val) -> np.float32:
        """
        Read float value from unsigned integer (bit representation).
        
        Parameters
        ----------
        uint_val : int or np.uint32
            Unsigned integer containing the float's bit pattern.
            
        Returns
        -------
        np.float32
            Float value reconstructed from bit pattern.
        """
        # Convert to uint32 and use view to interpret as float32
        u32 = np.uint32(uint_val)
        return u32.view(np.float32)


class EnumType(BaseType):
    """
    Enumeration type.

    Parameters
    ----------
    name : str
        Name of the enum type.
    enum_type : Type[Enum]
        Enum class type.
    width : int, optional
        Bitwidth of the enum type. If None, calculated from number of entries.
    """
    def __init__(self, 
                 name: str, 
                 enum_type : Type[Enum], 
                 width: int = None):
        self.name = name
        self.enum_type = enum_type
        self.entries = [(e.name, e.value) for e in enum_type]
        if width is None:
            import math
            width = max(1, math.ceil(math.log2(len(self.entries))))
        super().__init__(width)
    
    def cpp_repr(self) -> str:
        """
        Returns
        -------
        str
            C++ type string for this integer.
        """
        return f"ap_uint<{self.width}>"

    def read_expr_impl(self,
                       word_name: str,
                       word_width: int,
                       ind0: int) -> str:
        """
        Generate C++ expression to read this integer from a word.

        Returns
        -------
        str
            C++ code snippet (expression only).
        """
        if self.width == word_width and ind0 == 0:
            # Direct assignment if the field fills the whole word
            return f"{word_name}"
        else:
            # Slice assignment
            high = ind0 + self.width - 1
            return f"{word_name}.range({high}, {ind0})"

    def write_expr_impl(self,
                        var_name: str,
                        word_name: str,
                        word_width: int,
                        ind0: int) -> str:
        """
        Generate C++ expression to write this integer into a word.

        Returns
        -------
        str
            C++ code snippet (statement).
        """
        if self.width == word_width and ind0 == 0:
            # Direct assignment if the field fills the whole word
            return f"{word_name} = {var_name};"
        else:
            # Slice assignment
            high = ind0 + self.width - 1
            return f"{word_name}.range({high}, {ind0}) = {var_name};"

    def preamble(self) -> str:
        """
        Generate C++ enum declaration.

        Returns
        -------
        str
            C++ enum declaration code snippet.
        """
        enumerators = ",\n        ".join(f"{name} = {val}" for name, val in self.entries)
        return f"enum {self.name} : unsigned int {{\n        {enumerators}\n    }};"  

    def init_python_value(self):
        """
        Returns an initial value for a python field of this type.

        Returns
        -------
        Enum
            Initial value for a python field of this type (first enum value).
        """
        # Return the first enum value (lowest value)
        return min(self.enum_type, key=lambda e: e.value)
    
    def write_uint(self, val) -> np.uint32:
        """
        Write enum value as an unsigned integer.
        
        Parameters
        ----------
        val : Enum or int
            Enum value to convert. Can be an Enum instance or int value.
            
        Returns
        -------
        np.uint32
            Unsigned integer representation of the enum value.
        """
        if isinstance(val, self.enum_type):
            uint_val = val.value
        else:
            uint_val = int(val)
        
        mask = (1 << self.width) - 1
        return np.uint32(uint_val & mask)
    
    def read_uint(self, uint_val):
        """
        Read enum value from unsigned integer.
        
        Parameters
        ----------
        uint_val : int or np.uint32
            Unsigned integer containing the enum value.
            
        Returns
        -------
        Enum
            Enum instance corresponding to the value.
        """
        mask = (1 << self.width) - 1
        val = int(uint_val) & mask
        return self.enum_type(val)
    
class FieldInfo:
    def __init__(
            self, 
            name: str, 
            dtype: BaseType,
            descr: str = None,
            comment_style: str = "inline"):
        """
        Parameters
        ----------
        name : str
            Name of the field
        dtype : BaseType
            Data type of the field
        descr : str | None
            Optional description of the field
        comment_style : {"inline", "above"}
            Placement of the comment in generated C++ code
        """
        self.name = name
        self.dtype = dtype
        self.descr = descr
        self.comment_style = comment_style

    def cpp_decl(self) -> str:
        if self.descr and self.comment_style == 'above':
            return f"    // {self.descr}\n    {self.dtype.cpp_repr()} {self.name};"
        elif self.descr and self.comment_style == 'inline':
            return f"    {self.dtype.cpp_repr()} {self.name}; // {self.descr}"
        else:
            return f"    {self.dtype.cpp_repr()} {self.name};"

class VitisStruct(object):
    def __init__(
            self,
            name: str,
            fields: list[FieldInfo] | None = None):
        self.name = name
        self.fields = fields if fields is not None else []

        # Create dictionary of values
        self.data = {}
        for f in self.fields:
            self.data[f.name] = f.dtype.init_python_value()

    def get_length(self, bus_width: int = 32) -> int:
        """
        Get the number of stream words needed to represent this struct for a given bus width.

        Parameters
        ----------
        bus_width : int
            Bitwidth of the stream word.

        Returns
        -------
        int
            Number of stream words needed.
        """
        # Check that no field exceeds bus width
        for f in self.fields:
            if f.dtype.width > bus_width:
                raise ValueError(
                    f"Field '{f.name}' has width {f.dtype.width} bits, "
                    f"which exceeds bus width {bus_width} bits"
                )
            if f.dtype.width > 32:
                raise ValueError(
                    f"Field '{f.name}' has width {f.dtype.width} bits. "
                    "stream_utils helpers require field widths <= 32 bits"
                )

        # Pack fields into words using the same logic as gen_stream_write
        nwords = 0
        ind0 = 0
        for f in self.fields:
            bw = f.dtype.width
            if ind0 == 0 or (ind0 + bw > bus_width):
                nwords += 1
                ind0 = 0
            ind0 += bw

        return nwords

    def write_stream(self, bus_width: int = 32):
        """
        Generate a vector of unint<32> words representing this struct. 

        Parameters
        ----------
        bus_width : int
            Bus width.  Must be multiple of 32.

        Returns
        -------
        nd.array of (nwords, nbus_words) np.uint32
            An array of words representing this struct.
        """
        
        if bus_width % 32 != 0:
            raise ValueError("Bus width must be a multiple of 32")
        nbus_words = bus_width // 32


        # Create output array
        out_words = []

        ind0 = 0       # current bit index within the word
        word_count = 0 # count of words written
        bus_words = np.zeros((nbus_words,), dtype=np.uint32)
        
        for f in self.fields:
            bw = f.dtype.width
            val = self.data[f.name]

            # If field doesn't fit in current word, write current word and start a new one
            if ind0 + bw > bus_width:
                # Write current word to output array
                out_words.append(bus_words.copy())
                word_count += 1
                bus_words = np.zeros((nbus_words,), dtype=np.uint32)
                ind0 = 0

            # Write field bits into bus_words
            word = f.dtype.write_uint(val)
            j = ind0 // 32 # word index within bus_words
            k = ind0 % 32  # bit index within bus_words[j]
            bus_words[j] |= (word << k) & 0xFFFFFFFF
            word = word >> (32 - k)
            if k + bw > 32:
                # Spill over into next word
                bus_words[j + 1] |= word & 0xFFFFFFFF
                        
            # Advance bit index
            ind0 += bw

        # Write final word if needed
        if ind0 > 0:
            out_words.append(bus_words.copy())

        out_words = np.array(out_words, dtype=np.uint32)
        return out_words

    def read_stream(self, 
                    in_words: np.ndarray,
                    bus_width: int = 32):
        """
        Read struct data from a vector of uint32 words.

        Parameters
        ----------
        in_words : nd.array of (nwords, nbus_words) np.uint32
            An array of words representing this struct.
        bus_width : int
            Bus width.  Must be multiple of 32.
        """
        
        if bus_width % 32 != 0:
            raise ValueError("Bus width must be a multiple of 32")
        nbus_words = bus_width // 32

        ind0 = 0       # current bit index within the word
        word_count = 0 # count of words read

        for f in self.fields:
            bw = f.dtype.width

            # If field doesn't fit in current word, read a new one
            if ind0 + bw > bus_width:
                word_count += 1
                ind0 = 0

            # Read field bits from in_words
            j = ind0 // 32 # word index within bus_words
            k = ind0 % 32  # bit index within bus_words[j]
            word = in_words[word_count, j]
            field_bits = (word >> k) & ((1 << min(bw, 32 - k)) - 1)
            if k + bw > 32:
                # Spill over into next word
                word2 = in_words[word_count, j + 1]
                field_bits |= (word2 & ((1 << (bw - (32 - k))) - 1)) << (32 - k)
            
            # Convert bits to field value
            val = f.dtype.read_uint(field_bits)
            self.data[f.name] = val
            
            # Advance bit index
            ind0 += bw
    
class VitisCodeGen(object):
    """
    Class for generating Vitis C++ code for the VitisStruct.

    Parameters
    ----------
    vs :  VitisStruct
        VitisStruct instance to generate code for.
    """
    def __init__(self, 
                 vs : VitisStruct):
        self.name = vs.name
        self.fields = vs.fields
        self.stream_bus_widths = []
   
    def gen_include(
        self,
        include_file: None | str = None,
        include_dir: None | str = None,
        bus_widths: list[int] | None = None,
        copy_stream_utils: bool = True
    ) -> str: 
        """
        Generate and write the C++ include file for this data structure.

        The include file will be of the form:

        #ifndef <INCLUDE_FILE_H>
        #define <INCLUDE_FILE_H>

        class <name> {
            // fields...
            
            // one stream_read function for each bus width
            // one stream_write function for each bus width
            // generic stream_read and stream_write dispatch functions
            // equality operator
            // to_string method
        };

        #endif // <INCLUDE_FILE_H>

        Parameters
        ----------
        include_dir : str
            Directory where the include file is located. The directory will be
            created if needed.
        include_file : None | str
            Name of the include file. If None, defaults to <name>.h
        bus_widths : list[int] | None
            List of bus widths to generate stream functions for.
            If None, defaults to [32].
        copy_stream_utils : bool
            Whether to copy the stream_utils.h file to the include directory.

        Returns
        -------
        str
            Full path to the generated include file.
        """
        import os
        
        # Set defaults
        if include_file is None:
            if include_dir is None:
                include_dir = os.getcwd()
            include_file = os.path.join(include_dir, f"{self.name.lower()}.h")
        elif not os.path.isabs(include_file):
            if include_dir is None:
                include_dir = os.getcwd()
            include_file = os.path.join(include_dir, include_file)

        include_file = os.path.abspath(include_file)
        include_dir = os.path.dirname(include_file)
        if bus_widths is None:
            bus_widths = [32]
        
        # Store bus widths for dispatch method generation
        self.stream_bus_widths = bus_widths
        
        # Create include directory if it doesn't exist
        os.makedirs(include_dir, exist_ok=True)
        
        # Delete existing file if it exists
        file_path = include_file
        if os.path.exists(file_path):
            os.remove(file_path)
        
        # Generate include guard macro name from file basename
        guard_name = os.path.basename(include_file).upper().replace('.', '_').replace('-', '_')
        
        # Build the file content
        lines = []
        
        # Include guard start
        lines.append(f"#ifndef {guard_name}")
        lines.append(f"#define {guard_name}")
        lines.append("")
        
        # Include necessary headers
        lines.append("#include <hls_stream.h>")
        lines.append("#include <ap_int.h>")
        lines.append("#include <ap_axi_sdata.h>")
        stream_utils_path = os.path.join(os.path.dirname(__file__), "codegen", "stream_utils.h")
        # Copy stream_utils.h to include directory
        if copy_stream_utils:
            import shutil
            shutil.copyfile(stream_utils_path, os.path.join(include_dir, "stream_utils.h"))
        lines.append("#include \"stream_utils.h\"")    
        lines.append("#include <string>")
        lines.append("#include <sstream>")
        lines.append("")
        
        # Struct declaration with fields
        lines.append(f"class {self.name} {{")
        lines.append("public:")
        lines.append("")
        
        # Add preambles for fields that have them
        for field in self.fields:
            preamble = field.dtype.preamble()
            if preamble is not None and preamble.strip():
                lines.append(f"    {preamble}")
                lines.append("")
    
        for field in self.fields:
            lines.append(field.cpp_decl())
        lines.append("")
    
        # Generate stream functions for each bus width
        for bus_width in bus_widths:
            # Generate stream_read
            lines.append(self.gen_stream_read(bus_width))
            lines.append("")
            
            # Generate stream_write
            lines.append(self.gen_stream_write(bus_width))
            lines.append("")

        # Generate generic dispatch methods
        stream_write_dispatch, stream_read_dispatch = self.gen_stream_dispatch()
        lines.append(stream_write_dispatch)
        lines.append("")
        lines.append(stream_read_dispatch)
        lines.append("")
        
        # Generate equality operator
        lines.append(self.gen_equality_operator())
        lines.append("")
        
        # Generate to_string method
        lines.append(self.gen_string_method())
        lines.append("")

        # Close struct
        lines.append("};")
        lines.append("")

        # Include guard end
        lines.append(f"#endif // {guard_name}")
        lines.append("")

        # Write to file
        with open(file_path, 'w') as f:
            f.write("\n".join(lines))

        return file_path

    def gen_stream_read(self, bus_width: int = 32) -> str:
        """
        Generate the C++ method for reading this struct from an HLS stream.

        Parameters
        ----------
        bus_bits : int
            Bitwidth of the stream word.

        Returns
        -------
        str
            C++ method definition as a string.
        """
        # Check that no field exceeds bus width
        for f in self.fields:
            if f.dtype.width > bus_width:
                raise ValueError(
                    f"Field '{f.name}' has width {f.dtype.width} bits, "
                    f"which exceeds bus width {bus_width} bits"
                )
    
        lines = []
        lines.append("    template<typename Tstream>")
        lines.append(f"    bool stream_read_{bus_width}(hls::stream<Tstream>& in) {{")
        lines.append(f"        constexpr int bus_bits = decltype(Tstream::data)::width;")
        lines.append(f"        static_assert(bus_bits == {bus_width}, "
                    f"\"Only {bus_width}-bit stream supported in {self.name}::stream_read_{bus_width}\");")
        lines.append("")

        ind0 = 0       # current bit index within the word
        word_count = 0 # count of words read
        word_var = None

        for f in self.fields:
            bw = f.dtype.width

            # If field doesn't fit in current word, read a new one
            if ind0 + bw > bus_width or word_var is None:
                word_var = f"w{word_count}"
                lines.append(f"        Tstream {word_var} = in.read();")
                ind0 = 0
                word_count += 1

            # Generate assignment using field's read_expr
            expr = f.dtype.read_expr(word_var + ".data", bus_width, ind0)
            lines.append(f"        {f.name} = {expr};")

            # Advance bit index
            ind0 += bw

        lines.append(f"        bool tlast = {word_var}.last;")
        lines.append("")
        lines.append("        return tlast;")
        lines.append("    }")
        return "\n".join(lines)

    def gen_stream_write(self, bus_width: int = 32) -> str:
        """
        Generate the C++ method for writing this struct to an HLS stream.

        Parameters
        ----------
        bus_width : int
            Bitwidth of the stream word.

        Returns
        -------
        str
            C++ method definition as a string.
        """
        # Check that no field exceeds bus width
        for f in self.fields:
            if f.dtype.width > bus_width:
                raise ValueError(
                    f"Field '{f.name}' has width {f.dtype.width} bits, "
                    f"which exceeds bus width {bus_width} bits"
                )
            if f.dtype.width > 32:
                raise ValueError(
                    f"Field '{f.name}' has width {f.dtype.width} bits. "
                    "stream_utils helpers require field widths <= 32 bits"
                )
    
        lines = []
        lines.append("    template<typename Tstream>")
        lines.append(f"    void stream_write_{bus_width}(hls::stream<Tstream>& out, bool tlast = true) const {{")
        lines.append(f"        constexpr int bus_bits = decltype(Tstream::data)::width;")
        lines.append(f"        static_assert(bus_bits == {bus_width}, "
                    f"\"Only {bus_width}-bit stream supported in {self.name}::stream_write_{bus_width}\");")
        lines.append("")

        # Pack fields into stream words
        words = []
        current_word = []
        ind0 = 0
        for f in self.fields:
            bw = f.dtype.width
            if current_word and (ind0 + bw > bus_width):
                words.append(current_word)
                current_word = []
                ind0 = 0
            current_word.append((f, ind0))
            ind0 += bw
        if current_word:
            words.append(current_word)

        # Use shared AXI helpers for all supported bus widths.
        # We assume each individual field is <= 32 bits.
        for i, word_fields in enumerate(words):
            is_last_word = (i == len(words) - 1)
            last_arg = "tlast" if is_last_word else None

            if len(word_fields) == 1:
                f, field_ind0 = word_fields[0]
                bw = f.dtype.width

                if isinstance(f.dtype, FloatType):
                    value_expr = f"float_to_uint({f.name})"
                else:
                    value_expr = f"{f.name}"

                if field_ind0 == 0 and bw == 32:
                    if last_arg is None:
                        lines.append(f"        out.write(axi_word<Tstream>({value_expr}));")
                    else:
                        lines.append(f"        out.write(axi_word<Tstream>({value_expr}, {last_arg}));")
                else:
                    high = field_ind0 + bw - 1
                    if last_arg is None:
                        lines.append(
                            f"        out.write(axi_word_range<Tstream>({value_expr}, {high}, {field_ind0}));"
                        )
                    else:
                        lines.append(
                            f"        out.write(axi_word_range<Tstream>({value_expr}, {high}, {field_ind0}, {last_arg}));"
                        )
            else:
                word_var = f"w{i}"
                first_f, first_ind0 = word_fields[0]
                first_bw = first_f.dtype.width

                if isinstance(first_f.dtype, FloatType):
                    first_value_expr = f"float_to_uint({first_f.name})"
                else:
                    first_value_expr = f"{first_f.name}"

                if first_ind0 == 0 and first_bw == 32:
                    if last_arg is None:
                        lines.append(f"        Tstream {word_var} = axi_word<Tstream>({first_value_expr});")
                    else:
                        lines.append(f"        Tstream {word_var} = axi_word<Tstream>({first_value_expr}, {last_arg});")
                else:
                    first_high = first_ind0 + first_bw - 1
                    if last_arg is None:
                        lines.append(
                            f"        Tstream {word_var} = axi_word_range<Tstream>({first_value_expr}, {first_high}, {first_ind0});"
                        )
                    else:
                        lines.append(
                            f"        Tstream {word_var} = axi_word_range<Tstream>({first_value_expr}, {first_high}, {first_ind0}, {last_arg});"
                        )

                for f, field_ind0 in word_fields[1:]:
                    write_stmt = f.dtype.write_expr(f.name, word_var + ".data", bus_width, field_ind0)
                    lines.append(f"        {write_stmt}")
                lines.append(f"        out.write({word_var});")
            lines.append("")

        lines.append("    }")
        return "\n".join(lines)

    def gen_stream_dispatch(self) -> tuple[str, str]:
        """
        Generate generic stream_read and stream_write dispatch methods.
        
        Returns
        -------
        tuple[str, str]
            (stream_write code, stream_read code)
        """
        
        if not self.stream_bus_widths:
            # If no bus widths are defined, generate methods that always fail at compile time
            write_lines = []
            write_lines.append("    template<typename Tstream>")
            write_lines.append("    void stream_write(hls::stream<Tstream>& out, bool tlast = true) const {")
            write_lines.append("        static_assert(sizeof(Tstream) == 0, ")
            write_lines.append(f"                     \"No stream bus widths configured for {self.name}\");")
            write_lines.append("    }")
            
            read_lines = []
            read_lines.append("    template<typename Tstream>")
            read_lines.append("    bool stream_read(hls::stream<Tstream>& in) {")
            read_lines.append("        static_assert(sizeof(Tstream) == 0, ")
            read_lines.append(f"                     \"No stream bus widths configured for {self.name}\");")
            read_lines.append("        return false;")
            read_lines.append("    }")
            
            return "\n".join(write_lines), "\n".join(read_lines)

        # Generate stream_write dispatch
        write_lines = []
        write_lines.append("    template<typename Tstream>")
        write_lines.append("    void stream_write(hls::stream<Tstream>& out, bool tlast = true) const {")
        write_lines.append("        constexpr int bus_bits = decltype(Tstream::data)::width;")
        
        for i, width in enumerate(self.stream_bus_widths):
            if i == 0:
                write_lines.append(f"        if constexpr (bus_bits == {width}) {{")
            else:
                write_lines.append(f"        }} else if constexpr (bus_bits == {width}) {{")
            write_lines.append(f"            stream_write_{width}(out, tlast);")
        
        write_lines.append("        } else {")
        widths_str = ", ".join(str(w) for w in self.stream_bus_widths)
        write_lines.append(f"            static_assert(bus_bits == {self.stream_bus_widths[0]}, ")
        write_lines.append(f"                         \"Unsupported bus width. Supported widths: {widths_str}\");")
        write_lines.append("        }")
        write_lines.append("    }")
        
        # Generate stream_read dispatch
        read_lines = []
        read_lines.append("    template<typename Tstream>")
        read_lines.append("    bool stream_read(hls::stream<Tstream>& in) {")
        read_lines.append("        constexpr int bus_bits = decltype(Tstream::data)::width;")
        
        for i, width in enumerate(self.stream_bus_widths):
            if i == 0:
                read_lines.append(f"        if constexpr (bus_bits == {width}) {{")
            else:
                read_lines.append(f"        }} else if constexpr (bus_bits == {width}) {{")
            read_lines.append(f"            return stream_read_{width}(in);")
        
        read_lines.append("        } else {")
        read_lines.append(f"            static_assert(bus_bits == {self.stream_bus_widths[0]}, ")
        read_lines.append(f"                         \"Unsupported bus width. Supported widths: {widths_str}\");")
        read_lines.append("            return false;")
        read_lines.append("        }")
        read_lines.append("    }")
        
        return "\n".join(write_lines), "\n".join(read_lines)
    
    def gen_equality_operator(self) -> str:
        """
        Generate C++ equality operator for this struct.

        Returns
        -------
        str
            C++ method definition as a string.
        """
        lines = []
        lines.append(f"    bool operator==(const {self.name}& other) const {{")
        comparisons = [f"(this->{f.name} == other.{f.name})" for f in self.fields]
        lines.append("        return " + " && ".join(comparisons) + ";")
        lines.append("    }")
        return "\n".join(lines)
    
    def gen_string_method(self) -> str:
        """
        Generate C++ method to convert this struct to a string.

        Returns
        -------
        str
            C++ method definition as a string.
        """
        lines = []
        lines.append("    std::string to_string() const {")
        lines.append("        std::ostringstream oss;")
        lines.append('        oss << "{";')
        for i, f in enumerate(self.fields):
            if i < len(self.fields) - 1:
                lines.append(f'        oss << "{f.name}: " << {f.name} << ", ";')
            else:
                lines.append(f'        oss << "{f.name}: " << {f.name};')
        lines.append('        oss << "}";')
        lines.append("        return oss.str();")
        lines.append("    }")
        return "\n".join(lines)