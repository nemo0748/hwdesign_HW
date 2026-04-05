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

    // Read the command header from the input stream
    PolyCmdHdr cmd_hdr;
    cmd_hdr.read_axi4_stream<WORD_BW>(in_stream);

    // Write the response header to the output stream with the same transaction ID as the command header
    // Mark the header word as the last beat for this single-word write.
    PolyRespHdr resp_hdr;
    resp_hdr.tx_id = cmd_hdr.tx_id;
    resp_hdr.write_axi4_stream<WORD_BW>(out_stream, true);

    // We will read the input samples in chunks of the packing factor (number of samples that fit in one AXI word), 
    // and write the output samples in the same way. This is more efficient than reading/writing one sample at a time,
    // since it takes advantage of the full AXI bandwidth and allows for better pipelining. 
    // The arrays x_lane and y_lane hold one chunk of input and output samples, respectively.
    static const int pf = float32_array_utils::pf<WORD_BW>();
    float x_lane[pf];
    float y_lane[pf];
#pragma HLS ARRAY_PARTITION variable=x_lane complete dim=1
#pragma HLS ARRAY_PARTITION variable=y_lane complete dim=1

    // Process the input samples one packing factor at a time. For each chunk, we read the input samples from the stream, compute the output samples using the polynomial evaluation, and write the output samples to the stream. We also need to keep track of how many samples are left to process (nrem) in order to know when to assert tlast on the output stream.
    int nrem = cmd_hdr.nsamp;
    for (int i = 0; i < cmd_hdr.nsamp; i += pf) {

        // Read a chunk of input samples from the stream. The utility function read_axi4_stream_elem reads 
        // up to pf samples from the stream and stores them in x_lane, and returns the number of samples read 
        // (which may be less than pf for the last chunk).
        float32_array_utils::read_axi4_stream_elem<WORD_BW>(in_stream, x_lane, nrem);

        // Compute the output samples for this chunk using the polynomial evaluation. We only compute for the valid
        // samples in this chunk (up to nrem).  The unroll directive allows us to compute all samples in the chunk in parallel, 
        // which can improve performance.
        for (int k = 0; k < pf; ++k) {
#pragma HLS UNROLL
            if (k < nrem) {
                y_lane[k] = eval_poly_horner(cmd_hdr.coeffs.data, x_lane[k]);
            }
        }

        // Write the data
        bool tlast = (nrem <= pf);
        float32_array_utils::write_axi4_stream_elem<WORD_BW>(out_stream, y_lane, tlast, nrem);

        nrem -= pf;
    }

    // Write the response footer to the output stream. The response footer includes the number of samples read and an 
    //error code. In this example, we set the error code to NO_ERROR, but in a real implementation, you would set it based on 
    // any errors that occurred during processing (e.g., invalid command header, etc.).
    PolyRespFtr resp_ftr;
    resp_ftr.nsamp_read = cmd_hdr.nsamp;
    resp_ftr.error = PolyError::NO_ERROR;
    resp_ftr.write_axi4_stream<WORD_BW>(out_stream, true);
}

