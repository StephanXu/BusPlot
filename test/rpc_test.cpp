#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <thread>
#include <iostream>

#include "../src/rpc_protocol.hpp"
#include "../src/serial_rpc.hpp"

namespace asio = boost::asio;

static constexpr char SERVER_PORT[] = "COM2";
static constexpr char CLIENT_PORT[] = "COM3";

static constexpr int BAUD_RATE = 115200;
static const auto STOP_BITS = asio::serial_port::stop_bits::type::one;
static constexpr int CHARACTER_SIZE = 8;
static const auto PARITY = asio::serial_port::parity::type::none;
static const auto FLOW_CONTROL = asio::serial_port::flow_control::none;

struct FooReq {
    static constexpr uint16_t COMMAND = 0x0021;
    uint16_t bar{};
};

auto HandleFooRequest(const FooReq &req) -> void {
    std::cout << "Value: " << req.bar << std::endl;
}

auto Server() -> void {
    SerialRPC serial;
    serial.Connect(SERVER_PORT, BAUD_RATE, STOP_BITS, CHARACTER_SIZE, PARITY, FLOW_CONTROL);
    if (!serial.IsValid()) {
        return;
    }
    spdlog::trace("Server: Serial port connect success.");
    serial.RegisterMessage<FooReq>(HandleFooRequest);
    serial.StartGrabbing();
}

auto Client() -> void {
    SerialRPC serial;
    serial.Connect(SERVER_PORT, BAUD_RATE, STOP_BITS, CHARACTER_SIZE, PARITY, FLOW_CONTROL);
    if (!serial.IsValid()) {
        return;
    }
    spdlog::trace("Client: Serial port connect success.");
    uint16_t count = 0;
    while (true) {
        serial.Request(FooReq{++count});
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {
    spdlog::set_level(spdlog::level::trace);
    std::thread tc(Client);
    std::thread ts(Server);
    ts.join();
    tc.join();
}