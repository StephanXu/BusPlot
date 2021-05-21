#ifndef BUSPLOT_SERIAL_HPP
#define BUSPLOT_SERIAL_HPP

#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include <string>

class Serial {
public:

    Serial() : m_IOS(), m_SerialPort(m_IOS) {
    }

    auto Connect(const std::string &deviceName,
                 unsigned int baudRate,
                 boost::asio::serial_port::stop_bits::type stopBits,
                 unsigned int characterSize,
                 boost::asio::serial_port::parity::type parity,
                 boost::asio::serial_port::flow_control::type flowControl) -> bool {
        try {
            m_SerialPort.set_option(boost::asio::serial_port::baud_rate(baudRate));
            m_SerialPort.set_option(boost::asio::serial_port::stop_bits(stopBits));
            m_SerialPort.set_option(boost::asio::serial_port::character_size(characterSize));
            m_SerialPort.set_option(boost::asio::serial_port::parity(parity));
            m_SerialPort.set_option(boost::asio::serial_port::flow_control(flowControl));
            m_SerialPort.open(deviceName);
            m_IsValid = m_SerialPort.is_open();
        } catch (std::exception &err) {
            m_IsValid = false;
        }
        return IsValid();
    }

    auto StartGrabbing() -> void {
        boost::asio::async_read(
                m_SerialPort, m_Buffer,
                [this](const boost::system::error_code &err, size_t readLength) {
                    std::string content(std::istreambuf_iterator<char>(&m_Buffer), {});
                    spdlog::info("{}", content);
                });
    }

    auto IsValid() const noexcept -> bool { return m_IsValid; }

private:
    boost::asio::io_service m_IOS;
    boost::asio::serial_port m_SerialPort;
    bool m_IsValid = false;
    boost::asio::streambuf m_Buffer;
};

#endif // BUSPLOT_SERIAL_HPP