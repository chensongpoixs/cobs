project(rtc)

if(MSVC)
        set(rtc_PLATFORM_DEPS
                w32-pthreads)
endif()

set(rtc_HEADERS
         app-helpers.h
         crtc.h
         dc-capture.h
         nt-stuff.h
         obfuscate.h
         window-helpers.h
	)

set(rtc_SOURCES
         app-helpers.c
        crtc.c
        dc-capture.c
        nt-stuff.c
        obfuscate.c
        window-helpers.c
        plugin-main.c
	 )

add_library(rtc MODULE
	${rtc_SOURCES}
	${rtc_HEADERS}
	)
target_link_libraries(rtc
        ${rtc_PLATFORM_DEPS}
	libobs
	Dwmapi
	ipc-util)
set_target_properties(rtc PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(rtc data)

#add_subdirectory(graphics-hook)
#add_subdirectory(get-graphics-offsets)
#add_subdirectory(inject-helper)
