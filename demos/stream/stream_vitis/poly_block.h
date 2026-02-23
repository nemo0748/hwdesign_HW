#ifndef POLY_BLOCK_H
#define POLY_BLOCK_H

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <cstring>


// Data type for stream elements
// We use 32-bit wide data for simplicity with zero lengths
// for TKEEP, TSTRB, TUSER.  This data type will give us access to the TLAST field
// that will be used for synchronization
typedef ap_axis<32,0,0,0> stream_t; 

    
void poly_block(
    hls::stream<stream_t>& cmd_stream, 
    hls::stream<stream_t>& resp_stream);
    
#endif // POLY_BLOCK_H