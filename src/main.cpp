#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>

#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>

#include "gl.hpp"
#include "series.hpp"
#include "chart.hpp"
#include "rpc_protocol.hpp"
#include "serial_rpc.hpp"

#ifdef _WIN32
#define NOMINMAX

#include <Windows.h>

#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif // _WIN32

CMRC_DECLARE(resources);

GLFWwindow *window = nullptr;
std::shared_ptr<Chart> chart = nullptr;
bool shouldClose = false;
SerialRPC serialRPC;
std::string deviceName = "COM1";
int baudRate = 115200;
int characterSize = 8;
const char *stopBitItems[] = {u8"1", u8"1.5", u8"2"};
int curtStopBit = 0;
const char *parityItems[] = {u8"无校验", u8"奇校验", u8"偶校验"};
int curtParity = 0;
const char *flowControlItems[] = {u8"无", u8"软件", u8"硬件"};
int curtFlowControl = 0;

const char *pidModeItems[] = {u8"位置式"};
int curtPidMode = 0;
int motorId = 0;
float pidOutMax = 0;
float controlMatrix[3][3] = {};

float chartTimeLimit = 5000.f;
std::string connectErrorTips;

void HelpMarker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void StyleColorsVisualStudio(ImGuiStyle *dst = nullptr) {
    constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b) {
        return ImVec4((float) r / 255.0f, (float) g / 255.0f, (float) b / 255.0f, 1.0f);
    };

    auto &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
    const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
    const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

    const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
    const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
    const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

    const ImVec4 textColor = ColorFromBytes(255, 255, 255);
    const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
    const ImVec4 borderColor = ColorFromBytes(78, 78, 78);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    colors[ImGuiCol_ModalWindowDimBg] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
}

void HandleSerialConnect() {
    serialRPC.Connect(deviceName,
                      baudRate,
                      boost::asio::serial_port::stop_bits::type(curtStopBit),
                      characterSize,
                      boost::asio::serial_port::parity::type(curtParity),
                      boost::asio::serial_port::flow_control::type(curtFlowControl));
    if (!serialRPC.IsValid()) {
        connectErrorTips = u8"连接失败";
        return;
    }
    serialRPC.StartGrabbing([]() -> bool { return shouldClose; });
}

float GetScale() {
    float dDpi = 1;
#if (defined(_WIN32))
    HDC desktopDc = GetDC(nullptr);
    float horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
    float verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
    int dpi = (horizontalDPI + verticalDPI) / 2;
    dDpi = 1 + ((dpi - 96) / 24) * 0.25;
    if (dDpi < 1) {
        dDpi = 1;
    }
    ::ReleaseDC(nullptr, desktopDc);
#elif (defined(__APPLE__))
    dDpi = 2.f;
#else
    dDpi = 1.f;
#endif
    return dDpi;
}

auto HandleVariableAliasRequest(const VariableAliasReq &req) -> void {
    auto series = chart->GetOrAddSeries(req.m_VariableId);
    series->SetLabel(std::string(reinterpret_cast<const char *>(req.m_Alias)));
}

auto HandleUpdateVariableRequest(const UpdateVariableReq &req) -> void {
    auto series = chart->GetOrAddSeries(req.m_VariableId);
    auto t = std::chrono::time_point_cast<Duration>(Clock::now());
    auto s = static_cast<double>(t.time_since_epoch().count()) / 1000000.f;
    series->AddData(Dot{s, req.m_Value});
}

auto HandleRemoveVariableRequest(const RemoveVariableReq &req) -> void {
    chart->RemoveSeries(req.m_VariableId);
}


int main() {
    if (!glfwInit())
        exit(EXIT_FAILURE);

    serialRPC.RegisterMessage<VariableAliasReq>(HandleVariableAliasRequest);
    serialRPC.RegisterMessage<UpdateVariableReq>(HandleUpdateVariableRequest);
    serialRPC.RegisterMessage<RemoveVariableReq>(HandleRemoveVariableRequest);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(1440, 900, "BusPlot", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;
    StyleColorsVisualStudio();
    const auto scaleFactor = GetScale();
    ImGui::GetStyle().ScaleAllSizes(scaleFactor);

    auto fs = cmrc::resources::get_filesystem();
    auto defaultLayoutContent = fs.open("asset/default_layout.ini");
    std::vector<char> defaultLayoutBuffer(defaultLayoutContent.begin(), defaultLayoutContent.end());
    ImGui::LoadIniSettingsFromMemory(defaultLayoutBuffer.data(), defaultLayoutBuffer.size());
    io.Fonts->AddFontFromFileTTF("asset/PingFang.ttc",
                                 18.f * scaleFactor,
                                 nullptr,
                                 io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    chart = std::make_shared<Chart>();

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", nullptr,
                         ImGuiWindowFlags_NoDocking
                         | ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoCollapse
                         | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoBringToFrontOnFocus
                         | ImGuiWindowFlags_NoNavFocus);
            ImGui::PopStyleVar(3);
            ImGuiID dockerSpaceId = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockerSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
            ImGui::End();
        }

//        ImGui::ShowDemoWindow(nullptr);
//        ImPlot::ShowDemoWindow(nullptr);

        if (ImGui::Begin(u8"连接设置", nullptr)) {
            ImGui::InputText(u8"设备名", &deviceName);
            ImGui::InputInt(u8"波特率", &baudRate, 0);
            ImGui::Combo(u8"停止位", &curtStopBit, stopBitItems, sizeof(stopBitItems) / sizeof(const char *));
            ImGui::InputInt(u8"数据位", &characterSize, 0);
            ImGui::Combo(u8"奇偶校验", &curtParity, parityItems, sizeof(parityItems) / sizeof(const char *));
            ImGui::Combo(u8"流控制", &curtFlowControl, flowControlItems, sizeof(flowControlItems) / sizeof(const char *));

            ImGui::PushDisabled(serialRPC.IsValid());
            if (ImGui::Button(u8"连接", ImVec2(-1, 0))) {
                HandleSerialConnect();
            }
            ImGui::PopDisabled();

            if (connectErrorTips.length() > 0) {
                ImGui::Text("%s", connectErrorTips.c_str());
            }
            ImGui::End();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin(u8"图表", nullptr)) {
            chart->RenderPlot();
            ImGui::End();
        }
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin(u8"所有信号", nullptr)) {
            chart->RenderTable(scaleFactor);
            ImGui::End();
        }
        ImGui::PopStyleVar();

        if (ImGui::Begin(u8"参数设置", nullptr)) {
            ImGui::Combo(u8"模式", &curtPidMode, pidModeItems, sizeof(pidModeItems) / sizeof(const char *));
            ImGui::InputInt(u8"设备ID", &motorId);
            ImGui::DragFloat3(u8"Kp", controlMatrix[0]);
            ImGui::SameLine();
            HelpMarker(u8"设置P参数\n"
                       u8"拖动或双击修改参数值\n"
                       u8"值顺序为: (Scale, OutMax, Value)\n");
            ImGui::DragFloat3(u8"Ki", controlMatrix[1]);
            ImGui::SameLine();
            HelpMarker(u8"设置I参数\n"
                       u8"拖动或双击修改参数值\n"
                       u8"值顺序为: (Scale, OutMax, Value)\n");
            ImGui::DragFloat3(u8"Kd", controlMatrix[2]);
            ImGui::SameLine();
            HelpMarker(u8"设置D参数\n"
                       u8"拖动或双击修改参数值\n"
                       u8"值顺序为: (Scale, OutMax, Value)\n");
            ImGui::Button(u8"应用参数", ImVec2(-1, 0));
            if (ImGui::DragFloat(u8"图表时长", &chartTimeLimit, 10.f, 1000.f, 0.f, "%.3f ms")) {
                chart->SetTimeLimit(std::chrono::microseconds(static_cast<long long>(chartTimeLimit * 1000.f)));
            }
            ImGui::End();
        }

        ImGui::Render();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    shouldClose = true;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}