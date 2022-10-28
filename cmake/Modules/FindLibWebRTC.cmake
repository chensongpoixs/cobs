# Once done these will be defined:
#
#  LIBVLC_FOUND
#  LIBVLC_INCLUDE_DIRS
#  LIBVLC_LIBRARIES
#

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(_WebRTC QUIET LibWebRTC)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_path(WEBRTC_INCLUDE_DIR
	NAMES common_types.h
	HINTS
		ENV VLCPath${_lib_suffix}
		ENV VLCPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${WebRTCPath${_lib_suffix}}
		${WebRTCPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_WEBRTC_INCLUDE_DIRS}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include
	PATH_SUFFIXES
		webrtc include include)

find_library(WEBRTC_LIB
	NAMES  ${_WEBRTC_LIBRARIES} WEBRTC LibWebRTC
	HINTS
		ENV WebRTCPath${_lib_suffix}
		ENV WebRTCPath
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${WebRTCPath${_lib_suffix}}
		${WebRTCPath}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_WEBRTC_LIBRARY_DIRS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib${_lib_suffix} lib
		libs${_lib_suffix} libs
		bin${_lib_suffix} bin
		../lib${_lib_suffix} ../lib
		../libs${_lib_suffix} ../libs
		../bin${_lib_suffix} ../bin)

include(FindPackageHandleStandardArgs)
# OBS doesnt depend on linking, so we dont include VLC_LIB here as required.
find_package_handle_standard_args(LibWebRTC DEFAULT_MSG WEBRTC_INCLUDE_DIR)
mark_as_advanced(WEBRTC_INCLUDE_DIR WEBRTC_LIB)
message(status ${WEBRTC_LIB})
message(status ${WEBRTC_INCLUDE_DIR})
message(status ${LibWebRTC_FOUND})
if(LibWebRTC_FOUND)
	set(LIBWEBRTC_LIBRARIES ${WEBRTC_LIB})
	set(LIBWEBRTC_INCLUDE_DIRS ${WEBRTC_INCLUDE_DIR})
endif()
