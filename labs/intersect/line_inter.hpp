#ifndef INTERSECT_HPP
#define INTERSECT_HPP

#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "include/intersect_error.h"
#include "include/point.h"
#include "include/cmd_hdr.h"
#include "include/resp_ftr.h"
#include "include/resp_hdr.h"
#include "include/float32_array_utils.h"
#include "include/streamutils_hls.h"


// Maximum number of samples 
static const int max_nsamp = 1024;

// Dimension per sample (3D)
static const int ndim = 3;

// AXI bandwdith
static const int WORD_BW = 32;

// To build a 64-bit variant, set WORD_BW to 64.
static_assert(WORD_BW == 32 || WORD_BW == 64, "WORD_BW must be 32 or 64");

using axis_word_t = hls::axis<ap_uint<WORD_BW>, 0, 0, 0>;

// Kernel function prototype
void line_inter(hls::stream<axis_word_t>& in_stream, hls::stream<axis_word_t>& out_stream);

#endif