cmake_minimum_required(VERSION 3.13)

project(MIX_WEBXR)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")

add_executable(MIX_WEBXR ${CPP_SOURCES} ${C_SOURCES} ${CPP_SUBFOLDER_SOURCES} ${C_SUBFOLDER_SOURCES})

if(EMSCRIPTEN)
# WEBXR
add_subdirectory(emscripten-webxr)
target_link_libraries(MIX_WEBXR webxr)
#EMSCRIPTEN
file(GLOB EMS_INCLUDES_SUB "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*/*.h")
file(GLOB EMS_INCLUDES "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*.h")

include_directories(MIX_WEBXR ${EMS_INCLUDES} ${EMS_INCLUDES_SUB})
else()
set(includes_dir "includes/")
# GLTW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(MIX_WEBXR glfw)

# GLEW
set(gl3w_dir "${includes_dir}/GL")
add_library("gl3w" "${gl3w_dir}/src/gl3w.c")
target_include_directories("gl3w" PRIVATE "${glw_dir}/")
target_include_directories(MIX_WEBXR PRIVATE "${gl3w_dir}/")
target_link_libraries(MIX_WEBXR "gl3w" "${CMAKE_DL_LIBS}")

# Visual Studio compat.
if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
    set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT phys_sandbox)
endif()

endif()

add_subdirectory(glm)

if(NOT EMSCRIPTEN)
# Web Export
target_link_libraries(MIX_WEBXR glm)

message(STATUS "Exporting with WASM")
set(CMAKE_EXECUTABLE_SUFFIX ".html")
set_target_properties(MIX_WEBXR PROPERTIES LINK_FLAGS "-s EXPORTED_RUNTIME_METHODS=WebXR -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s STRICT=1  -lGL --shell-file ../web/page.html --use-preload-plugins")

set_target_properties(MIX_WEBXR PROPERTIES OUTPUT_NAME "index")
else()
# Native export
include_directories(
  includes
  gl3w_dir
 )

set_target_properties(MIX_WEBXR PROPERTIES OUTPUT_NAME "visualizator")
message(STATUS "Exporting to native")
target_link_libraries(MIX_WEBXR glm glfw)
endif()

# Copy the resources folder to the output folder
add_custom_command(TARGET MIX_WEBXR PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:MIX_WEBXR>/resources)
