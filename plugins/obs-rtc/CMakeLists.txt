project(obs-rtc)

if(MSVC)
        set(rtc_PLATFORM_DEPS
                w32-pthreads)
endif()

set(rtc_HEADERS
          
         crtc.h
         dc-capture.h 
         obfuscate.h
         window-helpers.h
	)

set(rtc_SOURCES
          
        crtc.c
        dc-capture.c 
        obfuscate.c
        window-helpers.c
        plugin-main.c
	 )

add_library(obs-rtc MODULE
	${rtc_SOURCES}
	${rtc_HEADERS}
	)
target_link_libraries(obs-rtc
        ${rtc_PLATFORM_DEPS}
	libobs
	Dwmapi
	ipc-util)
set_target_properties(obs-rtc PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(obs-rtc data)

#add_subdirectory(graphics-hook)
#add_subdirectory(get-graphics-offsets)
#add_subdirectory(inject-helper)
