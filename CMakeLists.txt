cmake_minimum_required(VERSION 3.9)
project(NGP C)

set(EXEC lmeca2710_project) # put whatever name you want for the executable

file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${EXEC}
               "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
               "${CMAKE_CURRENT_SOURCE_DIR}/src/inputs.c"
               "${CMAKE_CURRENT_SOURCE_DIR}/src/delaunay.c"
               "${CMAKE_CURRENT_SOURCE_DIR}/src/predicates.c"
               # you can add other source file here !
               )

target_include_directories(${EXEC} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(${EXEC} PROPERTIES
                      C_STANDARD 99
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_subdirectory(deps/BOV)
target_link_libraries(${EXEC} bov)

# set ${EXEC} as the startup project in visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC})

# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
    SET(CMAKE_CXX_FLAGS "-O3")
    SET(CMAKE_C_FLAGS "-O3")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
