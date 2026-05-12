#ifndef INCLUDE_INTERSECT_ERROR_H
#define INCLUDE_INTERSECT_ERROR_H

#include <ap_int.h>
#include <hls_stream.h>
#if __has_include(<hls_axi_stream.h>)
#include <hls_axi_stream.h>
#else
#include <ap_axi_sdata.h>
#endif
#include "streamutils_hls.h"

enum class IntersectError {
    NO_ERROR = 0,
    TLAST_EARLY_CMD_HDR = 1,
    NO_TLAST_CMD_HDR = 2,
    TLAST_EARLY_SAMP_IN = 3,
    NO_TLAST_SAMP_IN = 4,
    WRONG_NSAMP = 5,
};

#endif // INCLUDE_INTERSECT_ERROR_H