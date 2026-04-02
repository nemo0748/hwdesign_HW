/* Utilities for AXI stream manipulation */
#ifndef STREAM_UTILS_H
#define STREAM_UTILS_H

union FloatBits {
    float f;
    unsigned int u;
};

static unsigned int float_to_uint(float x) {
    FloatBits fb;
    fb.f = x;
    return fb.u;
};

static float uint_to_float(unsigned int a) {
    FloatBits fb;
    fb.u = a;
    return fb.f;
};

template <typename Tstream>
Tstream axi_word(ap_uint<32> value, bool last=false) {
    Tstream w;
    w.data = 0;
    w.keep = -1;
    w.strb = -1;
    w.data = value;
    w.last = last;
    return w;
};

template <typename Tstream>
Tstream axi_word_range(ap_uint<32> value, int hi, int lo, bool last=false) {
    Tstream w;
    w.data = 0;
    w.keep = -1;
    w.strb = -1;
    w.data.range(hi, lo) = value;
    w.last = last;
    return w;
};

#endif // STREAM_UTILS_H  