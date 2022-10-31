//#define MSC_CLASS "crtc_client"

#include "crtc_client.h"
 

#include <rtc_base/helpers.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/time_utils.h>
#include <sstream>

namespace crtc_client {
	void Initialize() // NOLINT(readability-identifier-naming)
	{
		 

		rtc::InitializeSSL();
		rtc::InitRandom(rtc::Time());
	}

	void Cleanup() // NOLINT(readability-identifier-naming)
	{
 

		rtc::CleanupSSL();
	}

	 
} // namespace crtc_client
