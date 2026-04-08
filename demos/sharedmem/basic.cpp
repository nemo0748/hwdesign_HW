#include <ap_int.h>
#include <hls_stream.h>

using data_t = int;

static constexpr int kMemoryDepth = 1024;
static constexpr int kMaxTransferWords = 64;

// The addresses are word offsets into the shared-memory image passed on the
// AXI master port.  Using offsets keeps the C test bench simple while still
// illustrating the protocol used by a host and accelerator.
struct Cmd {
    ap_uint<32> read_addr;
    ap_uint<32> write_addr;
    ap_uint<16> length;
    ap_int<16> gain;
    ap_int<16> bias;
};

struct Resp {
    ap_uint<16> words_done;
    ap_uint<8> status;
    ap_int<32> checksum;
};

static data_t transform_sample(data_t sample, ap_int<16> gain, ap_int<16> bias) {
#pragma HLS INLINE
    return gain * sample + bias;
}

void basic(
    hls::stream<Cmd>& in_stream,
    hls::stream<Resp>& out_stream,
    volatile data_t* shared_mem) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE m_axi port=shared_mem depth=kMemoryDepth offset=slave bundle=gmem \
    max_read_burst_length=16 max_write_burst_length=16
#pragma HLS INTERFACE s_axilite port=shared_mem bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    data_t local_buf[kMaxTransferWords];

    while (true) {
#ifndef __SYNTHESIS__
        if (in_stream.empty()) {
            break;
        }
#endif

        const Cmd cmd = in_stream.read();
        const unsigned int requested_len = cmd.length;
        const unsigned int read_base = cmd.read_addr;
        const unsigned int write_base = cmd.write_addr;

        unsigned int effective_len = requested_len;
        if (effective_len > static_cast<unsigned int>(kMaxTransferWords)) {
            effective_len = kMaxTransferWords;
        }

        if (read_base >= static_cast<unsigned int>(kMemoryDepth)
                || write_base >= static_cast<unsigned int>(kMemoryDepth)) {
            effective_len = 0;
        } else {
            const unsigned int max_read_len = kMemoryDepth - read_base;
            const unsigned int max_write_len = kMemoryDepth - write_base;
            if (effective_len > max_read_len) {
                effective_len = max_read_len;
            }
            if (effective_len > max_write_len) {
                effective_len = max_write_len;
            }
        }

        read_loop:
        for (unsigned int i = 0; i < effective_len; ++i) {
#pragma HLS PIPELINE II=1
            local_buf[i] = shared_mem[read_base + i];
        }

        ap_int<32> checksum = 0;
        compute_loop:
        for (unsigned int i = 0; i < effective_len; ++i) {
#pragma HLS PIPELINE II=1
            const data_t result = transform_sample(local_buf[i], cmd.gain, cmd.bias);
            local_buf[i] = result;
            checksum += result;
        }

        write_loop:
        for (unsigned int i = 0; i < effective_len; ++i) {
#pragma HLS PIPELINE II=1
            shared_mem[write_base + i] = local_buf[i];
        }

        Resp resp;
        resp.words_done = effective_len;
        resp.status = (effective_len == requested_len) ? 0 : 1;
        resp.checksum = checksum;
        out_stream.write(resp);
    }
}