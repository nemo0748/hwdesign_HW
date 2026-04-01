#include <iostream>
#include <vector>
#include <cmath>
#include "avgfilt.h"

int main() {
    hls::stream<float> in_stream("in_stream");
    hls::stream<float> out_stream("out_stream");

    const std::vector<float> x = {0.0f, 1.0f, 2.0f, -1.0f, 3.0f, 4.0f};
    std::vector<float> y_exp;
    y_exp.reserve(x.size());

    float xsq0 = 0.0f;
    float xsq1 = 0.0f;
    constexpr float inv_win_size = 1.0f / static_cast<float>(win_size);

    for (float xi : x) {
        const float xsq = xi * xi;
        const float yi = (xsq + xsq0 + xsq1) * inv_win_size;
        y_exp.push_back(yi);

        xsq1 = xsq0;
        xsq0 = xsq;
        in_stream.write(xi);
    }

    avgfilt(in_stream, out_stream);

    bool pass = true;
    const float tol = 1e-5f;

    for (std::size_t i = 0; i < y_exp.size(); ++i) {
        if (out_stream.empty()) {
            std::cout << "Missing output at index " << i << "\n";
            pass = false;
            break;
        }

        const float y = out_stream.read();
        const float diff = std::fabs(y - y_exp[i]);

        std::cout << "y[" << i << "] = " << y
                  << ", expected = " << y_exp[i]
                  << ", diff = " << diff << "\n";

        if (diff > tol) {
            pass = false;
        }
    }

    if (!out_stream.empty()) {
        std::cout << "Unexpected extra outputs in out_stream\n";
        pass = false;
    }

    if (pass) {
        std::cout << "tb_avgfilt PASSED\n";
        return 0;
    }

    std::cout << "tb_avgfilt FAILED\n";
    return 1;

}
