cmake_minimum_required(VERSION 3.5)
project(redis-C)

# Force Debug build type
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(REDIS_BUILD OFF)
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

# Add all source files to the library
if(REDIS_BUILD)
    if(REDIS_SHARED_LIBRARY)
        add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})
    else()
        add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
    endif()
endif()

if(REDIS_ENABLE_TEST)
    add_subdirectory(test)
endif()



