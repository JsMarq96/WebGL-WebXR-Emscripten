cmake_minimum_required(VERSION 3.13)

project(MIX_WEBXR)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")


file(GLOB EMS_INCLUDES_SUB "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*/*.h")
file(GLOB EMS_INCLUDES "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*.h")

include_directories(MIX_WEBXR ${EMS_INCLUDES} ${EMS_INCLUDES_SUB})

add_executable(MIX_WEBXR ${CPP_SOURCES} ${C_SOURCES})

add_subdirectory(emscripten-webxr)
target_link_libraries(MIX_WEBXR webxr)

add_subdirectory(glm)
target_link_libraries(MIX_WEBXR glm)

message(STATUS "Exporting with WASM")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set_target_properties(MIX_WEBXR PROPERTIES LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s STRICT=1 -lGL -lglfw --shell-file ../web/page.html --preload-file resources@/ --use-preload-plugins")

add_custom_command(TARGET MIX_WEBXR PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:MIX_WEBXR>/resources)
