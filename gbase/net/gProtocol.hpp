#include <type_traits>
#include <concepts>
#include <cstddef>
#include <cstdint>
constexpr uint8_t __G_PROTOCOL_MAJOR_VERSION__ = 1;
/*
    gProtocol - Defines how application handles message delivery to another process via network.
        Version 1
            supports application level message delivery via TCP/IP protocol.
            Defines how to identify start and end of message byte stream

            +----------------------------------------------------------+
            |               (Application Layer) gProtocol              |
            +----------------------------------------------------------+
            |                         TCP                              |
            +----------------------------------------------------------+
            |                    internet protocol                     |
            +----------------------------------------------------------+
            |                  communication network                   |
            +----------------------------------------------------------+

            Frame
                - header - 1 byte
                    - start of message  = 0x1
                    - message partition = 0x2
                - packet
                    - size of data - 16 bytes
                    - data - (size of data) bytes

                    0                   1
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |     header    | proto version |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |         size of data          |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    ~            data               ~
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            
            protocol negotiation
                Client                          Server
                   |                               |
                   |------------startSession------>|
                   |<------------AckStart----------|
                   |                               |
                   |                               |
                   |----------StartDataTrans------>|
                   |<-------------Acked------------|
                   |-------------SendData--------->|
                   |-------------SendData--------->|
                   |-------------SendData--------->|
                   |-------------SendData--------->|
                   |-----------EndDataTrans------->|
                   |<-------------Acked------------|
                   |                               |
                   |<----------StartDataTrans------|
                   |--------------Acked----------->|
                   |<------------RecData-----------|
                   |<------------RecData-----------|
                   |<------------RecData-----------|
                   |<------------RecData-----------|
                   |<----------EndDataTrans--------|
                   |--------------Acked----------->|
                   |                               |
                   |                               |
                   |-----------EndSession--------->|
                   |<-------------Acked------------|


*/
namespace gbase::net::gProtocol::v1
{

    static constexpr uint8_t    START_SESSION             = static_cast<uint8_t>(0x1);
    static constexpr uint8_t    START_SESSION_ACK         = static_cast<uint8_t>(0x2);
    static constexpr uint8_t    END_SESSION               = static_cast<uint8_t>(0x3);
    static constexpr uint8_t    END_SESSION_ACK           = static_cast<uint8_t>(0x4);
    static constexpr uint8_t    START_DATA_TRANSMISSION   = static_cast<uint8_t>(0x5);
    static constexpr uint8_t    DATA_TRANSMISSION_ACK     = static_cast<uint8_t>(0x6);
    static constexpr uint8_t    END_DATA_TRANSMISSION     = static_cast<uint8_t>(0x7);
    static constexpr uint8_t    END_DATA_TRANSMISSION_ACK = static_cast<uint8_t>(0x8);

    uint16_t __header_and_proto_version__ = 0x0;
    uint16_t __size_of_data__ = 0x0;

    template <typename T, typename Ret>
    concept ClientSideProtocolApplicable = requires(T t) {
        { t.initialize() } -> std::same_as<Ret>;
        { t.send() } -> std::same_as<Ret>;
        { t.recieve() } -> std::same_as<Ret>;
    };

    namespace client
    {
        template <ClientSideProtocolApplicable<void> IPCMethod>
        class Protocol
        {
        private:
            IPCMethod method;

        public:
            Protocol() = default;
            ~Protocol() = default;

            void initialize(IPCMethod &method)
            {
                this->method = method;
                method.initilize();
                // protocol negotiation
                __header_and_proto_version |= START_SESSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                // sends start
            }

            void send()
            {
                this->method.send();
            }; // send to server
            void recieve();
        };
    } // namespace client

    namespace server
    {
        template <class IPCMethod>
        class Protocol
        {
        private:
            /* data */
        public:
            Protocol();
            ~Protocol();

            void send(); // send to client
            void recieve();
        };
    } // namespace server

} // namespace gbase::net::gProtocol
