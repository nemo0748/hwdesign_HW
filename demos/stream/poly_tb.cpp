#include <cstdint>
#include <string>
#include <stdexcept>
#include <cmath>

#include "poly.hpp"
#include "include/streamutils_tb.h"

int main(int argc, char** argv) {
    const std::string data_dir = (argc > 1) ? argv[1] : "data";

    static const int nsamp = 10;

    // Place in some command data
    std::vector<float> coeffs = {1.0f, 2.0f, 0.5f, -1.0f};
    PolyCmdHdr cmd_hdr;
    cmd_hdr.tx_id = 0x1234;
    cmd_hdr.nsamp = nsamp;
    memcpy(cmd_hdr.coeffs.data, coeffs.data(), 4 * sizeof(float));

    // Put in some input data uniformly spaced between 0 and 1
    SampDataIn samp_in;
    for (int i = 0; i < nsamp; ++i) {
        samp_in.data[i] = (float) i / nsamp;
    }

    // Compute the expecteed output using the same polynomial evaluation as the DUT
    SampDataOut samp_out_exp;
    for (int i = 0; i < nsamp; ++i) {
        float x = samp_in.data[i];
        samp_out_exp.data[i] = coeffs[0]
                            + coeffs[1] * x
                            + coeffs[2] * x * x
                            + coeffs[3] * x * x * x;
    }

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

    // Compare the DUT sample out with the expected output
    float mae = 0.;
    for (int i = 0; i < nsamp; ++i) {
        mae +=  std::fabs(samp_out.data[i] - samp_out_exp.data[i]);
    }
    mae /= nsamp;
    std::cout << "Mean Absolute Error (MAE): " << mae << std::endl;

    return 0;
}
