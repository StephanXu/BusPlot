#ifndef BUSPLOT_SERIAL_RPC_HPP
#define BUSPLOT_SERIAL_RPC_HPP

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include <string>
#include <memory>
#include <unordered_map>

#include "rpc_protocol.hpp"

class SerialRPC {
    template<class ReqType>
    using MessageCallBack = std::function<void(const ReqType &)>;
    using CloseCondition = std::function<bool()>;
public:

    SerialRPC();

    ~SerialRPC();

    auto Connect(const std::string &deviceName,
                 unsigned int baudRate,
                 boost::asio::serial_port::stop_bits::type stopBits,
                 unsigned int characterSize,
                 boost::asio::serial_port::parity::type parity,
                 boost::asio::serial_port::flow_control::type flowControl) -> bool;

    auto StartGrabbing(const CloseCondition &condition) -> void;

    auto Join() -> void;

    auto IsValid() const noexcept -> bool;

    auto Close() -> void {
        m_SerialPort.close();
    }

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

    auto ReadProcess() -> void;

    std::shared_ptr<std::thread> m_WorkingThread;
    boost::asio::io_service m_IOS;
    boost::asio::serial_port m_SerialPort;
    boost::asio::streambuf m_Buffer;
    std::unordered_map<uint16_t, std::function<void(size_t)>> m_Callbacks;
    bool m_IsValid = false;
    CloseCondition m_CloseCondition = []() { return false; };
};

#endif // BUSPLOT_SERIAL_RPC_HPP