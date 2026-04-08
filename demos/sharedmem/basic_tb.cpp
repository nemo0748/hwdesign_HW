#include <algorithm>
#include <cstdlib>
#include <iostream>

#include <ap_int.h>
#include <hls_stream.h>

using data_t = int;

static constexpr int kMemoryDepth = 1024;

// Keep these protocol messages identical to the kernel definitions in basic.cpp.
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

void basic(hls::stream<Cmd>& in_stream, hls::stream<Resp>& out_stream, volatile data_t* shared_mem);

static void tb_write_burst(data_t* shared_mem, unsigned int base_addr, const data_t* src, unsigned int length) {
    for (unsigned int i = 0; i < length; ++i) {
        shared_mem[base_addr + i] = src[i];
    }
}

static void tb_read_burst(const data_t* shared_mem, unsigned int base_addr, data_t* dst, unsigned int length) {
    for (unsigned int i = 0; i < length; ++i) {
        dst[i] = shared_mem[base_addr + i];
    }
}

static data_t transform_sample(data_t sample, int gain, int bias) {
    return gain * sample + bias;
}

int main() {
    static constexpr unsigned int kReadBase = 32;
    static constexpr unsigned int kWriteBase = 160;
    static constexpr unsigned int kNumWords = 8;

    data_t shared_mem[kMemoryDepth] = {};
    const data_t input_data[kNumWords] = {3, -2, 7, 0, 5, 11, -4, 6};
    data_t output_data[kNumWords] = {};
    data_t expected_data[kNumWords] = {};

    const int gain = 3;
    const int bias = -2;
    int expected_checksum = 0;
    for (unsigned int i = 0; i < kNumWords; ++i) {
        expected_data[i] = transform_sample(input_data[i], gain, bias);
        expected_checksum += expected_data[i];
    }

    // The test bench models its own AXI master writes into the common memory.
    tb_write_burst(shared_mem, kReadBase, input_data, kNumWords);

    hls::stream<Cmd> cmd_stream("cmd_stream");
    hls::stream<Resp> resp_stream("resp_stream");

    Cmd cmd;
    cmd.read_addr = kReadBase;
    cmd.write_addr = kWriteBase;
    cmd.length = kNumWords;
    cmd.gain = gain;
    cmd.bias = bias;
    cmd_stream.write(cmd);

    basic(cmd_stream, resp_stream, shared_mem);

    if (resp_stream.empty()) {
        std::cerr << "Missing response from kernel\n";
        return EXIT_FAILURE;
    }

    const Resp resp = resp_stream.read();
    tb_read_burst(shared_mem, kWriteBase, output_data, kNumWords);

    bool pass = true;

    if (resp.words_done != kNumWords) {
        std::cerr << "Unexpected words_done: " << resp.words_done << "\n";
        pass = false;
    }

    if (resp.status != 0) {
        std::cerr << "Kernel reported non-zero status: " << resp.status << "\n";
        pass = false;
    }

    if (resp.checksum != expected_checksum) {
        std::cerr << "Checksum mismatch: got " << resp.checksum
                  << ", expected " << expected_checksum << "\n";
        pass = false;
    }

    for (unsigned int i = 0; i < kNumWords; ++i) {
        std::cout << "out[" << i << "] = " << output_data[i]
                  << ", expected = " << expected_data[i] << "\n";
        if (output_data[i] != expected_data[i]) {
            pass = false;
        }
    }

    if (!resp_stream.empty()) {
        std::cerr << "Unexpected extra response messages\n";
        pass = false;
    }

    if (!pass) {
        std::cerr << "basic_tb FAILED\n";
        return EXIT_FAILURE;
    }

    std::cout << "basic_tb PASSED\n";
    return EXIT_SUCCESS;
}