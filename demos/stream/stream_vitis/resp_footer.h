#ifndef RESP_FOOTER_H
#define RESP_FOOTER_H

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include "stream_utils.h"
#include <string>
#include <sstream>

class RespFooter {
public:

    enum ErrCodes : unsigned int {
        NO_ERR = 0,
        SYNC_ERR = 1
    };

    ap_uint<32> nread; // Number of data points read before TLAST
    ap_uint<8> err_code; // Error Code

    template<typename Tstream>
    bool stream_read_32(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in RespFooter::stream_read_32");

        Tstream w0 = in.read();
        nread = w0.data;
        Tstream w1 = in.read();
        err_code = w1.data.range(7, 0);
        bool tlast = w1.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_32(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 32, "Only 32-bit stream supported in RespFooter::stream_write_32");

        out.write(axi_word<Tstream>(nread));

        out.write(axi_word_range<Tstream>(err_code, 7, 0, tlast));

    }

    template<typename Tstream>
    bool stream_read_64(hls::stream<Tstream>& in) {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in RespFooter::stream_read_64");

        Tstream w0 = in.read();
        nread = w0.data.range(31, 0);
        err_code = w0.data.range(39, 32);
        bool tlast = w0.last;

        return tlast;
    }

    template<typename Tstream>
    void stream_write_64(hls::stream<Tstream>& out, bool tlast = true) const {
        constexpr int bus_bits = decltype(Tstream::data)::width;
        static_assert(bus_bits == 64, "Only 64-bit stream supported in RespFooter::stream_write_64");

        Tstream w0 = axi_word<Tstream>(nread, tlast);
        w0.data.range(39, 32) = err_code;
        out.write(w0);

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

    bool operator==(const RespFooter& other) const {
        return (this->nread == other.nread) && (this->err_code == other.err_code);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "{";
        oss << "nread: " << nread << ", ";
        oss << "err_code: " << err_code;
        oss << "}";
        return oss.str();
    }

};

#endif // RESP_FOOTER_H
