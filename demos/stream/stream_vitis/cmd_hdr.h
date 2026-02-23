#ifndef CMD_HDR_H
#define CMD_HDR_H

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include "stream_utils.h"
#include <string>
#include <sstream>

class CmdHdr {
public:

    ap_int<16> trans_id; // Transaction ID
    float a0; // Coefficient a0
    float a1; // Coefficient a1
    float a2; // Coefficient a2
    ap_uint<32> n; // Number of data points

    template<typename Tstream>
    bool stream_read_32(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in CmdHdr::stream_read_32");

        Tstream w0 = in.read();
        trans_id = w0.data.range(15, 0);
        Tstream w1 = in.read();
        a0 = uint_to_float(w1.data);
        Tstream w2 = in.read();
        a1 = uint_to_float(w2.data);
        Tstream w3 = in.read();
        a2 = uint_to_float(w3.data);
        Tstream w4 = in.read();
        n = w4.data;
        bool tlast = w4.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_32(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in CmdHdr::stream_write_32");

        out.write(axi_word_range<Tstream>(trans_id, 15, 0));

        out.write(axi_word<Tstream>(float_to_uint(a0)));

        out.write(axi_word<Tstream>(float_to_uint(a1)));

        out.write(axi_word<Tstream>(float_to_uint(a2)));

        out.write(axi_word<Tstream>(n, tlast));

    }

    template<typename Tstream>
    bool stream_read_64(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in CmdHdr::stream_read_64");

        Tstream w0 = in.read();
        trans_id = w0.data.range(15, 0);
        a0 = uint_to_float(w0.data.range(47, 16));
        Tstream w1 = in.read();
        a1 = uint_to_float(w1.data.range(31, 0));
        a2 = uint_to_float(w1.data.range(63, 32));
        Tstream w2 = in.read();
        n = w2.data.range(31, 0);
        bool tlast = w2.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_64(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in CmdHdr::stream_write_64");

        Tstream w0 = axi_word_range<Tstream>(trans_id, 15, 0);
        w0.data.range(47, 16) = float_to_uint(a0);
        out.write(w0);

        Tstream w1 = axi_word<Tstream>(float_to_uint(a1));
        w1.data.range(63, 32) = float_to_uint(a2);
        out.write(w1);

        out.write(axi_word<Tstream>(n, tlast));

    }

    template<typename Tstream>
    void stream_write(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        if constexpr (bus_bits == 32) {
            stream_write_32(out, tlast);
        } else if constexpr (bus_bits == 64) {
            stream_write_64(out, tlast);
        } else {
            static_assert(bus_bits == 32, 
                         "Unsupported bus width. Supported widths: 32, 64");
        }
    }

    template<typename Tstream>
    bool stream_read(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        if constexpr (bus_bits == 32) {
            return stream_read_32(in);
        } else if constexpr (bus_bits == 64) {
            return stream_read_64(in);
        } else {
            static_assert(bus_bits == 32, 
                         "Unsupported bus width. Supported widths: 32, 64");
            return false;
        }
    }

    bool operator==(const CmdHdr& other) const {
        return (this->trans_id == other.trans_id) && (this->a0 == other.a0) && (this->a1 == other.a1) && (this->a2 == other.a2) && (this->n == other.n);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "{";
        oss << "trans_id: " << trans_id << ", ";
        oss << "a0: " << a0 << ", ";
        oss << "a1: " << a1 << ", ";
        oss << "a2: " << a2 << ", ";
        oss << "n: " << n;
        oss << "}";
        return oss.str();
    }

};

#endif // CMD_HDR_H
