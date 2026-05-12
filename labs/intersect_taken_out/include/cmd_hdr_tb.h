#ifndef INCLUDE_CMD_HDR_TB_H
#define INCLUDE_CMD_HDR_TB_H

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include "streamutils_tb.h"

#include "point_tb.h"

#define PYSILICON_ENABLE_INCLUDE_CMD_HDR_TB_H_MEMBERS
#include "cmd_hdr.h"
#undef PYSILICON_ENABLE_INCLUDE_CMD_HDR_TB_H_MEMBERS

inline void CmdHdr::dump_json(std::ostream& os, int indent, int level) const {
    const int step = (indent < 0) ? 0 : indent;
    os << "{";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"tx_id\": ";
    os << static_cast<unsigned long long>(this->tx_id);
    os << ",";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"a\": ";
    os << "[";
    for (int i0 = 0; i0 < 3; ++i0) {
    if (i0 > 0) { os << ","; }
    os << this->a.data[i0];
    }
    os << "]";
    os << ",";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"b\": ";
    os << "[";
    for (int i0 = 0; i0 < 3; ++i0) {
    if (i0 > 0) { os << ","; }
    os << this->b.data[i0];
    }
    os << "]";
    os << ",";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"uab\": ";
    os << "[";
    for (int i0 = 0; i0 < 3; ++i0) {
    if (i0 > 0) { os << ","; }
    os << this->uab.data[i0];
    }
    os << "]";
    os << ",";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"dab\": ";
    os << this->dab;
    os << ",";
    os << "\n";
    for (int i = 0; i < (level + 1) * step; ++i) { os << ' '; }
    os << "\"nsamp\": ";
    os << static_cast<unsigned long long>(this->nsamp);
    os << "\n";
    for (int i = 0; i < (level) * step; ++i) { os << ' '; }
    os << "}";
}

inline void CmdHdr::load_json(const std::string& json_text, size_t& pos) {
    streamutils::json_expect_char(json_text, pos, '{');
    bool seen_root_tx_id = false;
    bool seen_root_a = false;
    bool seen_root_b = false;
    bool seen_root_uab = false;
    bool seen_root_dab = false;
    bool seen_root_nsamp = false;
    bool first = true;
    while (true) {
    streamutils::json_skip_ws(json_text, pos);
    if (pos < json_text.size() && json_text[pos] == '}') {
        ++pos;
        break;
    }
    if (!first) {
        streamutils::json_expect_char(json_text, pos, ',');
    }
    first = false;
    std::string key = streamutils::json_parse_string(json_text, pos);
    streamutils::json_expect_char(json_text, pos, ':');
    if (key == "tx_id") {
        seen_root_tx_id = true;
        this->tx_id = static_cast<ap_uint<16>>(static_cast<unsigned long long>(streamutils::json_parse_number(json_text, pos)));
    }
    else if (key == "a") {
        seen_root_a = true;
        streamutils::json_expect_char(json_text, pos, '[');
        for (int i0 = 0; i0 < 3; ++i0) {
            if (i0 > 0) {
                streamutils::json_expect_char(json_text, pos, ',');
            }
            this->a.data[i0] = static_cast<float>(streamutils::json_parse_number(json_text, pos));
        }
        streamutils::json_expect_char(json_text, pos, ']');
    }
    else if (key == "b") {
        seen_root_b = true;
        streamutils::json_expect_char(json_text, pos, '[');
        for (int i0 = 0; i0 < 3; ++i0) {
            if (i0 > 0) {
                streamutils::json_expect_char(json_text, pos, ',');
            }
            this->b.data[i0] = static_cast<float>(streamutils::json_parse_number(json_text, pos));
        }
        streamutils::json_expect_char(json_text, pos, ']');
    }
    else if (key == "uab") {
        seen_root_uab = true;
        streamutils::json_expect_char(json_text, pos, '[');
        for (int i0 = 0; i0 < 3; ++i0) {
            if (i0 > 0) {
                streamutils::json_expect_char(json_text, pos, ',');
            }
            this->uab.data[i0] = static_cast<float>(streamutils::json_parse_number(json_text, pos));
        }
        streamutils::json_expect_char(json_text, pos, ']');
    }
    else if (key == "dab") {
        seen_root_dab = true;
        this->dab = static_cast<float>(streamutils::json_parse_number(json_text, pos));
    }
    else if (key == "nsamp") {
        seen_root_nsamp = true;
        this->nsamp = static_cast<ap_uint<16>>(static_cast<unsigned long long>(streamutils::json_parse_number(json_text, pos)));
    }
    else {
        throw std::runtime_error("Malformed JSON: unexpected key for schema.");
    }
    }
    if (!seen_root_tx_id) {
    throw std::runtime_error("Malformed JSON: missing required key 'tx_id'.");
    }
    if (!seen_root_a) {
    throw std::runtime_error("Malformed JSON: missing required key 'a'.");
    }
    if (!seen_root_b) {
    throw std::runtime_error("Malformed JSON: missing required key 'b'.");
    }
    if (!seen_root_uab) {
    throw std::runtime_error("Malformed JSON: missing required key 'uab'.");
    }
    if (!seen_root_dab) {
    throw std::runtime_error("Malformed JSON: missing required key 'dab'.");
    }
    if (!seen_root_nsamp) {
    throw std::runtime_error("Malformed JSON: missing required key 'nsamp'.");
    }
}

inline void CmdHdr::load_json(std::istream& is) {
    std::string json_text((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    size_t pos = 0;
    streamutils::json_skip_ws(json_text, pos);
    this->load_json(json_text, pos);
    streamutils::json_skip_ws(json_text, pos);
    if (pos != json_text.size()) {
        throw std::runtime_error("Trailing characters after JSON object.");
    }
}

inline void CmdHdr::dump_json_file(const char* file_path, int indent) const {
    std::ofstream ofs(file_path);
    if (!ofs) {
        throw std::runtime_error("Failed to open output JSON file.");
    }
    this->dump_json(ofs, indent);
}

inline void CmdHdr::load_json_file(const char* file_path) {
    std::ifstream ifs(file_path);
    if (!ifs) {
        throw std::runtime_error("Failed to open input JSON file.");
    }
    this->load_json(ifs);
}

#endif // INCLUDE_CMD_HDR_TB_H