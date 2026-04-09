#include "streamutils_hls.h"

namespace streamutils {

const char* tlast_status_info::names[tlast_status_info::count] = {
    "no_tlast",
    "tlast_at_end",
    "tlast_early",
};

} // namespace streamutils