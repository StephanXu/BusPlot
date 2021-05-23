#include <boost/asio.hpp>
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

[[noreturn]] auto Client() -> void {
    asio::io_service ios;
    asio::serial_port serialPort(ios, PORT);
    serialPort.set_option(asio::serial_port::baud_rate(BAUD_RATE));
    serialPort.set_option(asio::serial_port::stop_bits(STOP_BITS));
    serialPort.set_option(asio::serial_port::character_size(CHARACTER_SIZE));
    serialPort.set_option(asio::serial_port::parity(PARITY));
    serialPort.set_option(asio::serial_port::flow_control(FLOW_CONTROL));
    if (!serialPort.is_open()) {
        spdlog::critical("Client: Initialize serial port failed.");
        return;
    }
    spdlog::trace("Client: Serial port connect success.");
    {
        std::array<RPCRequest<VariableAliasReq>, 2> setAliasBuffer = {
                SerialRPC::MakeRequest(VariableAliasReq{1, "Foo"}),
                SerialRPC::MakeRequest(VariableAliasReq{2, "Bar"}),
        };
        asio::write(serialPort, asio::buffer(setAliasBuffer));
    }
    while (true) {
        auto t = std::chrono::time_point_cast<Duration>(Clock::now());
        auto s = static_cast<double>(t.time_since_epoch().count()) / 1000000.f;
        std::array<RPCRequest<UpdateVariableReq>, 2> updateBuffer = {
                SerialRPC::MakeRequest(UpdateVariableReq{1, static_cast<float>(50.f * std::sin(s * 40.f) + 10.f)}),
                SerialRPC::MakeRequest(UpdateVariableReq{2, static_cast<float>(10.f * std::cos(s * 20.f) + 10.f)})
        };
        auto wroteLength = asio::write(serialPort, asio::buffer(updateBuffer));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


int main() {
    spdlog::set_level(spdlog::level::trace);
    Client();
}