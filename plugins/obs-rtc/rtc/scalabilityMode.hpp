#ifndef MSC_SCALABILITY_MODE_HPP
#define MSC_SCALABILITY_MODE_HPP
#include "json.hpp"
#include <string>

namespace crtc_client {
	nlohmann::json parseScalabilityMode(const std::string& scalabilityMode);
} // namespace crtc_client

#endif
