#ifndef POLY_STREAM_H
#define POLY_STREAM_H

#include <hls_stream.h>

void avgfilt(hls::stream<float> &x_stream,
             hls::stream<float> &y_stream);

// Fixed coefficients
constexpr int win_size = 3;

#endif
