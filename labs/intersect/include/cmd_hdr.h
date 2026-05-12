#ifndef INCLUDE_CMD_HDR_H
#define INCLUDE_CMD_HDR_H

#include <ap_int.h>
#include <hls_stream.h>
#if __has_include(<hls_axi_stream.h>)
#include <hls_axi_stream.h>
#else
#include <ap_axi_sdata.h>
#endif
#include "streamutils_hls.h"

#include "point.h"

struct CmdHdr {
    ap_uint<16> tx_id;  // Transaction ID
    Point a;  // Start point of the line segment
    Point b;  // End point of the line segment
    Point uab;  // Direction vector of the line segment, b-a/(||b-a||)
    float dab;  // Length of the line segment, ||b-a||
    ap_uint<16> nsamp;  // Number of samples x

    static constexpr int bitwidth = 352;

    template<int word_bw>
    struct word_bw_tag {};

    template<int word_bw>
    static constexpr int nwords_value(word_bw_tag<word_bw>) {
            static_assert(word_bw < 0, "Unsupported word_bw for nwords");
            return 0;
    }

    static constexpr int nwords_value(word_bw_tag<32>) {
            return 12;
    }

    static constexpr int nwords_value(word_bw_tag<64>) {
            return 6;
    }

    template<int word_bw>
    static constexpr int nwords() {
        return nwords_value(word_bw_tag<word_bw>{});
    }

    static ap_uint<bitwidth> pack_to_uint(const CmdHdr& data) {
        ap_uint<bitwidth> res = 0;
        res.range(15, 0) = data.tx_id;
        res.range(111, 16) = Point::pack_to_uint(data.a);
        res.range(207, 112) = Point::pack_to_uint(data.b);
        res.range(303, 208) = Point::pack_to_uint(data.uab);
        res.range(335, 304) = streamutils::float_to_uint(data.dab);
        res.range(351, 336) = data.nsamp;
        return res;
    }

    static CmdHdr unpack_from_uint(const ap_uint<bitwidth>& packed) {
        CmdHdr data;
        data.tx_id = (ap_uint<16>)(packed.range(15, 0));
        data.a = Point::unpack_from_uint(packed.range(111, 16));
        data.b = Point::unpack_from_uint(packed.range(207, 112));
        data.uab = Point::unpack_from_uint(packed.range(303, 208));
        data.dab = streamutils::uint_to_float((uint32_t)(packed.range(335, 304)));
        data.nsamp = (ap_uint<16>)(packed.range(351, 336));
        return data;
    }

    template<int word_bw>
    static void write_array_impl(word_bw_tag<word_bw>, const CmdHdr* self, ap_uint<word_bw> x[]) {
        static_assert(word_bw < 0, "Unsupported word_bw for write_array");
        (void)self;
        (void)x;
    }

    static void write_array_impl(word_bw_tag<32>, const CmdHdr* self, ap_uint<32> x[]) {
        x[0] = 0;
        x[0].range(15, 0) = self->tx_id;
        {
            const int n0_eff = 3;
            int out_idx = 1;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                x[out_idx++] = streamutils::float_to_uint(self->a.data[i0]);
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 4;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                x[out_idx++] = streamutils::float_to_uint(self->b.data[i0]);
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 7;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                x[out_idx++] = streamutils::float_to_uint(self->uab.data[i0]);
            }
        }
        x[10] = streamutils::float_to_uint(self->dab);
        x[11] = 0;
        x[11].range(15, 0) = self->nsamp;
    }

    static void write_array_impl(word_bw_tag<64>, const CmdHdr* self, ap_uint<64> x[]) {
        x[0] = 0;
        x[0].range(15, 0) = self->tx_id;
        {
            const int n0_eff = 3;
            int out_idx = 1;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->a.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->a.data[i + 1]);
                }
                x[out_idx++] = w;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 3;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->b.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->b.data[i + 1]);
                }
                x[out_idx++] = w;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 5;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->uab.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->uab.data[i + 1]);
                }
                x[out_idx++] = w;
            }
        }
        x[7] = 0;
        x[7].range(31, 0) = streamutils::float_to_uint(self->dab);
        x[7].range(47, 32) = self->nsamp;
    }

    template<int word_bw>
    void write_array(ap_uint<word_bw> x[]) const {
        write_array_impl(word_bw_tag<word_bw>{}, this, x);
    }

    template<int word_bw>
    static void write_stream_impl(word_bw_tag<word_bw>, const CmdHdr* self, hls::stream<ap_uint<word_bw>> &s) {
        static_assert(word_bw < 0, "Unsupported word_bw for write_stream");
        (void)self;
        (void)s;
    }

    static void write_stream_impl(word_bw_tag<32>, const CmdHdr* self, hls::stream<ap_uint<32>> &s) {
            ap_uint<32> w = 0;
        w.range(15, 0) = self->tx_id;
        {
            s.write(w);
            w = 0;
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->a.data[i0]);
                s.write(w);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->b.data[i0]);
                s.write(w);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->uab.data[i0]);
                s.write(w);
                out_idx++;
            }
        }
        w = streamutils::float_to_uint(self->dab);
        s.write(w);
        w = 0;
        w.range(15, 0) = self->nsamp;
        s.write(w);
    }

    static void write_stream_impl(word_bw_tag<64>, const CmdHdr* self, hls::stream<ap_uint<64>> &s) {
            ap_uint<64> w = 0;
        w.range(15, 0) = self->tx_id;
        {
            s.write(w);
            w = 0;
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->a.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->a.data[i + 1]);
                }
                s.write(w);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->b.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->b.data[i + 1]);
                }
                s.write(w);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->uab.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->uab.data[i + 1]);
                }
                s.write(w);
                out_idx++;
            }
        }
        w.range(31, 0) = streamutils::float_to_uint(self->dab);
        w.range(47, 32) = self->nsamp;
        s.write(w);
    }

    template<int word_bw>
    void write_stream(hls::stream<ap_uint<word_bw>> &s) const {
        write_stream_impl(word_bw_tag<word_bw>{}, this, s);
    }

    template<int word_bw>
    static void write_axi4_stream_impl(word_bw_tag<word_bw>, const CmdHdr* self, hls::stream<streamutils::axi4s_word<word_bw>> &s, bool tlast) {
        static_assert(word_bw < 0, "Unsupported word_bw for write_axi4_stream");
        (void)self;
        (void)s;
        (void)tlast;
    }

    static void write_axi4_stream_impl(word_bw_tag<32>, const CmdHdr* self, hls::stream<streamutils::axi4s_word<32>> &s, bool tlast) {
            ap_uint<32> w = 0;
        w.range(15, 0) = self->tx_id;
        {
            streamutils::write_axi4_word<32>(s, w, false);
            w = 0;
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->a.data[i0]);
                streamutils::write_axi4_word<32>(s, w, false);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->b.data[i0]);
                streamutils::write_axi4_word<32>(s, w, false);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = streamutils::float_to_uint(self->uab.data[i0]);
                streamutils::write_axi4_word<32>(s, w, false);
                out_idx++;
            }
        }
        w = streamutils::float_to_uint(self->dab);
        streamutils::write_axi4_word<32>(s, w, false);
        w = 0;
        w.range(15, 0) = self->nsamp;
        streamutils::write_axi4_word<32>(s, w, tlast);
    }

    static void write_axi4_stream_impl(word_bw_tag<64>, const CmdHdr* self, hls::stream<streamutils::axi4s_word<64>> &s, bool tlast) {
            ap_uint<64> w = 0;
        w.range(15, 0) = self->tx_id;
        {
            streamutils::write_axi4_word<64>(s, w, false);
            w = 0;
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->a.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->a.data[i + 1]);
                }
                streamutils::write_axi4_word<64>(s, w, false);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->b.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->b.data[i + 1]);
                }
                streamutils::write_axi4_word<64>(s, w, false);
                out_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int out_idx = 0;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = 0;
                if (i + 0 < n0_eff) {
                    w.range(31, 0) = streamutils::float_to_uint(self->uab.data[i + 0]);
                }
                if (i + 1 < n0_eff) {
                    w.range(63, 32) = streamutils::float_to_uint(self->uab.data[i + 1]);
                }
                streamutils::write_axi4_word<64>(s, w, false);
                out_idx++;
            }
        }
        w.range(31, 0) = streamutils::float_to_uint(self->dab);
        w.range(47, 32) = self->nsamp;
        streamutils::write_axi4_word<64>(s, w, tlast);
    }

    template<int word_bw>
    void write_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>> &s, bool tlast = true) const {
        write_axi4_stream_impl(word_bw_tag<word_bw>{}, this, s, tlast);
    }

    template<int word_bw>
    static void read_array_impl(word_bw_tag<word_bw>, CmdHdr* self, const ap_uint<word_bw> x[]) {
        static_assert(word_bw < 0, "Unsupported word_bw for read_array");
        (void)self;
        (void)x;
    }

    static void read_array_impl(word_bw_tag<32>, CmdHdr* self, const ap_uint<32> x[]) {
        self->tx_id = (ap_uint<16>)(x[0].range(15, 0));
        {
            const int n0_eff = 3;
            int in_idx = 1;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                self->a.data[i0] = streamutils::uint_to_float((uint32_t)(x[in_idx]));
                in_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 4;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                self->b.data[i0] = streamutils::uint_to_float((uint32_t)(x[in_idx]));
                in_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 7;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                self->uab.data[i0] = streamutils::uint_to_float((uint32_t)(x[in_idx]));
                in_idx++;
            }
        }
        self->dab = streamutils::uint_to_float((uint32_t)(x[10]));
        self->nsamp = (ap_uint<16>)(x[11].range(15, 0));
    }

    static void read_array_impl(word_bw_tag<64>, CmdHdr* self, const ap_uint<64> x[]) {
        self->tx_id = (ap_uint<16>)(x[0].range(15, 0));
        {
            const int n0_eff = 3;
            int in_idx = 1;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = x[in_idx++];
                if (i + 0 < n0_eff) {
                    self->a.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->a.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 3;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = x[in_idx++];
                if (i + 0 < n0_eff) {
                    self->b.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->b.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 5;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                ap_uint<64> w = x[in_idx++];
                if (i + 0 < n0_eff) {
                    self->uab.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->uab.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        self->dab = streamutils::uint_to_float((uint32_t)(x[7].range(31, 0)));
        self->nsamp = (ap_uint<16>)(x[7].range(47, 32));
    }

    template<int word_bw>
    void read_array(const ap_uint<word_bw> x[]) {
        read_array_impl(word_bw_tag<word_bw>{}, this, x);
    }

    template<int word_bw>
    static void read_stream_impl(word_bw_tag<word_bw>, CmdHdr* self, hls::stream<ap_uint<word_bw>> &s) {
        static_assert(word_bw < 0, "Unsupported word_bw for read_stream");
        (void)self;
        (void)s;
    }

    static void read_stream_impl(word_bw_tag<32>, CmdHdr* self, hls::stream<ap_uint<32>> &s) {
            ap_uint<32> w = 0;
        w = s.read();
        self->tx_id = (ap_uint<16>)(w.range(15, 0));
        {
            const int n0_eff = 3;
            int in_idx = 1;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = s.read();
                self->a.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 4;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = s.read();
                self->b.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 7;
            for (int i0 = 0; i0 < n0_eff; ++i0) {
                w = s.read();
                self->uab.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
            }
        }
        w = s.read();
        self->dab = streamutils::uint_to_float((uint32_t)(w));
        w = s.read();
        self->nsamp = (ap_uint<16>)(w.range(15, 0));
    }

    static void read_stream_impl(word_bw_tag<64>, CmdHdr* self, hls::stream<ap_uint<64>> &s) {
            ap_uint<64> w = 0;
        w = s.read();
        self->tx_id = (ap_uint<16>)(w.range(15, 0));
        {
            const int n0_eff = 3;
            int in_idx = 1;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = s.read();
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->a.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->a.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 3;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = s.read();
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->b.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->b.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        {
            const int n0_eff = 3;
            int in_idx = 5;
            for (int i = 0; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                w = s.read();
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->uab.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->uab.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
            }
        }
        w = s.read();
        self->dab = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
        self->nsamp = (ap_uint<16>)(w.range(47, 32));
    }

    template<int word_bw>
    void read_stream(hls::stream<ap_uint<word_bw>> &s) {
        read_stream_impl(word_bw_tag<word_bw>{}, this, s);
    }

    template<int word_bw>
    static void read_axi4_stream_impl(word_bw_tag<word_bw>, CmdHdr* self, hls::stream<streamutils::axi4s_word<word_bw>> &s, streamutils::tlast_status &tl) {
        static_assert(word_bw < 0, "Unsupported word_bw for read_axi4_stream");
        (void)self;
        (void)s;
        (void)tl;
    }

    static void read_axi4_stream_impl(word_bw_tag<32>, CmdHdr* self, hls::stream<streamutils::axi4s_word<32>> &s, streamutils::tlast_status &tl) {
            ap_uint<32> w = 0;
            tl = streamutils::tlast_status::no_tlast;
            bool last = false;
        if (last) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            auto axis_word = s.read();
            w = axis_word.data;
            last = axis_word.last;
        }
        self->tx_id = (ap_uint<16>)(w.range(15, 0));
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 1;
            int elem_idx = 0;
            bool stop = false;
            for (int i0 = 0; i0 < n0_eff && !stop; ++i0) {
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                self->a.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
                elem_idx++;
                if (last && elem_idx < (n0_eff)) {
                    stop = true;
                }
            }
            if (stop) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 4;
            int elem_idx = 0;
            bool stop = false;
            for (int i0 = 0; i0 < n0_eff && !stop; ++i0) {
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                self->b.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
                elem_idx++;
                if (last && elem_idx < (n0_eff)) {
                    stop = true;
                }
            }
            if (stop) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 7;
            int elem_idx = 0;
            bool stop = false;
            for (int i0 = 0; i0 < n0_eff && !stop; ++i0) {
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                self->uab.data[i0] = streamutils::uint_to_float((uint32_t)(w));
                in_idx++;
                elem_idx++;
                if (last && elem_idx < (n0_eff)) {
                    stop = true;
                }
            }
            if (stop) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        if (last) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            auto axis_word = s.read();
            w = axis_word.data;
            last = axis_word.last;
        }
        self->dab = streamutils::uint_to_float((uint32_t)(w));
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        if (last) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            auto axis_word = s.read();
            w = axis_word.data;
            last = axis_word.last;
        }
        self->nsamp = (ap_uint<16>)(w.range(15, 0));
        if (tl != streamutils::tlast_status::no_tlast) {
            return;
        }
        if (last) {
            tl = streamutils::tlast_status::tlast_at_end;
        }
    }

    static void read_axi4_stream_impl(word_bw_tag<64>, CmdHdr* self, hls::stream<streamutils::axi4s_word<64>> &s, streamutils::tlast_status &tl) {
            ap_uint<64> w = 0;
            tl = streamutils::tlast_status::no_tlast;
            bool last = false;
        if (last) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            auto axis_word = s.read();
            w = axis_word.data;
            last = axis_word.last;
        }
        self->tx_id = (ap_uint<16>)(w.range(15, 0));
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 1;
            int i = 0;
            for (; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                if (last) {
                    break;
                }
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->a.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->a.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
                if (last) {
                    break;
                }
            }
            if ((i + 2) < n0_eff) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 3;
            int i = 0;
            for (; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                if (last) {
                    break;
                }
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->b.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->b.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
                if (last) {
                    break;
                }
            }
            if ((i + 2) < n0_eff) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            const int n0_eff = 3;
            int in_idx = 5;
            int i = 0;
            for (; i < n0_eff; i += 2) {
                #pragma HLS PIPELINE II=1
                if (last) {
                    break;
                }
                {
                    auto axis_word = s.read();
                    w = axis_word.data;
                    last = axis_word.last;
                }
                in_idx++;
                if (i + 0 < n0_eff) {
                    self->uab.data[i + 0] = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
                }
                if (i + 1 < n0_eff) {
                    self->uab.data[i + 1] = streamutils::uint_to_float((uint32_t)(w.range(63, 32)));
                }
                if (last) {
                    break;
                }
            }
            if ((i + 2) < n0_eff) {
                tl = streamutils::tlast_status::tlast_early;
                return;
            }
        }
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        if (last) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        {
            auto axis_word = s.read();
            w = axis_word.data;
            last = axis_word.last;
        }
        self->dab = streamutils::uint_to_float((uint32_t)(w.range(31, 0)));
        if (tl != streamutils::tlast_status::no_tlast) {
            tl = streamutils::tlast_status::tlast_early;
            return;
        }
        self->nsamp = (ap_uint<16>)(w.range(47, 32));
        if (tl != streamutils::tlast_status::no_tlast) {
            return;
        }
        if (last) {
            tl = streamutils::tlast_status::tlast_at_end;
        }
    }

    template<int word_bw>
    void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>> &s, streamutils::tlast_status &tl) {
        read_axi4_stream_impl(word_bw_tag<word_bw>{}, this, s, tl);
    }

    template<int word_bw>
    void read_axi4_stream(hls::stream<streamutils::axi4s_word<word_bw>> &s) {
        streamutils::tlast_status tl = streamutils::tlast_status::no_tlast;
        read_axi4_stream<word_bw>(s, tl);
    }

#ifdef PYSILICON_ENABLE_INCLUDE_CMD_HDR_TB_H_MEMBERS
    void dump_json(std::ostream& os, int indent = 2, int level = 0) const;
    void load_json(const std::string& json_text, size_t& pos);
    void load_json(std::istream& is);
    void dump_json_file(const char* file_path, int indent = 2) const;
    void load_json_file(const char* file_path);
#endif
};

#endif // INCLUDE_CMD_HDR_H