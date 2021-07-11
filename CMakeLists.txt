cmake_minimum_required(VERSION 3.19)

project(apm_set C)

set(CMAKE_C_STANDARD 99)

if (WIN32 AND MINGW)
        # -mconsole: console subsystem
        # -mwindows: window subsystem
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mconsole -static ")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra ")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -ggdb3 -gdwarf-4 -O0")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

set(SOURCE_FILES
    main.c ata_smart.c ata_smart.h)

set(APPRES_OBJS)
if (WIN32 AND MINGW)
        set(WINRES_OUT ${CMAKE_BINARY_DIR}/appres.o)

        add_custom_command(
                OUTPUT ${WINRES_OUT}
                COMMAND windres ${CMAKE_CURRENT_SOURCE_DIR}/app.rc ${WINRES_OUT}
        )

        add_custom_target(WinResObj ALL DEPENDS ${WINRES_OUT})

        set(APPRES_OBJS ${WINRES_OUT})
endif()

add_executable(apm_set ${SOURCE_FILES} ${APPRES_OBJS})

message(STATUS "CMake Config: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: " ${CMAKE_C_COMPILER})
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "External Object: ${APPRES_OBJS}")
