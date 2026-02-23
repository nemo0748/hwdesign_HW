#ifndef RESP_HDR_H
#define RESP_HDR_H

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include "stream_utils.h"
#include <string>
#include <sstream>

class RespHdr {
public:

    ap_int<16> trans_id; // Transaction ID

    template<typename Tstream>
    bool stream_read_32(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in RespHdr::stream_read_32");

        Tstream w0 = in.read();
        trans_id = w0.data.range(15, 0);
        bool tlast = w0.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_32(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in RespHdr::stream_write_32");

        out.write(axi_word_range<Tstream>(trans_id, 15, 0, tlast));

    }

    template<typename Tstream>
    bool stream_read_64(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in RespHdr::stream_read_64");

        Tstream w0 = in.read();
        trans_id = w0.data.range(15, 0);
        bool tlast = w0.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_64(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in RespHdr::stream_write_64");

        out.write(axi_word_range<Tstream>(trans_id, 15, 0, tlast));

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

    bool operator==(const RespHdr& other) const {
        return (this->trans_id == other.trans_id);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "{";
        oss << "trans_id: " << trans_id;
        oss << "}";
        return oss.str();
    }

};

#endif // RESP_HDR_H
