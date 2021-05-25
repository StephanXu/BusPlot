#ifndef BUSPLOT_RPC_PROTOCOL_HPP
#define BUSPLOT_RPC_PROTOCOL_HPP

static constexpr uint8_t SOF = 0xA5;

#pragma pack(push, 1)

struct FrameHeader {
    uint8_t m_DataLength{};
    uint16_t m_Command{};
};

struct FrameTail {
    uint16_t m_CRC16{};
};

struct VariableAliasReq {
    static constexpr uint16_t COMMAND = 0x0010;
    uint16_t m_VariableId{};
    uint8_t m_Alias[10] = {};
};

struct UpdateVariableReq {
    static constexpr uint16_t COMMAND = 0x0020;
    uint16_t m_VariableId{};
    float m_Value{};
};

struct RemoveVariableReq {
    static constexpr uint16_t COMMAND = 0x0030;
    uint16_t m_VariableId{};
};

struct ApplyArgumentReq {
private:
    struct Argument {
        float m_Scale, m_OutMax, m_Value;
    };
public:
    static constexpr uint16_t COMMAND = 0x0040;
    Argument m_P, m_I, m_D;
};

template<class ReqType>
struct RPCRequest {
    uint8_t m_SOF{};
    FrameHeader m_Header;
    ReqType m_Request;
    FrameTail m_Tail;
};

#pragma pack(pop)

#endif // BUSPLOT_RPC_PROTOCOL_HPP