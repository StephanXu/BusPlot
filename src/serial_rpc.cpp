
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>

#include <string>
#include <memory>

#include "rpc_protocol.hpp"
#include "serial_rpc.hpp"

namespace asio = boost::asio;

SerialRPC::SerialRPC() : m_IOS(), m_SerialPort(m_IOS) {
}

SerialRPC::~SerialRPC() {
    StopGrabbing();
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

auto SerialRPC::StartGrabbing() -> void {
    ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
    m_WorkingThread = std::make_shared<std::thread>([this]() {
        m_IOS.run();
    });
}

auto SerialRPC::Join() -> void {
    if (m_WorkingThread) {
        m_WorkingThread->join();
    }
}

auto SerialRPC::IsValid() const noexcept -> bool { return m_IsValid; }

auto SerialRPC::Close() -> void {
    m_SerialPort.close();
}

auto SerialRPC::StopGrabbing() -> void {
    m_IOS.stop();
}

auto SerialRPC::ReadHeaderHandler(const boost::system::error_code &err, size_t len) -> void {
    if (err) {
        spdlog::warn("SerialPort read header failed: {}", err.message());
        ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
        return;
    }
    const auto &header = m_HeaderBuffer[0];
    spdlog::trace("SerialPort: Receive header: Length: {}, Command: {}", header.m_DataLength,
                  header.m_Command);
    auto handleIt = m_Callbacks.find(header.m_Command);
    if (handleIt == m_Callbacks.end()) {
        spdlog::warn("SerialPort: Ignore command {}", header.m_Command);
        ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
        return;
    }
    handleIt->second(header.m_DataLength);
}

auto SerialRPC::ReadSOFHandler(const boost::system::error_code &err, size_t len) -> void {
    if (err) {
        spdlog::critical("SerialPort read SOF failed: {}", err.message());
        return;
    }
    if (m_SOFBuffer[0] == SOF) {
        ReadAsync(m_HeaderBuffer, &SerialRPC::ReadHeaderHandler);
    } else {
        ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
    }
}
