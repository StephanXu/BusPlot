
#include <spdlog/spdlog.h>

#include <chrono>

#include "serial_rpc.hpp"
#include "gui.hpp"

#ifdef _WIN32
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif // _WIN32

static SerialRPC serialRPC;
static Gui gui(&serialRPC);

auto HandleVariableAliasRequest(const VariableAliasReq &req) -> void {
    auto series = gui.Chart().GetOrAddSeries(req.m_VariableId);
    series->SetLabel(std::string(reinterpret_cast<const char *>(req.m_Alias)));
}

auto HandleUpdateVariableRequest(const UpdateVariableReq &req) -> void {
    auto series = gui.Chart().GetOrAddSeries(req.m_VariableId);
    auto t = std::chrono::time_point_cast<Duration>(Clock::now());
    auto s = static_cast<double>(t.time_since_epoch().count()) / 1000000.f;
    series->AddData(Dot{s, req.m_Value});
}

auto HandleRemoveVariableRequest(const RemoveVariableReq &req) -> void {
    gui.Chart().RemoveSeries(req.m_VariableId);
}


int main() {
    spdlog::set_level(spdlog::level::info);

    serialRPC.RegisterMessage<VariableAliasReq>(HandleVariableAliasRequest);
    serialRPC.RegisterMessage<UpdateVariableReq>(HandleUpdateVariableRequest);
    serialRPC.RegisterMessage<RemoveVariableReq>(HandleRemoveVariableRequest);

    gui.Run();
}