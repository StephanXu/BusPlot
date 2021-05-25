#include <spdlog/spdlog.h>

#include <thread>

#include "../src/rpc_protocol.hpp"
#include "../src/serial_rpc.hpp"

namespace asio = boost::asio;

using Clock = std::chrono::system_clock;
using TimeType = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Duration = std::chrono::microseconds;

static constexpr char PORT[] = "COM2";

static constexpr int BAUD_RATE = 115200;
static const auto STOP_BITS = asio::serial_port::stop_bits::type::one;
static constexpr int CHARACTER_SIZE = 8;
static const auto PARITY = asio::serial_port::parity::type::none;
static const auto FLOW_CONTROL = asio::serial_port::flow_control::none;

auto Client() -> void {
    SerialRPC rpc;
    rpc.Connect(PORT, BAUD_RATE, STOP_BITS, CHARACTER_SIZE, PARITY, FLOW_CONTROL);
    if (!rpc.IsValid()) {
        return;
    }
    spdlog::trace("Client: Serial port connect success.");
    rpc.Request(VariableAliasReq{1, "Foo"});
    rpc.Request(VariableAliasReq{2, "Bar"});
    while (true) {
        auto t = std::chrono::time_point_cast<Duration>(Clock::now());
        auto s = static_cast<double>(t.time_since_epoch().count()) / 1000000.f;
        rpc.Request(UpdateVariableReq{1, static_cast<float>(50.f * std::sin(s * 40.f) + 10.f)});
        rpc.Request(UpdateVariableReq{2, static_cast<float>(10.f * std::cos(s * 20.f) + 10.f)});
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


int main() {
    spdlog::set_level(spdlog::level::trace);
    Client();
}