#ifndef BUSPLOT_SERIAL_RPC_HPP
#define BUSPLOT_SERIAL_RPC_HPP

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include <string>
#include <iterator>
#include <memory>
#include <unordered_map>

#include "rpc_protocol.hpp"

class SerialRPC {
    template<class ReqType>
    using MessageCallBack = std::function<void(const ReqType &)>;

public:

    SerialRPC() : m_IOS(), m_SerialPort(m_IOS) {
    }

    ~SerialRPC() {
        Join();
    }

    auto Connect(const std::string &deviceName,
                 unsigned int baudRate,
                 boost::asio::serial_port::stop_bits::type stopBits,
                 unsigned int characterSize,
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

    auto StartGrabbing() -> void {
        m_WorkingThread = std::make_shared<std::thread>([this]() {
            while (true) {
                ReadProcess();
            }
        });
    }

    auto Join() -> void {
        if (m_WorkingThread) {
            m_WorkingThread->join();
        }
    }

    auto IsValid() const noexcept -> bool { return m_IsValid; }

    template<class ReqType>
    auto RegisterMessage(MessageCallBack<ReqType> process) -> bool {
        const auto handleRequest = [this, process](size_t dataLength) -> void {
            if (dataLength != sizeof(ReqType)) {
                spdlog::warn("SerialPort: Package length {} can't match request type {}, whose size is {}.",
                             dataLength,
                             typeid(ReqType).name(),
                             sizeof(ReqType));
                return;
            }
            std::array<ReqType, 1> reqBuffer;
            std::array<FrameTail, 1> tailBuffer;
            boost::asio::read(m_SerialPort, boost::asio::buffer(reqBuffer),
                              boost::asio::transfer_exactly(sizeof(ReqType)));
            boost::asio::read(m_SerialPort, boost::asio::buffer(tailBuffer),
                              boost::asio::transfer_exactly(sizeof(FrameTail)));
            const auto &req = reqBuffer.front();
            process(req);
        };

        auto[_, suc] = m_Callbacks.insert(std::make_pair(ReqType::COMMAND, handleRequest));
        return suc;
    }

    template<class ReqType>
    static auto MakeRequest(const ReqType &requestBody) -> RPCRequest<ReqType> {
        return RPCRequest<ReqType>
                {
                        SOF,
                        FrameHeader{sizeof(ReqType), ReqType::COMMAND},
                        requestBody,
                        {0}
                };
    }

private:

    auto ReadProcess() -> void {
//        auto readLength = boost::asio::read_until(m_SerialPort, m_Buffer, SOF);
//        m_Buffer.consume(readLength);
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
        spdlog::info("SerialPort: Receive header: Length: {}, Command: {}", header.m_DataLength, header.m_Command);
        auto handleIt = m_Callbacks.find(header.m_Command);
        if (handleIt == m_Callbacks.end()) {
            spdlog::warn("SerialPort: Ignore command {}", header.m_Command);
            return;
        }
        handleIt->second(header.m_DataLength);
    }

    std::shared_ptr<std::thread> m_WorkingThread;
    boost::asio::io_service m_IOS;
    boost::asio::serial_port m_SerialPort;
    boost::asio::streambuf m_Buffer;
    std::unordered_map<uint16_t, std::function<void(size_t)>> m_Callbacks;
    bool m_IsValid = false;
};

#endif // BUSPLOT_SERIAL_RPC_HPP