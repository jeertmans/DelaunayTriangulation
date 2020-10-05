cmake_minimum_required(VERSION 3.6)
project(BOV C)

option(BOV_BUILD_EXAMPLES "Build the BOV example programs" ON)

add_library(bov src/BOV.c)

set_target_properties(bov PROPERTIES
                      C_STANDARD 99
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
target_include_directories(bov PUBLIC include)
target_include_directories(bov PRIVATE src)
target_include_directories(bov PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/shaderIncludes)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)

set(shadersGLSL
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/points_vert.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/points_geom.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/points_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/lines_geom.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/lines_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve_geom.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangles_geom.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangles_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/text_vert.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/text_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/default_vert.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/default_frag.glsl"
	)
set(shadersBasename
    "points_vert"
    "points_geom"
    "points_frag"
    "lines_geom"
    "lines_frag"
    "curve_geom"
    "triangles_geom"
    "triangles_frag"
    "text_vert"
    "text_frag"
    "default_vert"
    "default_frag"
    )

# transform shaders in a valid C string contained in a C header file
# file(GLOB_RECURSE shadersGLSL ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl)
foreach(iterator RANGE 11)
	list(GET shadersGLSL ${iterator} fileGLSL)
	list(GET shadersBasename ${iterator} fileBasename)
	set(fileH "${CMAKE_CURRENT_BINARY_DIR}/shaderIncludes/${fileBasename}.h")
	list(APPEND shadersH ${fileH})
	file(RELATIVE_PATH printFileH ${CMAKE_SOURCE_DIR} ${fileH})
	file(RELATIVE_PATH printFileGLSL ${CMAKE_SOURCE_DIR} ${fileGLSL})

	add_custom_command(OUTPUT ${fileH}
	                   COMMAND cmake -DIN=${fileGLSL} -DVAR=${fileBasename} -DOUT=${fileH} -P ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaderToString.cmake
	                   DEPENDS ${fileGLSL}
	                   COMMENT "generating ${printFileH} from shader ${printFileGLSL}")
endforeach(iterator)
add_custom_target(shaderIncludes
                  DEPENDS ${shadersH}
                  SOURCES ${shadersH})
add_dependencies(bov shaderIncludes)

# add dependencies: glfw, glad, -lm
target_link_libraries(bov
                      PUBLIC glad glfw)



if(BOV_BUILD_EXAMPLES)
	add_executable(animation ${CMAKE_CURRENT_SOURCE_DIR}/examples/animation.c)
	add_executable(character_table ${CMAKE_CURRENT_SOURCE_DIR}/examples/character_table.c)
	add_executable(hello_world ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello_world.c)
	add_executable(koch_snowflake ${CMAKE_CURRENT_SOURCE_DIR}/examples/koch_snowflake.c)
	add_executable(markers ${CMAKE_CURRENT_SOURCE_DIR}/examples/markers.c)
	add_executable(lines ${CMAKE_CURRENT_SOURCE_DIR}/examples/lines.c)
	add_executable(text_style ${CMAKE_CURRENT_SOURCE_DIR}/examples/text_style.c)
	add_executable(triangles ${CMAKE_CURRENT_SOURCE_DIR}/examples/triangles.c)

	target_link_libraries(animation bov)
	target_link_libraries(character_table bov)
	target_link_libraries(hello_world bov)
	target_link_libraries(koch_snowflake bov)
	target_link_libraries(markers bov)
	target_link_libraries(lines bov)
	target_link_libraries(text_style bov)
	target_link_libraries(triangles bov)

	set_target_properties(animation
	                      character_table
	                      koch_snowflake
	                      markers
	                      lines
	                      text_style
	                      hello_world
	                      triangles
	                      PROPERTIES
	                      C_STANDARD 99
	                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/examples")

endif(BOV_BUILD_EXAMPLES)