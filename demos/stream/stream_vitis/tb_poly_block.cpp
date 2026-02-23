#include <iostream>
#include <vector>
#include <cmath>

#include "poly_block.h"
#include "cmd_hdr.h"
#include "resp_hdr.h"
#include "resp_footer.h"
#include "stream_utils.h"

int main() {
    hls::stream<stream_t> cmd_stream("cmd_stream");
    hls::stream<stream_t> resp_stream("resp_stream");

    CmdHdr cmd_hdr;
    cmd_hdr.trans_id = 42;
    cmd_hdr.a0 = 1.0f;
    cmd_hdr.a1 = 2.0f;
    cmd_hdr.a2 = 0.5f;
    cmd_hdr.n = 6;

    std::vector<float> x = {0.0f, 1.0f, 2.0f, -1.0f, 3.0f, 4.0f};
    std::vector<float> y_exp(cmd_hdr.n);
    for (unsigned i = 0; i < cmd_hdr.n; ++i) {
        y_exp[i] = cmd_hdr.a0 + cmd_hdr.a1 * x[i] + cmd_hdr.a2 * x[i] * x[i];
    }

    cmd_hdr.stream_write<stream_t>(cmd_stream, false);
    for (unsigned i = 0; i < cmd_hdr.n; ++i) {
        bool is_last = (i == cmd_hdr.n - 1);
        cmd_stream.write(axi_word<stream_t>(float_to_uint(x[i]), is_last));
    }

    poly_block(cmd_stream, resp_stream);

    bool pass = true;

    RespHdr resp_hdr;
    bool hdr_last = resp_hdr.stream_read<stream_t>(resp_stream);
    if (hdr_last) {
        std::cout << "Unexpected TLAST on response header\n";
        pass = false;
    }
    if (resp_hdr.trans_id != cmd_hdr.trans_id) {
        std::cout << "Transaction ID mismatch: got " << resp_hdr.trans_id
                  << " expected " << cmd_hdr.trans_id << "\n";
        pass = false;
    }

    for (unsigned i = 0; i < cmd_hdr.n; ++i) {
        stream_t w = resp_stream.read();
        float y = uint_to_float(w.data);
        float diff = std::fabs(y - y_exp[i]);

        std::cout << "y[" << i << "] = " << y
                  << ", expected = " << y_exp[i]
                  << ", diff = " << diff << "\n";

        if (w.last) {
            std::cout << "Unexpected TLAST on output sample " << i << "\n";
            pass = false;
        }
        if (diff > 1e-5f) {
            pass = false;
        }
    }

    RespFooter resp_footer;
    bool footer_last = resp_footer.stream_read<stream_t>(resp_stream);
    if (!footer_last) {
        std::cout << "Expected TLAST on response footer\n";
        pass = false;
    }
    if (resp_footer.nread != cmd_hdr.n) {
        std::cout << "nread mismatch: got " << resp_footer.nread
                  << " expected " << cmd_hdr.n << "\n";
        pass = false;
    }
    if (resp_footer.err_code != RespFooter::NO_ERR) {
        std::cout << "Unexpected err_code: " << (unsigned)resp_footer.err_code << "\n";
        pass = false;
    }

    if (!resp_stream.empty()) {
        std::cout << "Unexpected extra words in response stream\n";
        pass = false;
    }

    if (pass) {
        std::cout << "tb_poly_block PASSED\n";
        return 0;
    }

    std::cout << "tb_poly_block FAILED\n";
    return 1;
}
