#ifndef BUSPLOT_SERIAL_RPC_HPP
#define BUSPLOT_SERIAL_RPC_HPP

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/serial_port.hpp>

#include <string>
#include <memory>
#include <unordered_map>

#include "rpc_protocol.hpp"
#include "crc.hpp"

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

    auto StartGrabbing() -> void;

    auto Join() -> void;

    [[nodiscard]] auto IsValid() const noexcept -> bool;

    auto Close() -> void;

    auto StopGrabbing() -> void;

    template<class ReqType>
    auto RegisterMessage(MessageCallBack<ReqType> process) -> bool {
        const auto handleRequest = [this, process](size_t dataLength) -> void {
            if (dataLength != sizeof(ReqType)) {
                spdlog::warn("SerialPort: Package length {} can't match request type {}, whose size is {}.",
                             dataLength,
                             typeid(ReqType).name(),
                             sizeof(ReqType));
                ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
                return;
            }
            boost::asio::async_read(
                    m_SerialPort, boost::asio::buffer(m_BodyBuffer),
                    boost::asio::transfer_exactly(sizeof(BodyWithTail<ReqType>)),
                    boost::bind(&SerialRPC::ReadBodyHandler<ReqType>,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred,
                                process));
        };
        auto[_, suc] = m_Callbacks.insert(std::make_pair(ReqType::COMMAND, handleRequest));
        return suc;
    }

    template<class ReqType>
    static auto MakeRequest(const ReqType &requestBody) -> RPCRequest<ReqType> {
        auto req = RPCRequest<ReqType>
                {
                        SOF,
                        FrameHeader{sizeof(ReqType), ReqType::COMMAND},
                        requestBody,
                        {0}
                };
        CRC::AppendCRC16Checksum(reinterpret_cast<uint8_t *>(&req), sizeof(req));
        return req;
    }

private:

    auto ReadSOFHandler(const boost::system::error_code &err, size_t len) -> void;

    auto ReadHeaderHandler(const boost::system::error_code &err, size_t len) -> void;

    template<class ReqType>
    auto ReadBodyHandler(const boost::system::error_code &err, size_t len, MessageCallBack<ReqType> process) -> void {
        if (err) {
            spdlog::warn("SerialPort read body failed: {}", err.message());
            ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
            return;
        }
        RPCRequest<ReqType> buf{SOF, m_HeaderBuffer[0], {}, {}};
        std::copy(m_BodyBuffer.begin(),
                  m_BodyBuffer.begin() + sizeof(RPCRequest<ReqType>) - sizeof(FrameHeader) - sizeof(uint8_t),
                  reinterpret_cast<uint8_t *>(&buf.m_Request));
        if (!CRC::VerifyCRC16Checksum(&buf, sizeof(buf))) {
            spdlog::warn("SerialPort CRC16 verify failed");
            ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
            return;
        };
        process(buf.m_Request);
        ReadAsync(m_SOFBuffer, &SerialRPC::ReadSOFHandler);
    }

    template<class BufferType, size_t N, class HandlerType>
    auto ReadAsync(std::array<BufferType, N> &buffer, HandlerType handler) -> void {
        boost::asio::async_read(m_SerialPort, boost::asio::buffer(buffer),
                                boost::asio::transfer_exactly(sizeof(BufferType) * N),
                                boost::bind(handler, this, boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    template<class BufferType>
    auto ReadSerialPort(BufferType &destBuffer) -> bool {
        std::array<BufferType, 1> buffer;
        std::promise<bool> promise;
        boost::asio::async_read(
                m_SerialPort, boost::asio::buffer(buffer),
                boost::asio::transfer_exactly(sizeof(BufferType)),
                [&](const boost::system::error_code &err, size_t length) {
                    if (err) {
                        spdlog::critical("SerialPort Read failed: {}", err.message());
                        promise.set_value(false);
                        return;
                    }
                    promise.set_value(true);
                });
        destBuffer = buffer[0];
        return promise.get_future().get();
    }

    std::shared_ptr<std::thread> m_WorkingThread;
    boost::asio::io_service m_IOS;
    boost::asio::serial_port m_SerialPort;
    boost::asio::streambuf m_Buffer;
    std::unordered_map<uint16_t, std::function<void(size_t)>> m_Callbacks;
    bool m_IsValid = false;

    std::array<uint8_t, 1> m_SOFBuffer{};
    std::array<FrameHeader, 1> m_HeaderBuffer{};
    std::array<uint8_t, 512> m_BodyBuffer{};
};

#endif // BUSPLOT_SERIAL_RPC_HPP