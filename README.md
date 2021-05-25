

# Bus Plot

Cross-platform plotting for embedded devices status.

## Build

### 1. Install dependencies through Vcpkg

Generally, you could install dependencies through vcpkg easily. The example below has specified the platform (`x64-windows-static`), you can also choose other options to build for other platforms.

```bash
./vcpkg.exe install \
    --triplet x64-windows-static
    boost-asio \
    fmt \
    spdlog \
    glfw3 \
    glad \
    freetype
```

### 2. Build

You need to set `CMAKE_TOOLCHAIN_FILE` variable to `/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`.

Build for debug: 

```bash
mkdir -p build/debug
cd build/debug
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
cd ../..
```

Build for release:

```bash
mkdir -p build/release
cd build/release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ../..
```

## Serial RPC protocol

Bus Plot implemented a Remote Procedure Call (RPC) protocol for communication between host device and slave device. You can extend it to implement your own functions.

### Basic Usage

#### Define protocol

First you need to declare your own request structure like:

```c++
struct FooReq {
    static constexpr uint16_t COMMAND = 0x0021;
    uint16_t bar{};
};
```

It simply contains a member `bar`.

#### Server side

On the server side, you need to implement a handle function to response request:

```c++
auto HandleFooRequest(const FooReq &req) -> void {
    std::cout << "Value: " << req.bar << std::endl;
}
```

Finnally just register the request structure and handle function before start grabbing:

```c++
serialRPC.RegisterMessage<FooReq>(HandleFooRequest);
```

#### Client side

You can simply construct request body and call `Request` to make a call.

```c++
FooReq req = {16};
m_SerialRPC.Request(req);
```

#### Establish Connect

Both of server and client are required to establish a connection before start grabbing:

```c++
serialRPC.Connect("COM3",
                  115200,
                  boost::asio::serial_port::stop_bits::type::one,
                  8,
                  boost::asio::serial_port::parity::type::none,
                  boost::asio::serial_port::flow_control::type::none);
if (!serialRPC.IsValid()) {
    std::cerr << "Connect failed" << std::endl;
    return;
}
serialRPC.StartGrabbing();
```

### Basic structure

The basic structure are declared in [rpc_protocol.hpp](https://github.com/StephanXu/BusPlot/blob/main/src/rpc_protocol.hpp). A `RPCRequest` is composed of `SOF`, `FrameHeader`, `Request` and `FrameTail`. `Request` could be various types such as `VariableAliasReq`, `UpdateVariableReq`, etc.

**The declaration of `RPCRequest`**

```c++
template<class ReqType>
struct RPCRequest {
    uint8_t m_SOF{};
    FrameHeader m_Header;
    ReqType m_Request;
    FrameTail m_Tail;
};
```

#### Header

The frame header has two properties, `DataLength` describes the size of `Request` in bytes and `Command` specifies a specific function to call.

**The declaration of `FrameHeader`:**

```c++
struct FrameHeader {
    uint8_t m_DataLength{};
    uint16_t m_Command{};
};
```

#### Request

Request need to contain a `COMMAND` member to indicate its command value.

**The declaration of `VariableAliasReq`**

```c++
struct VariableAliasReq {
    static constexpr uint16_t COMMAND = 0x0010;
    uint16_t m_VariableId{};
    uint8_t m_Alias[10] = {};
};
```

#### Tail

Frame tail contains a CRC16 result which calculating the entire request except itself. The implementation of the CRC16 algorithm is contained in [crc.hpp](https://github.com/StephanXu/BusPlot/blob/main/src/crc.hpp) and [crc.cpp](https://github.com/StephanXu/BusPlot/blob/main/src/crc.cpp).

**The declaration of `FrameTail`**

```c++
struct FrameTail {
    uint16_t m_CRC16{};
};
```

