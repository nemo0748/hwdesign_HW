#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "poly.hpp"


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

    // Read the command header from the input stream and track whether TLAST arrives
    // at the expected boundary for the header payload.
    PolyCmdHdr cmd_hdr;
    streamutils::tlast_status cmd_hdr_tlast = streamutils::tlast_status::no_tlast;
    cmd_hdr.read_axi4_stream<WORD_BW>(in_stream, cmd_hdr_tlast);

    // Return a response header immediately so the test bench can correlate the output
    // stream with the transaction ID from the command.
    PolyRespHdr resp_hdr;
    resp_hdr.tx_id = cmd_hdr.tx_id;
    resp_hdr.write_axi4_stream<WORD_BW>(out_stream, true);

    // To illustrate the pipelining, we read the entire array of input samples
    // into on-chip storage, then process and write out the results.  This is not
    // the most efficient way to implement this function, but it allows us to
    // examine the pipelining behavior without dealing with AXI stream handshakes.
    // The input and output buffers are partitioned to allow simultaneous access to all
    // samples in a lane of the input and output stream.
    float x[max_nsamp];
    float y[max_nsamp];
#pragma HLS ARRAY_PARTITION variable=x type=cyclic factor=UNROLL_FACTOR  dim=1
#pragma HLS ARRAY_PARTITION variable=y type=cyclic factor=UNROLL_FACTOR  dim=1



    int nsamp = cmd_hdr.nsamp;
    if (nsamp > max_nsamp) {
        nsamp = max_nsamp;
    }
    streamutils::tlast_status samp_in_tlast;
    int nsamp_read = 0;
    float32_array_utils::read_axi4_stream<WORD_BW>(in_stream, x, samp_in_tlast, nsamp_read, nsamp);

    // Run the main polynomial evaluation loop.  The loop is pipelined to II=1, 
    // so a new sample can be processed every cycle after the pipeline is filled.
    compute_loop:  for (int i = 0; i < nsamp; ++i) {
#pragma HLS loop_tripcount min=1 max=max_nsamp
#if UNROLL_FACTOR > 1
#pragma HLS unroll factor=UNROLL_FACTOR        
#else
#pragma HLS pipeline II=1
#endif
        y[i] = eval_poly_horner(cmd_hdr.coeffs.data, x[i]);
    }

    // Write the output samples back to the output stream.
    float32_array_utils::write_axi4_stream<WORD_BW>(out_stream, y, true, nsamp);


    // Summarize how many samples were consumed and classify any stream-boundary errors
    // after the pipelined loop has completed.
    PolyRespFtr resp_ftr;
    resp_ftr.nsamp_read = nsamp_read;
    resp_ftr.error = PolyError::NO_ERROR;
    bool need_flush = false;
    if (cmd_hdr_tlast == streamutils::tlast_status::tlast_early) {
        resp_ftr.error = PolyError::TLAST_EARLY_CMD_HDR;
    } else if (cmd_hdr_tlast == streamutils::tlast_status::no_tlast) {
        resp_ftr.error = PolyError::NO_TLAST_CMD_HDR;
        need_flush = true;
    } else if (cmd_hdr.nsamp == 0) {
        resp_ftr.nsamp_read = 0;
    } else if (samp_in_tlast == streamutils::tlast_status::tlast_early) {
        resp_ftr.error = PolyError::TLAST_EARLY_SAMP_IN;
    } else if (samp_in_tlast == streamutils::tlast_status::no_tlast) {
        resp_ftr.error = PolyError::NO_TLAST_SAMP_IN;
        need_flush = true;
    } else if (nsamp_read != nsamp) {
        resp_ftr.error = PolyError::WRONG_NSAMP;
    }

    // If TLAST has not yet been seen for a malformed input message, drain words until the
    // next TLAST boundary so the following transaction starts aligned on the input stream.
    if (need_flush) {
        streamutils::flush_axi4_stream_to_tlast<WORD_BW>(in_stream);
    }

    // Terminate the response footer with TLAST so the test bench can detect the end of
    // the final response message independently of the payload stream.
    resp_ftr.write_axi4_stream<WORD_BW>(out_stream, true);
}
