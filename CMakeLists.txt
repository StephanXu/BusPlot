cmake_minimum_required(VERSION 3.19)
project(BusPlot VERSION 1.0)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CMakeRC)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

cmrc_add_resource_library(BusPlotResources
                          ALIAS
                          BusPlot::Resources
                          NAMESPACE
                          resources
                          asset/default_layout.ini
                          asset/PingFang.ttf)
set_property(TARGET BusPlotResources PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(BusPlot)
target_compile_features(BusPlot PRIVATE cxx_std_17)
target_sources(BusPlot
               PRIVATE
               src/main.cpp
               src/gui.hpp
               src/gui.cpp
               src/series.hpp
               src/series.cpp
               src/chart.hpp
               src/chart.cpp
               src/serial_rpc.hpp
               src/serial_rpc.cpp
               src/rpc_protocol.hpp
               src/crc.hpp
               src/crc.cpp)
target_sources(BusPlot PRIVATE
               imgui/core/imgui_tables.cpp
               imgui/core/imconfig.h
               imgui/core/imgui_demo.cpp
               imgui/core/imgui_draw.cpp
               imgui/core/imgui_freetype.cpp
               imgui/core/imgui_freetype.h
               imgui/core/imgui_internal.h
               imgui/core/imgui_stdlib.cpp
               imgui/core/imgui_stdlib.h
               imgui/core/imgui_widgets.cpp
               imgui/core/imgui.cpp
               imgui/core/imgui.h
               imgui/core/imstb_rectpack.h
               imgui/core/imstb_textedit.h
               imgui/core/imstb_truetype.h

               imgui/backends/imgui_impl_opengl3.cpp
               imgui/backends/imgui_impl_opengl3.h
               imgui/backends/imgui_impl_glfw.cpp
               imgui/backends/imgui_impl_glfw.h

               imgui/plot/implot.h
               imgui/plot/implot_demo.cpp
               imgui/plot/implot_internal.h
               imgui/plot/implot_items.cpp
               imgui/plot/implot.cpp)
target_include_directories(BusPlot
                           PRIVATE
                           src
                           imgui/backends
                           imgui/core
                           imgui/plot)
target_compile_definitions(BusPlot
                           PRIVATE
                           "IMGUI_ENABLE_FREETYPE")
target_link_libraries(BusPlot PRIVATE
                      BusPlot::Resources
                      glfw
                      glad::glad
                      spdlog::spdlog
                      fmt::fmt
                      freetype
                      Boost::system)
set_property(TARGET BusPlot PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


add_executable(RPCTest)
target_compile_features(RPCTest PRIVATE cxx_std_17)
target_link_libraries(RPCTest
                      PRIVATE
                      Boost::system
                      spdlog::spdlog)
target_sources(RPCTest
               PRIVATE
               test/rpc_test.cpp
               src/serial_rpc.hpp
               src/serial_rpc.cpp
               src/rpc_protocol.hpp
               src/crc.hpp
               src/crc.cpp)
set_property(TARGET RPCTest PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(Simulator)
target_compile_features(Simulator PRIVATE cxx_std_17)
target_link_libraries(Simulator
                      PRIVATE
                      Boost::system
                      spdlog::spdlog)
target_sources(Simulator
               PRIVATE
               test/simulator.cpp
               src/serial_rpc.hpp
               src/serial_rpc.cpp
               src/rpc_protocol.hpp
               src/crc.hpp
               src/crc.cpp)
set_property(TARGET Simulator PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")