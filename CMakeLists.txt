cmake_minimum_required(VERSION 3.19)

project(BusPlot VERSION 1.0)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CMakeRC)

add_executable(BusPlot)

target_compile_features(BusPlot PRIVATE cxx_std_17)
target_sources(BusPlot
        PRIVATE
        "src/main.cpp"
        "src/element.cpp"
        "src/series.cpp"
        "src/shader.cpp"
        "src/chart.cpp"
        "src/text.cpp"
        "src/axis.cpp"
        )
target_include_directories(BusPlot PRIVATE "src")

cmrc_add_resource_library(resources-shader
        ALIAS
        resources::shader
        NAMESPACE
        resources
        "shaders/vertex.glsl"
        "shaders/fragment.glsl"
        "shaders/text_fragment.glsl"
        "shaders/text_vertex.glsl"
        )
target_link_libraries(BusPlot PRIVATE resources::shader)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE glad::glad)

find_package(glm CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE glm::glm)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE spdlog::spdlog)

find_package(freetype CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE freetype)