#ifndef INCLUDE_FLOAT32_ARRAY_UTILS_H
#define INCLUDE_FLOAT32_ARRAY_UTILS_H

#include <ap_int.h>
#include <hls_stream.h>
#if __has_include(<hls_axi_stream.h>)
#include <hls_axi_stream.h>
#else
#include <ap_axi_sdata.h>
#endif
#include "streamutils_hls.h"

namespace float32_array_utils {

using value_type = float;
static constexpr int value_bitwidth = 32;

template<int>
struct unsupported_word_bw { static constexpr bool value = false; };

template<int word_bw>
static constexpr int get_nwords(int len) {
    return (len <= 0) ? 0 : ((len * value_bitwidth + word_bw - 1) / word_bw);
}

template<int word_bw>
static constexpr int pf() {
    return word_bw / 32;
}

template<int word_bw>
struct read_array_elem_impl {
    static void run(const ap_uint<word_bw>* src, value_type* out, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for read_array_elem");
        (void)src;
        (void)out;
        (void)n;
    }
};

template<>
struct read_array_elem_impl<32> {
    static void run(const ap_uint<32>* src, value_type* out, int n) {
        #pragma HLS INLINE
        if (n > 0 && src != nullptr) {
            out[0] = streamutils::uint_to_float((uint32_t)(src[0]));
        }
    }
};

template<>
struct read_array_elem_impl<64> {
    static void run(const ap_uint<64>* src, value_type* out, int n) {
        #pragma HLS INLINE
        if (src == nullptr) {
            return;
        }
        ap_uint<64> w = src[0];
        if (n > 0) {
            out[0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
        }
        if (n > 1) {
            out[1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
        }
    }
};

template<int word_bw>
inline void read_array_elem(const ap_uint<word_bw>* src, value_type out[pf<word_bw>()], int n = pf<word_bw>()) {
    #pragma HLS INLINE
    read_array_elem_impl<word_bw>::run(src, out, n);
}

template<int word_bw>
struct write_array_elem_impl {
    static void run(const value_type* in, ap_uint<word_bw>* dst, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for write_array_elem");
        (void)in;
        (void)dst;
        (void)n;
    }
};

template<>
struct write_array_elem_impl<32> {
    static void run(const value_type* in, ap_uint<32>* dst, int n) {
        #pragma HLS INLINE
        if (n > 0 && dst != nullptr) {
            dst[0] = streamutils::float_to_uint(in[0]);
        }
    }
};

template<>
struct write_array_elem_impl<64> {
    static void run(const value_type* in, ap_uint<64>* dst, int n) {
        #pragma HLS INLINE
        if (dst == nullptr) {
            return;
        }
        ap_uint<64> w = 0;
        if (n > 0) {
            w.range(31, 0) = streamutils::float_to_uint(in[0]);
        }
        if (n > 1) {
            w.range(63, 32) = streamutils::float_to_uint(in[1]);
        }
        dst[0] = w;
    }
};

template<int word_bw>
inline void write_array_elem(const value_type in[pf<word_bw>()], ap_uint<word_bw>* dst, int n = pf<word_bw>()) {
    #pragma HLS INLINE
    write_array_elem_impl<word_bw>::run(in, dst, n);
}

template<int word_bw>
struct read_stream_elem_impl {
    static void run(hls::stream<ap_uint<word_bw>>& s, value_type* out, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for read_stream_elem");
        (void)s;
        (void)out;
        (void)n;
    }
};

template<>
struct read_stream_elem_impl<32> {
    static void run(hls::stream<ap_uint<32>>& s, value_type* out, int n) {
        #pragma HLS INLINE
        if (n > 0) {
            ap_uint<32> w = s.read();
            out[0] = streamutils::uint_to_float((uint32_t)(w));
        }
    }
};

template<>
struct read_stream_elem_impl<64> {
    static void run(hls::stream<ap_uint<64>>& s, value_type* out, int n) {
        #pragma HLS INLINE
        ap_uint<64> w = s.read();
        if (n > 0) {
            out[0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
        }
        if (n > 1) {
            out[1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
        }
    }
};

template<int word_bw>
inline void read_stream_elem(hls::stream<ap_uint<word_bw>>& s, value_type out[pf<word_bw>()], int n = pf<word_bw>()) {
    #pragma HLS INLINE
    read_stream_elem_impl<word_bw>::run(s, out, n);
}

template<int word_bw>
struct read_axi4_stream_elem_impl {
    static void run(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type* out, streamutils::tlast_status& tl, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for read_axi4_stream_elem");
        (void)s;
        (void)out;
        (void)tl;
        (void)n;
    }
};

template<>
struct read_axi4_stream_elem_impl<32> {
    static void run(hls::stream<streamutils::axi4s_word<32>>& s, value_type* out, streamutils::tlast_status& tl, int n) {
        #pragma HLS INLINE
        tl = streamutils::tlast_status::no_tlast;
        if (n > 0) {
            auto axis_word = s.read();
            ap_uint<32> w = axis_word.data;
            out[0] = streamutils::uint_to_float((uint32_t)(w));
            if (axis_word.last) {
                tl = streamutils::tlast_status::tlast_at_end;
            }
        }
    }
};

template<>
struct read_axi4_stream_elem_impl<64> {
    static void run(hls::stream<streamutils::axi4s_word<64>>& s, value_type* out, streamutils::tlast_status& tl, int n) {
        #pragma HLS INLINE
        tl = streamutils::tlast_status::no_tlast;
        auto axis_word = s.read();
        ap_uint<64> w = axis_word.data;
        if (n > 0) {
            out[0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
        }
        if (n > 1) {
            out[1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
        }
        if (axis_word.last) {
            tl = streamutils::tlast_status::tlast_at_end;
        }
    }
};

template<int word_bw>
inline void read_axi4_stream_elem(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type out[pf<word_bw>()], streamutils::tlast_status& tl, int n = pf<word_bw>()) {
    #pragma HLS INLINE
    read_axi4_stream_elem_impl<word_bw>::run(s, out, tl, n);
}

template<int word_bw>
inline void read_axi4_stream_elem(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type out[pf<word_bw>()], int n = pf<word_bw>()) {
    #pragma HLS INLINE
    streamutils::tlast_status tl = streamutils::tlast_status::no_tlast;
    read_axi4_stream_elem<word_bw>(s, out, tl, n);
}

template<int word_bw>
struct write_stream_elem_impl {
    static void run(hls::stream<ap_uint<word_bw>>& s, const value_type* in, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for write_stream_elem");
        (void)s;
        (void)in;
        (void)n;
    }
};

template<>
struct write_stream_elem_impl<32> {
    static void run(hls::stream<ap_uint<32>>& s, const value_type* in, int n) {
        #pragma HLS INLINE
        if (n > 0) {
            ap_uint<32> w = streamutils::float_to_uint(in[0]);
            s.write(w);
        }
    }
};

template<>
struct write_stream_elem_impl<64> {
    static void run(hls::stream<ap_uint<64>>& s, const value_type* in, int n) {
        #pragma HLS INLINE
        ap_uint<64> w = 0;
        if (n > 0) {
            w.range(31, 0) = streamutils::float_to_uint(in[0]);
        }
        if (n > 1) {
            w.range(63, 32) = streamutils::float_to_uint(in[1]);
        }
        s.write(w);
    }
};

template<int word_bw>
inline void write_stream_elem(hls::stream<ap_uint<word_bw>>& s, const value_type in[pf<word_bw>()], int n = pf<word_bw>()) {
    #pragma HLS INLINE
    write_stream_elem_impl<word_bw>::run(s, in, n);
}

template<int word_bw>
struct write_axi4_stream_elem_impl {
    static void run(hls::stream<streamutils::axi4s_word<word_bw>>& s, const value_type* in, bool tlast, int n) {
        static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for write_axi4_stream_elem");
        (void)s;
        (void)in;
        (void)tlast;
        (void)n;
    }
};

template<>
struct write_axi4_stream_elem_impl<32> {
    static void run(hls::stream<streamutils::axi4s_word<32>>& s, const value_type* in, bool tlast, int n) {
        #pragma HLS INLINE
        if (n > 0) {
            ap_uint<32> w = streamutils::float_to_uint(in[0]);
            streamutils::write_axi4_word<32>(s, w, tlast);
        }
    }
};

template<>
struct write_axi4_stream_elem_impl<64> {
    static void run(hls::stream<streamutils::axi4s_word<64>>& s, const value_type* in, bool tlast, int n) {
        #pragma HLS INLINE
        ap_uint<64> w = 0;
        if (n > 0) {
            w.range(31, 0) = streamutils::float_to_uint(in[0]);
        }
        if (n > 1) {
            w.range(63, 32) = streamutils::float_to_uint(in[1]);
        }
        streamutils::write_axi4_word<64>(s, w, tlast);
    }
};

template<int word_bw>
inline void write_axi4_stream_elem(hls::stream<streamutils::axi4s_word<word_bw>>& s, const value_type in[pf<word_bw>()], bool tlast = false, int n = pf<word_bw>()) {
    #pragma HLS INLINE
    write_axi4_stream_elem_impl<word_bw>::run(s, in, tlast, n);
}

template<int word_bw>
inline void read_stream(hls::stream<ap_uint<word_bw>>& s, value_type* dst, int len) {
    #pragma HLS INLINE
    if (dst == nullptr || len <= 0) {
        return;
    }
    for (int i = 0; i < len; i += pf<word_bw>()) {
        read_stream_elem<word_bw>(s, dst + i, len - i);
    }
}

template<int word_bw>
inline void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type* dst, streamutils::tlast_status& tl, int& nread, int len) {
    #pragma HLS INLINE
    tl = streamutils::tlast_status::no_tlast;
    nread = 0;
    if (dst == nullptr || len <= 0) {
        return;
    }
    bool stop = false;
    for (int i = 0; i < len && !stop; i += pf<word_bw>()) {
        streamutils::tlast_status lane_tl = streamutils::tlast_status::no_tlast;
        const int lane_count = ((len - i) < pf<word_bw>()) ? (len - i) : pf<word_bw>();
        read_axi4_stream_elem<word_bw>(s, dst + i, lane_tl, len - i);
        if (lane_tl == streamutils::tlast_status::tlast_early) {
            tl = lane_tl;
            stop = true;
        }
        if (lane_tl != streamutils::tlast_status::tlast_early) {
            nread += lane_count;
        }
        if (lane_tl == streamutils::tlast_status::tlast_at_end) {
            tl = (i + pf<word_bw>() >= len) ? streamutils::tlast_status::tlast_at_end : streamutils::tlast_status::tlast_early;
            stop = true;
        }
    }
}

template<int word_bw>
inline void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type* dst, streamutils::tlast_status& tl, int len) {
    #pragma HLS INLINE
    int nread = 0;
    read_axi4_stream<word_bw>(s, dst, tl, nread, len);
}

template<int word_bw>
inline void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type* dst, int& nread, int len) {
    #pragma HLS INLINE
    streamutils::tlast_status tl = streamutils::tlast_status::no_tlast;
    read_axi4_stream<word_bw>(s, dst, tl, nread, len);
    }

template<int word_bw>
inline void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>>& s, value_type* dst, int len) {
    #pragma HLS INLINE
    streamutils::tlast_status tl = streamutils::tlast_status::no_tlast;
    int nread = 0;
    read_axi4_stream<word_bw>(s, dst, tl, nread, len);
    }

template<int word_bw>
inline void write_stream(hls::stream<ap_uint<word_bw>>& s, const value_type* src, int len) {
    #pragma HLS INLINE
    if (src == nullptr || len <= 0) {
        return;
    }
    for (int i = 0; i < len; i += pf<word_bw>()) {
        write_stream_elem<word_bw>(s, src + i, len - i);
    }
}

template<int word_bw>
inline void write_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>>& s, const value_type* src, bool tlast = true, int len = pf<word_bw>()) {
    #pragma HLS INLINE
    if (src == nullptr || len <= 0) {
        return;
    }
    for (int i = 0; i < len; i += pf<word_bw>()) {
        const bool lane_tlast = (i + pf<word_bw>() >= len) ? tlast : false;
        write_axi4_stream_elem<word_bw>(s, src + i, lane_tlast, len - i);
    }
}

/**
 * @brief Read an array of float values from packed ap_uint words.
 *
 * Elements are unpacked greedily from least-significant bits first with no
 * padding between adjacent elements, matching the PySilicon DataSchema array
 * packing convention.
 *
 * @tparam word_bw Packed source word width in bits.
 * @param src Pointer to packed source words.
 * @param dst Pointer to the destination array.
 * @param len Number of elements to decode.
 */
template<int word_bw>
inline void read_array(const ap_uint<word_bw>* src, value_type* dst, int len) {
    static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for float32_array_utils::read_array");
    (void)src;
    (void)dst;
    (void)len;
}

/**
 * @brief Write an array of float values into packed ap_uint words.
 *
 * Elements are packed greedily from least-significant bits first with no
 * padding between adjacent elements, matching the PySilicon DataSchema array
 * packing convention.
 *
 * @tparam word_bw Packed destination word width in bits.
 * @param src Pointer to the source array.
 * @param dst Pointer to the packed destination words.
 * @param len Number of elements to encode.
 */
template<int word_bw>
inline void write_array(const value_type* src, ap_uint<word_bw>* dst, int len) {
    static_assert(unsupported_word_bw<word_bw>::value, "Unsupported word_bw for float32_array_utils::write_array");
    (void)src;
    (void)dst;
    (void)len;
}

/**
 * @brief Read an array of float values from packed 32-bit words.
 *
 * The packed input uses greedy LSB-first packing with no inter-element padding.
 * This specialization is optimized for word_bw = 32.
 *
 * @param src Pointer to packed source words.
 * @param dst Pointer to the destination array.
 * @param len Number of elements to decode.
 */
template<>
inline void read_array<32>(const ap_uint<32>* src, value_type* dst, int len) {
    #pragma HLS INLINE
    if (src == nullptr || dst == nullptr || len <= 0) {
        return;
    }

    int in_idx = 0;
    for (int i = 0; i < len; ++i) {
        #pragma HLS PIPELINE II=1
        dst[i] = streamutils::uint_to_float((uint32_t)(src[in_idx]));
        ++in_idx;
    }
}

/**
 * @brief Write an array of float values into packed 32-bit words.
 *
 * The packed output uses greedy LSB-first packing with no inter-element padding.
 * This specialization is optimized for word_bw = 32.
 *
 * @param src Pointer to the source array.
 * @param dst Pointer to the packed destination words.
 * @param len Number of elements to encode.
 */
template<>
inline void write_array<32>(const value_type* src, ap_uint<32>* dst, int len) {
    #pragma HLS INLINE
    if (src == nullptr || dst == nullptr || len <= 0) {
        return;
    }

    int out_idx = 0;
    for (int in_idx = 0; in_idx < len; ++in_idx) {
        #pragma HLS PIPELINE II=1
        dst[out_idx++] = streamutils::float_to_uint(src[in_idx]);
    }
}

/**
 * @brief Read an array of float values from packed 64-bit words.
 *
 * The packed input uses greedy LSB-first packing with no inter-element padding.
 * This specialization is optimized for word_bw = 64.
 *
 * @param src Pointer to packed source words.
 * @param dst Pointer to the destination array.
 * @param len Number of elements to decode.
 */
template<>
inline void read_array<64>(const ap_uint<64>* src, value_type* dst, int len) {
    #pragma HLS INLINE
    if (src == nullptr || dst == nullptr || len <= 0) {
        return;
    }

    int in_idx = 0;
    for (int i = 0; i < len; i += 2) {
        #pragma HLS PIPELINE II=1
        ap_uint<64> w = src[in_idx++];
        for (int j = 0; j < 2; ++j) {
            #pragma HLS UNROLL
            if (i + j < len) {
                if (j == 0) {
                    dst[i + j] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                else if (j == 1) {
                    dst[i + j] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
    }
}

/**
 * @brief Write an array of float values into packed 64-bit words.
 *
 * The packed output uses greedy LSB-first packing with no inter-element padding.
 * This specialization is optimized for word_bw = 64.
 *
 * @param src Pointer to the source array.
 * @param dst Pointer to the packed destination words.
 * @param len Number of elements to encode.
 */
template<>
inline void write_array<64>(const value_type* src, ap_uint<64>* dst, int len) {
    #pragma HLS INLINE
    if (src == nullptr || dst == nullptr || len <= 0) {
        return;
    }

    int out_idx = 0;
    for (int i = 0; i < len; i += 2) {
        #pragma HLS PIPELINE II=1
        ap_uint<64> w = 0;
        for (int j = 0; j < 2; ++j) {
            #pragma HLS UNROLL
            if (i + j < len) {
                if (j == 0) {
                    w.range(31, 0) = streamutils::float_to_uint(src[i + 0]);
                }
                else if (j == 1) {
                    w.range(63, 32) = streamutils::float_to_uint(src[i + 1]);
                }
            }
        }
        dst[out_idx++] = w;
    }
}

}  // namespace float32_array_utils

#endif // INCLUDE_FLOAT32_ARRAY_UTILS_H
