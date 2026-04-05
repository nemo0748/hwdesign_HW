#include <fstream>
#include <cstdint>
#include <string>
#include <stdexcept>

#include "poly.hpp"
#include "include/float32_array_utils_tb.h"
#include "include/streamutils_tb.h"

int main(int argc, char** argv) {

    // Number of samples
    static const int nsamp = 10;

    // Place in some command data
    std::vector<float> coeffs = {1.0f, 2.0f, 0.5f, -1.0f};
    PolyCmdHdr cmd_hdr;
    cmd_hdr.tx_id = 0x1234;
    cmd_hdr.nsamp = nsamp;
    memcpy(cmd_hdr.coeffs.data, coeffs.data(), 4 * sizeof(float));

    // Put in some input data uniformly spaced between 0 and 1
    float x[nsamp];
    for (int i = 0; i < nsamp; ++i) {
        x[i] = (float) i / nsamp;
    }

    // Compute the expecteed output using the same polynomial evaluation as the DUT
    float y_exp[nsamp];
    for (int i = 0; i < nsamp; ++i) {
        float x_val = x[i];
        y_exp[i] = coeffs[0] + coeffs[1] * x_val
                + coeffs[2] * x_val * x_val
                + coeffs[3] * x_val * x_val * x_val;
    }

    // Create the streams to the DUT
    hls::stream<axis_word_t> in_stream;
    hls::stream<axis_word_t> out_stream;

    // Write the command header and input samples to the DUT input stream
    cmd_hdr.write_axi4_stream<WORD_BW>(in_stream, true);

    // Write the data samples
    float32_array_utils::write_axi4_stream<WORD_BW>(in_stream, x, true, nsamp);

    // Run the DUT that will write to the output stream
    poly(in_stream, out_stream);

    // Read the response header and output samples from the accelerator.  
    // Similar to the input sample stream, the output samples are read in chunks of pf at a time, 
    // and the end of the stream is determined by checking for TLAST.
    PolyRespHdr resp_hdr;
    streamutils::tlast_status resp_hdr_tlast;
    resp_hdr.read_axi4_stream<WORD_BW>(out_stream, resp_hdr_tlast);
    if (resp_hdr_tlast != streamutils::tlast_status::tlast_at_end) {
        throw std::runtime_error("Error: TLAST was not asserted at the end of the response header");
    }

    // Check the transaction ID in the response header matches the one in the command header
    if (resp_hdr.tx_id != cmd_hdr.tx_id) {
        throw std::runtime_error("Error: Transaction ID in response header does not match command header");
    }
    

    // Read the output samples
    float y_dut[nsamp];
    streamutils::tlast_status samp_out_tlast;
    float32_array_utils::read_axi4_stream<WORD_BW>(out_stream, y_dut, samp_out_tlast, nsamp);
    if (samp_out_tlast != streamutils::tlast_status::tlast_at_end) {
        std::cout << "Error: TLAST status at end of output samples: " << static_cast<int>(samp_out_tlast) << std::endl;
        throw std::runtime_error("Error: TLAST was not asserted at the end of the output samples");
    }

    // Read the response footer (if any)
    PolyRespFtr resp_ftr;
    streamutils::tlast_status resp_ftr_tlast;
    resp_ftr.read_axi4_stream<WORD_BW>(out_stream, resp_ftr_tlast);
    if (resp_ftr_tlast != streamutils::tlast_status::tlast_at_end) {
        throw std::runtime_error("Error: TLAST was not asserted at the end of the response footer");
    }

    // Check the error code in the response footer
    if (resp_ftr.error != PolyError::NO_ERROR) {
        throw std::runtime_error("Error: DUT returned error code " + std::to_string(static_cast<unsigned int>(resp_ftr.error)));
    }

 
    // Compare the DUT sample out with the expected output
    float mae = 0.;
    for (int i = 0; i < nsamp; ++i) {
        mae +=  std::fabs(y_dut[i] - y_exp[i]);
    }
    mae /= nsamp;
    std::cout << "Mean Absolute Error (MAE): " << mae << std::endl;

    return 0;
}
