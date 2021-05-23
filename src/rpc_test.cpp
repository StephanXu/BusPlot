#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <thread>

#include "rpc_protocol.hpp"
#include "serial_rpc.hpp"

namespace asio = boost::asio;

static constexpr char SERVER_PORT[] = "/dev/ptyp1";
static constexpr char CLIENT_PORT[] = "/dev/ttyp1";

static constexpr int BAUD_RATE = 115200;
static const auto STOP_BITS = asio::serial_port::stop_bits::type::one;
static constexpr int CHARACTER_SIZE = 8;
static const auto PARITY = asio::serial_port::parity::type::none;
static const auto FLOW_CONTROL = asio::serial_port::flow_control::none;

auto HandleVariableAlias(const VariableAliasReq &req) -> void {
    spdlog::info("Set variable {} as alias {}.", req.m_VariableId, req.m_Alias);
}

auto Server() -> void {
    SerialRPC serial;
    serial.Connect(SERVER_PORT, BAUD_RATE, STOP_BITS, CHARACTER_SIZE, PARITY, FLOW_CONTROL);
    if (!serial.IsValid()) {
        return;
    }
    serial.RegisterMessage<VariableAliasReq>(HandleVariableAlias);
    serial.StartGrabbing();
    serial.Join();

}

[[noreturn]] auto Client() -> void {
    asio::io_service ios;
    asio::serial_port serialPort(ios, CLIENT_PORT);
    serialPort.set_option(asio::serial_port::baud_rate(BAUD_RATE));
    serialPort.set_option(asio::serial_port::stop_bits(STOP_BITS));
    serialPort.set_option(asio::serial_port::character_size(CHARACTER_SIZE));
    serialPort.set_option(asio::serial_port::parity(PARITY));
    serialPort.set_option(asio::serial_port::flow_control(FLOW_CONTROL));
    if (!serialPort.is_open()) {
        spdlog::critical("Client: Initialize serial port failed.");
        return;
    }
    std::array<RPCRequest<VariableAliasReq>, 1> frameBuffer = {
            SerialRPC::MakeRequest(VariableAliasReq{10, "Var1"})
    };
    while (true) {
        auto wroteLength = asio::write(serialPort, asio::buffer(frameBuffer));
        spdlog::trace("Client: Write {} bytes.", wroteLength);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {
    std::thread tc(Client);
    std::thread ts(Server);
    ts.join();
    tc.join();
}