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
target_include_directories(BusPlot
        PRIVATE
        "src"
        "imgui/backends"
        "imgui/core"
        "imgui/plot")
target_compile_definitions(BusPlot PRIVATE "IMGUI_ENABLE_FREETYPE")
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

#find_package(imgui CONFIG REQUIRED)
#target_link_libraries(BusPlot PRIVATE imgui::imgui)
find_package(freetype CONFIG REQUIRED)
target_link_libraries(BusPlot PRIVATE freetype)

#find_package(implot CONFIG REQUIRED)
#target_link_libraries(BusPlot PRIVATE implot::implot)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(BusPlot PRIVATE Boost::system)

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
        imgui/plot/implot.cpp
        )