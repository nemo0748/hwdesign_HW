#include "poly.hpp"

#ifndef UNROLL_FACTOR
#define UNROLL_FACTOR 2
#endif


static float eval_poly_horner(const float coeff[4], float x) {
#pragma HLS INLINE
    float y = coeff[3];
    y = y * x + coeff[2];
    y = y * x + coeff[1];
    y = y * x + coeff[0];
    return y;
}

void poly(hls::stream<axis_word_t>& in_stream, hls::stream<axis_word_t>& out_stream) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

    PolyCmdHdr cmd_hdr;
    cmd_hdr.read_axi4_stream<WORD_BW>(in_stream);

    PolyRespHdr resp_hdr;
    resp_hdr.tx_id = cmd_hdr.tx_id;
    resp_hdr.write_axi4_stream<WORD_BW>(out_stream, false);

    static float x[MAX_NSAMP], y[MAX_NSAMP];
#pragma HLS ARRAY_PARTITION variable=x type=cyclic factor=UNROLL_FACTOR  dim=1
#pragma HLS ARRAY_PARTITION variable=y type=cyclic factor=UNROLL_FACTOR  dim=1

    // Read to the input buffer
    load_loop:  for (int i=0; i < (int)cmd_hdr.nsamp; ++i) {
        auto word = in_stream.read();
        x[i] = streamutils::uint_to_float(word.data);
    }

    proc_loop:  for (int i = 0; i < cmd_hdr.nsamp; i++ ) {
#pragma HLS loop_tripcount min=1 max=MAX_NSAMP
#if UNROLL_FACTOR > 1
#pragma HLS unroll factor=UNROLL_FACTOR
#else
#pragma HLS pipeline  
#endif
        y[i] = eval_poly_horner(cmd_hdr.coeffs.data, x[i]);
    }

    // Write the output to the stream
    store_loop: for (int i=0; i < (int)cmd_hdr.nsamp; ++i) {
        axis_word_t word;
        word.data = streamutils::float_to_uint(y[i]);
        word.last = (i == cmd_hdr.nsamp - 1) ? 1 : 0;
        out_stream.write(word);
    }


    PolyRespFtr resp_ftr;
    resp_ftr.nsamp_read = cmd_hdr.nsamp;
    resp_ftr.error = PolyError::NO_ERROR;
    resp_ftr.write_axi4_stream<WORD_BW>(out_stream);
}
