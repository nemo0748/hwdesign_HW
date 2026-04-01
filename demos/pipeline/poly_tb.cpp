#include <cstdint>
#include <string>
#include <stdexcept>
#include <cmath>

#include "poly.hpp"
#include "include/streamutils_tb.h"

int main(int argc, char** argv) {
    const std::string data_dir = (argc > 1) ? argv[1] : "data";

    // Read the input test vectors
    PolyCmdHdr cmd_hdr;
    streamutils::read_uint32_file(cmd_hdr, (data_dir + "/cmd_hdr_data.bin").c_str());

    const int nsamp = cmd_hdr.nsamp;
    SampDataIn samp_in;
    streamutils::read_uint32_file_len(samp_in, (data_dir + "/samp_in_data.bin").c_str(), nsamp);

    // Create the streams to the DUT
    hls::stream<axis_word_t> in_stream;
    hls::stream<axis_word_t> out_stream;

    // Write the command header and input samples to the DUT input stream
    cmd_hdr.write_axi4_stream<WORD_BW>(in_stream, false);
    samp_in.write_axi4_stream<WORD_BW>(in_stream, true, nsamp);

    // Run the DUT that will write to the output stream
    poly(in_stream, out_stream);

    // Read the response header, output samples, and response footer from the DUT output stream
    PolyRespHdr resp_hdr;
    resp_hdr.read_axi4_stream<WORD_BW>(out_stream);

    SampDataOut samp_out;
    samp_out.read_axi4_stream<WORD_BW>(out_stream, nsamp);

    PolyRespFtr resp_ftr;
    resp_ftr.read_axi4_stream<WORD_BW>(out_stream);

    // Compare the DUT sample out with the golden model
    SampDataOut samp_out_exp;
    streamutils::read_uint32_file_len(samp_out_exp, (data_dir + "/samp_out_data.bin").c_str(), nsamp);
    float mae = 0.;
    for (int i = 0; i < nsamp; ++i) {
        mae +=  std::fabs(samp_out.data[i] - samp_out_exp.data[i]);
    }
    mae /= nsamp;
    std::cout << "Mean Absolute Error (MAE): " << mae << std::endl;


    // Write all the data files to the data directory
    streamutils::write_uint32_file(resp_hdr, (data_dir + "/dut_resp_hdr_data.bin").c_str());
    streamutils::write_uint32_file_len(samp_out, (data_dir + "/dut_samp_out_data.bin").c_str(), nsamp);
    streamutils::write_uint32_file(resp_ftr, (data_dir + "/dut_resp_ftr_data.bin").c_str());

    return 0;
}
