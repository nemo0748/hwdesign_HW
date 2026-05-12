#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "line_inter.hpp"
// duplicate...
// const char* streamutils::tlast_status_info::names[streamutils::tlast_status_info::count] = {
//     "no_tlast",
//     "tlast_at_end",
//     "tlast_early"
// };

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
    float a[ndim], b[ndim], uab[ndim];
#pragma HLS ARRAY_PARTITION variable=a complete dim=1
#pragma HLS ARRAY_PARTITION variable=b complete dim=1
#pragma HLS ARRAY_PARTITION variable=uab complete dim=1

    float dab = cmd_hdr.dab;

    setup_loop: for (int i = 0; i < ndim; i++) {
#pragma HLS PIPELINE II=1
        a[i] = cmd_hdr.a.data[i];
        b[i] = cmd_hdr.b.data[i];
        uab[i] = cmd_hdr.uab.data[i];
    }
    

    // TODO:  Return a response header immediately so the test bench can correlate the output
    // stream with the transaction ID from the command.
    //   RespHdr resp_hdr;
    //   resp_hdr.tx_id = cmd_hdr.tx_id;
    //   resp_hdr.write_axi4_stream<WORD_BW>(...);
    RespHdr resp_hdr;
    resp_hdr.trans_id = cmd_hdr.tx_id;
    resp_hdr.write_axi4_stream<WORD_BW>(out_stream, true);


    // TODO:  Create local arrays for the input samples and output results.  
    //     float x[max_nsamp][ndim];
    // Add appropriate #pragma HLS ARRAY_PARTITION directives for x so that it can access dimensions at the same time
    float x[max_nsamp][ndim];
    float y[max_nsamp][ndim]; //replacing
    float dsq[max_nsamp];
#pragma HLS ARRAY_PARTITION variable=x complete dim=2
#pragma HLS ARRAY_PARTITION variable=y complete dim=2


    // TODO:  Read the data from in_stream into the x
    //    float32_array_utils::read_axi4_stream<WORD_BW>(...)
    int nsamp = (cmd_hdr.nsamp > max_nsamp) ? max_nsamp : (int)cmd_hdr.nsamp;
    int nelem_read = 0;
    streamutils::tlast_status samp_in_tlast;
    //float32_array_utils::read_axi4_stream<WORD_BW>(in_stream, &x[0][0], samp_in_tlast, nelem_read, nsamp * ndim);
    float32_array_utils::read_axi4_stream<WORD_BW>(in_stream, &x[0][0], samp_in_tlast, nelem_read, nsamp * ndim);

    // TODO:  Create a vector for the closet point on the line segment to each input sample
    //   float z[ndim];
    // Add appropriate #pragma HLS ARRAY_PARTITION directives for z so that it can access
    // Note: z is used inside the compute_loop per sample.
    float z[ndim];
#pragma HLS ARRAY_PARTITION variable=z complete dim=1


    // TODO:  Run the main intersection computation with a pipelined loop with II.  
    // Fully unroll any nested loops.
    // Label the loop `compute_loop` so the test bench can correlate the performance results 
    // with this loop.
    //
    // compute_loop: for (int i = 0; i < nsamp; ++i) {
    //    ...
    // }

    compute_loop: for (int i = 0; i < nsamp; ++i) {
#pragma HLS PIPELINE II=1
        
        float dot_val = 0;
        // Project (x - a) onto uab
        dot_product_unroll: for (int j = 0; j < ndim; ++j) {
#pragma HLS UNROLL
            dot_val += (x[i][j] - a[j]) * uab[j];
        }

        // Clamp projection to segment [0, dab]
        float t = dot_val;
        if (t < 0.0f) t = 0.0f;
        if (t > dab)  t = dab;

        //added
        float dist_sq = 0;
        // Compute closest point z and output y
        coord_update_unroll: for (int j = 0; j < ndim; ++j) {
#pragma HLS UNROLL
            // z[j] = a[j] + t * uab[j];
            // y[i][j] = z[j];
            float diff = x[i][j] - (a[j] + t * uab[j]);
            dist_sq += diff * diff;
        }
        dsq[i] = dist_sq;
    }
    

    // TODO: Write the output samples back to the output stream.
    //   float32_array_utils::write_axi4_stream<WORD_BW>(...)
    // float32_array_utils::write_axi4_stream<WORD_BW>(out_stream, &y[0][0], true, nsamp * ndim);
    float32_array_utils::write_axi4_stream<WORD_BW>(out_stream, dsq, true, nsamp);

    // Summarize how many samples were consumed and classify any stream-boundary errors
    // after the pipelined loop has completed.
    RespFtr resp_ftr;
    // resp_ftr.nsamp_out = nelem_read;
    resp_ftr.nsamp_out = nsamp;
    resp_ftr.error = IntersectError::NO_ERROR;
    bool need_flush = false;
    if (cmd_hdr_tlast == streamutils::tlast_status::tlast_early) {
        resp_ftr.error = IntersectError::TLAST_EARLY_CMD_HDR;
    } else if (cmd_hdr_tlast == streamutils::tlast_status::no_tlast) {
        resp_ftr.error = IntersectError::NO_TLAST_CMD_HDR;
        need_flush = true;
    } else if (cmd_hdr.nsamp == 0) {
        resp_ftr.nsamp_out = 0;
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