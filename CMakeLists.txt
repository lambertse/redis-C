cmake_minimum_required(VERSION 3.5)
project(redis-C)

# Force Debug build type
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(REDIS_BUILD ON)
set(REDIS_ENABLE_TEST ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

# Define compile flags for Debug and Release modes
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# Include all header files in the include directory
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.h")

# Include all source files in the src directory
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(IO_MUX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/io-multiplexing/include)
add_subdirectory(3rdparty/io-multiplexing)

set(CLIENT_SOURCE   src/cli.c
                    src/config.c
)

set(SERVER_SOURCE   src/server.c 
                    src/cmd_handler.c
                    src/config.c
                    src/storage.c
                    src/data_structure/count_min_sketch.c
)

# Add all source files to the library
if(REDIS_BUILD)
    # Build Server
    add_executable(redis-c-server ${SERVER_SOURCE})
    target_include_directories(redis-c-server PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IO_MUX_INCLUDE_DIR}
    )
    target_link_libraries(redis-c-server PRIVATE io-multiplexing)

    # Build CLI
    add_executable(redis-c-cli ${CLIENT_SOURCE})
    target_include_directories(redis-c-cli PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IO_MUX_INCLUDE_DIR}
    )
    target_link_libraries(redis-c-cli PRIVATE io-multiplexing)
endif()

if(REDIS_ENABLE_TEST)
    add_subdirectory(test)
endif()



