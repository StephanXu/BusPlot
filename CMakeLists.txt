cmake_minimum_required(VERSION 3.19)

project(BusPlot VERSION 1.0)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CMakeRC)

add_executable(BusPlot)

target_compile_features(BusPlot PRIVATE cxx_std_17)
target_sources(BusPlot
        PRIVATE
        "src/main.cpp"
        "src/series.cpp"
        "src/chart.cpp"
        )
target_include_directories(BusPlot PRIVATE "src")
set_property(TARGET BusPlot PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

cmrc_add_resource_library(BusPlotResources
        ALIAS
        BusPlot::Resources
        NAMESPACE
        resources
        "asset/default_layout.ini"
        )
set_property(TARGET BusPlotResources PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(BusPlot PRIVATE BusPlot::Resources)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE glad::glad)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE spdlog::spdlog)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE fmt::fmt)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE imgui::imgui)

find_package(implot CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE implot::implot)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(BusPlot PRIVATE Boost::system)