#ifndef BUSPLOT_GUI_HPP
#define BUSPLOT_GUI_HPP

#include <atomic>

#include "chart.hpp"
#include "serial_rpc.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif // _WIN32

class Gui {
public:
    explicit Gui(SerialRPC *rpc);

    ~Gui();

    auto Initialize() -> void;

    auto Run() -> void;

    auto Chart() noexcept -> Chart &;

    auto CloseWindow() -> void;

private:

    auto Render() -> void;

    static void HelpMarker(const char *desc);

    static void StyleColorsVisualStudio(ImGuiStyle *dst = nullptr);

    static float GetScale();

    void HandleSerialConnect();

    static const char *STOP_BIT_ITEMS[3];
    static const char *PARITY_ITEMS[3];
    static const char *FLOW_CONTROL_ITEMS[3];
    static const char *PID_MODE_ITEMS[1];

    class Chart m_Chart{};

    GLFWwindow *m_Window = nullptr;
    std::string m_DeviceName = "COM1";
    int m_BaudRate = 115200;
    int m_CharacterSize = 8;
    int m_CurtStopBit = 0;
    int m_CurtParity = 0;
    int m_CurtFlowControl = 0;
    int m_CurtPidMode = 0;
    int m_MotorId = 0;
    float m_PidOutMax = 0;
    float m_ControlMatrix[3][3] = {};
    float m_ScaleFactor = 0;
    float m_ChartTimeLimit = 5000.f;
    std::string m_ConnectErrorTips;
    std::atomic<bool> m_Valid = false;
    SerialRPC &m_SerialRPC;
};

#endif // BUSPLOT_GUI_HPP