#include "cmd_hdr.h"
#include "resp_hdr.h"
#include "resp_footer.h"
#include "stream_utils.h"
#include "poly_block.h"
/*
    Block polynomial streaming kernel.

    Input stream format is a header of type 
    
    CmdHdr 
    followed by n 32-bit float data samples:
        x[0], ..., x[n-1] (with TLAST=1 on x[n-1])

    Output stream format:

    RespHdr
    y[0], ..., y[nread-1] (with TLAST=1 on y[nread-1])
    RespFooter

    There is an SYNC_ERR if TLAST is seen before n samples are read,
    or if the header is incomplete 
    (i.e. TLAST is seen before the full CmdHdr is read). In case of SYNC_ERR, 
    the kernel will flush all input until the next TLAST before processing the next transaction.
    where:
        y = a0 + a1*x + a2*(x*x)
*/
void poly_block(
    hls::stream<stream_t>& cmd_stream, 
    hls::stream<stream_t>& resp_stream)
{
#pragma HLS INTERFACE axis port=cmd_stream
#pragma HLS INTERFACE axis port=resp_stream
#pragma HLS INTERFACE ap_ctrl_none port=return


    while (1) {
#ifndef __SYNTHESIS__
        if (cmd_stream.empty()) {
            break;
        }
#endif

        CmdHdr cmd_hdr;

        // Read command from input FIFO using stream_read
        bool tlast = cmd_hdr.stream_read<stream_t>(cmd_stream);


        // Write the response header with the echo-ed transaction ID
        // TLAST is not set on the response header since there will
        // be data words following the header
        RespHdr resp_hdr;
        resp_hdr.trans_id = cmd_hdr.trans_id;
        resp_hdr.stream_write<stream_t>(resp_stream, false);

        // Create the response footer
        RespFooter resp_footer;
        resp_footer.nread = 0; // to be updated later
        resp_footer.err_code = RespFooter::NO_ERR; // default to no error, may be updated later

        // For this protocol, command header TLAST is expected to be false
        // since sample words follow the header in the same transaction.
        // A true TLAST here indicates malformed framing.
        if (tlast) {
            
            // Write error response header with SYNC_ERR code
            resp_footer.err_code = RespFooter::SYNC_ERR;
            resp_footer.stream_write<stream_t>(resp_stream, true); // Write footer with error code and TLAST=1 to indicate end of response

            // Drain until TLAST to resync
            while (!cmd_stream.empty()) {
                auto w = cmd_stream.read();
                if (w.last) break;
            }
            continue; // skip normal processing
        }


        // Perform calculation
        float a0 = cmd_hdr.a0;
        float a1 = cmd_hdr.a1;
        float a2 = cmd_hdr.a2;
        int nread = 0;

        proc_loop:  for (int i = 0; i < (int)cmd_hdr.n; ++i) {
            // Read next input sample
            auto w = cmd_stream.read();
            float xin = uint_to_float(w.data);

            // Compute the polynomial
            float y = a0 + a1 * xin + a2 * xin * xin;

            // Increment sample count and write output word
            nread++;
            stream_t y_word = axi_word<stream_t>(float_to_uint(y)); 
            resp_stream.write(y_word);
        }

    
        // Write the response footer
        resp_footer.nread = nread;
        resp_footer.err_code = RespFooter::NO_ERR;
        resp_footer.stream_write<stream_t>(resp_stream, true);
    }

}
