#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "line_inter.hpp"


void line_inter(hls::stream<axis_word_t>& in_stream, hls::stream<axis_word_t>& out_stream) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

    // Read the command header from the input stream and track whether TLAST arrives
    // at the expected boundary for the header payload.
    CmdHdr cmd_hdr;
    streamutils::tlast_status cmd_hdr_tlast = streamutils::tlast_status::no_tlast;
    cmd_hdr.read_axi4_stream<WORD_BW>(in_stream, cmd_hdr_tlast);

    // TODO:  Transfer the variables a, b, and uav to local variables
    //    float a[ndim], b[ndim], uab[ndim];
    //    dab = ...
    // Make sure to add #pragma HLS ARRAY_PARTITION directives to fully partition these arrays for simultaneous access 
    // to all dimensions in the pipelined loop below.
    // Copy the values from the command header to local variables for use in the pipelined loop .  This loop should also have a 
    // pragma for pipelining
    

    // TODO:  Return a response header immediately so the test bench can correlate the output
    // stream with the transaction ID from the command.
    //   RespHdr resp_hdr;
    //   resp_hdr.tx_id = cmd_hdr.tx_id;
    //   resp_hdr.write_axi4_stream<WORD_BW>(...);


    // TODO:  Create local arrays for the input samples and output results.  
    //     float x[max_nsamp][ndim];
    // Add appropriate #pragma HLS ARRAY_PARTITION directives for x so that it can access dimensions at the same time


    // TODO:  Read the data from in_stream into the x
    //    float32_array_utils::read_axi4_stream<WORD_BW>(...)
    

    // TODO:  Create a vector for the closet point on the line segment to each input sample
    //   float z[ndim];
    // Add appropriate #pragma HLS ARRAY_PARTITION directives for z so that it can access


    // TODO:  Run the main intersection computation with a pipelined loop with II.  
    // Fully unroll any nested loops.
    // Label the loop `compute_loop` so the test bench can correlate the performance results 
    // with this loop.
    //
    // compute_loop: for (int i = 0; i < nsamp; ++i) {
    //    ...
    // }
    

    // TODO: Write the output samples back to the output stream.
    //   float32_array_utils::write_axi4_stream<WORD_BW>(...)
   

    // Summarize how many samples were consumed and classify any stream-boundary errors
    // after the pipelined loop has completed.
    RespFtr resp_ftr;
    resp_ftr.nelem_read = nelem_read;
    resp_ftr.error = IntersectError::NO_ERROR;
    bool need_flush = false;
    if (cmd_hdr_tlast == streamutils::tlast_status::tlast_early) {
        resp_ftr.error = IntersectError::TLAST_EARLY_CMD_HDR;
    } else if (cmd_hdr_tlast == streamutils::tlast_status::no_tlast) {
        resp_ftr.error = IntersectError::NO_TLAST_CMD_HDR;
        need_flush = true;
    } else if (cmd_hdr.nsamp == 0) {
        resp_ftr.nelem_read = 0;
    } else if (samp_in_tlast == streamutils::tlast_status::tlast_early) {
        resp_ftr.error = IntersectError::TLAST_EARLY_SAMP_IN;
    } else if (samp_in_tlast == streamutils::tlast_status::no_tlast) {
        resp_ftr.error = IntersectError::NO_TLAST_SAMP_IN;
        need_flush = true;
    } else if (nelem_read != ndim * nsamp) {
        resp_ftr.error = IntersectError::WRONG_NSAMP;
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
