
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include <string>
#include <future>
#include <memory>
#include <unordered_map>

#include "rpc_protocol.hpp"
#include "serial_rpc.hpp"

SerialRPC::SerialRPC() : m_IOS(), m_SerialPort(m_IOS) {
}

SerialRPC::~SerialRPC() {
    Close();
    Join();
}

auto SerialRPC::Connect(const std::string &deviceName, unsigned int baudRate,
                        boost::asio::serial_port::stop_bits::type stopBits, unsigned int characterSize,
                        boost::asio::serial_port::parity::type parity,
                        boost::asio::serial_port::flow_control::type flowControl) -> bool {
    try {
        m_SerialPort.open(deviceName);
        m_SerialPort.set_option(boost::asio::serial_port::baud_rate(baudRate));
        m_SerialPort.set_option(boost::asio::serial_port::stop_bits(stopBits));
        m_SerialPort.set_option(boost::asio::serial_port::character_size(characterSize));
        m_SerialPort.set_option(boost::asio::serial_port::parity(parity));
        m_SerialPort.set_option(boost::asio::serial_port::flow_control(flowControl));
        m_IsValid = m_SerialPort.is_open();
    } catch (std::exception &err) {
        m_IsValid = false;
    }
    return IsValid();
}

auto SerialRPC::StartGrabbing(const CloseCondition &condition) -> void {
    m_CloseCondition = condition;
    m_WorkingThread = std::make_shared<std::thread>([this]() {
        while (!m_CloseCondition()) {
            ReadProcess();
        }
    });
}

auto SerialRPC::Join() -> void {
    if (m_WorkingThread) {
        m_WorkingThread->join();
    }
}

auto SerialRPC::IsValid() const noexcept -> bool { return m_IsValid; }

auto SerialRPC::ReadProcess() -> void {
    std::array<uint8_t, 1> sofBuffer{};
    boost::asio::read(m_SerialPort, boost::asio::buffer(sofBuffer), boost::asio::transfer_exactly(sizeof(uint8_t)));
    if (sofBuffer[0] != SOF) {
        spdlog::trace("SerialPort: SOF not match, skip.");
        return;
    }
    std::array<FrameHeader, 1> headerBuffer{};
    boost::asio::read(m_SerialPort,
                      boost::asio::buffer(headerBuffer),
                      boost::asio::transfer_exactly(sizeof(FrameHeader)));
    const auto &header = headerBuffer[0];
    spdlog::trace("SerialPort: Receive header: Length: {}, Command: {}", header.m_DataLength, header.m_Command);
    auto handleIt = m_Callbacks.find(header.m_Command);
    if (handleIt == m_Callbacks.end()) {
        spdlog::warn("SerialPort: Ignore command {}", header.m_Command);
        return;
    }
    handleIt->second(header.m_DataLength);
}
