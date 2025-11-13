include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/src)


add_executable(base32_unit_test base32_ut.c 
    ${CMAKE_SOURCE_DIR}/src/util/base32.c 
)

# Unit test for data structure
add_executable(cms_unit_test data_structure/count_min_sketch_ut.c 
    ${CMAKE_SOURCE_DIR}/src/data_structure/count_min_sketch.c 
)
add_executable(geo_hash_unit_test data_structure/geo_hash_ut.c 
    ${CMAKE_SOURCE_DIR}/src/data_structure/geo_hash.c 
)
add_executable(skip_list_unit_test data_structure/skip_list_ut.c 
    ${CMAKE_SOURCE_DIR}/src/data_structure/skip_list.c 
)
