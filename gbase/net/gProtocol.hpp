#include <type_traits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ByteBuffer.hpp>
#include <flat_map> // C++23 header

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

    static constexpr uint8_t START_SESSION = static_cast<uint8_t>(0x1);
    static constexpr uint8_t START_SESSION_ACK = static_cast<uint8_t>(0x2);
    static constexpr uint8_t END_SESSION = static_cast<uint8_t>(0x3);
    static constexpr uint8_t END_SESSION_ACK = static_cast<uint8_t>(0x4);
    static constexpr uint8_t START_DATA_TRANSMISSION = static_cast<uint8_t>(0x5);
    static constexpr uint8_t START_DATA_TRANSMISSION_ACK = static_cast<uint8_t>(0x6);
    static constexpr uint8_t END_DATA_TRANSMISSION = static_cast<uint8_t>(0x7);
    static constexpr uint8_t END_DATA_TRANSMISSION_ACK = static_cast<uint8_t>(0x8);

    uint16_t __header_and_proto_version__ = 0x0;
    uint16_t __size_of_data__ = 0x0;

    template <typename T, typename Ret>
    concept ClientSideProtocolApplicable = requires(T t) {
        { t.initialize() } -> std::same_as<Ret>;
        { t.send() } -> std::same_as<Ret>;
        { t.receive() } -> std::same_as<Ret>;
    };

    namespace client
    {
        template <ClientSideProtocolApplicable<void> IPCMethod>
        class Protocol
        {
        public:
            Protocol() = default;
            ~Protocol() = default;

            void initialize(IPCMethod &method)
            {
                this->__method = method;
                __method.initilize();
                // protocol negotiation
                __header_and_proto_version |= (uint16_t)START_SESSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                gbase::ByteBuffer<std::byte> bb;

                // sends start
                bb.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                __method.send(std::move(bb));

                // receive ack
                bb = __method.receive();
                bb.read<uint16_t>(reinterpret_cast<char *>(&__header_and_proto_version));
                bb.read<uint16_t>(reinterpret_cast<char *>(&__size_of_data__));
                bb.read<static_cast<size_t>(__size_of_data__)>(reinterpret_cast<char *>(&__client_id)); // keep size of data to 2 bytes for client id in ACK

                uint8_t ack_header = (uint8_t)(__header_and_proto_version >> 8) & START_SESSION_ACK;
                uint8_t server_proto_version = __header_and_proto_version << 8;

                if (ack_header != START_SESSION_ACK)
                {
                    GLOG_ERROR("Invalid ack received from server {}", ack_header);
                    return;
                }

                if (server_proto_version != __G_PROTOCOL_MAJOR_VERSION__)
                {
                    GLOG_ERROR("Invalid protocol version in server {} and yours {}", server_proto_version, __G_PROTOCOL_MAJOR_VERSION__);
                    return;
                }

                __header_and_proto_version = 0x0;
                __size_of_data__ = 0x0;
            }

            static gbase::ByteBuffer<std::byte> applyOnSend(gbase::ByteBuffer<std::byte> &data)
            {
                __size_of_data__ = data.get_filled_size(); // fix warning
                __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                gbase::ByteBuffer<std::byte> bb;

                // send sot
                bb.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                bb.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
                bb.append(data.get().get(), data.get_filled_size());

                return bb;
            }; // send to server

            static auto recieve() -> gbase::ByteBuffer<std::byte> &&
            {
                gbase::ByteBuffer<std::byte> bb;

                bb = method.receive();
                bb.read<uint16_t>(reinterpret_cast<char *>(&__header_and_proto_version));
                bb.read<uint16_t>(reinterpret_cast<char *>(&__size_of_data__));
                uint16_t target_client_id = 0x0;
                bb.read<uint16_t>(reinterpret_cast<char *>(&target_client_id));

                uint8_t ack_header = (uint8_t)(__header_and_proto_version >> 8) & START_DATA_TRANSMISSION;

                if (ack_header != START_DATA_TRANSMISSION || target_client_id != __client_id)
                {
                    GLOG_ERROR("Invalid ack received from server {}. unable to start data receiving transmission", ack_header);
                    return;
                }

                bb.release();

                __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                bb.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                bb.append(reinterpret_cast<const char *>(&__client_id), sizeof(uint16_t));
                __method.send(std::move(bb));

                bb.release();

                while (bb.get_filled_size() < __size_of_data__)
                {
                    bb = __method.receive();
                }

                return std::move(bb);
            };
        };
    } // namespace client

    namespace server
    {
        enum State : int
        {
            APPLICATION_DATA_COMPLETE,
            APPLICATION_DATA_WAITING,
            START_DATA_TRANSMISSION_ACK_WAITING,
            START_DATA_TRANSMISSION_ACK_RECEIVED,
            END_DATA_TRANSMISSION_ACK_WAITING,
            END_DATA_TRANSMISSION_ACK_RECEIVED
        };

        template <class IPCMethod>
        class Protocol
        {

        private:
            using ClientId = int;
            std::flat_map<ClientId, State> __client_states;
            std::flat_map<ClientId, gbase::ByteBuffer<std::byte>> __data_waiting_to_sent;

        public:
            Protocol() = default;
            ~Protocol() = default;

            auto applyOnSend(ClientId client_id, gbase::ByteBuffer<std::byte> &data) -> gbase::ByteBuffer<std::byte>
            {
                if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                {
                    switch (client_state)
                    {
                    case State::APPLICATION_DATA_COMPLETE:
                        /* code */
                        break;
                    case State::APPLICATION_DATA_WAITING:
                        /* code */
                        break;
                    case State::START_DATA_TRANSMISSION_ACK_WAITING:
                        /* code */
                        break;
                    case State::START_DATA_TRANSMISSION_ACK_RECEIVED:
                        __client_states.emplace(client_id, State::START_DATA_TRANSMISSION_ACK_RECEIVED);
                        if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                        {
                            if (client_state == State::START_DATA_TRANSMISSION_ACK_WAITING)
                            {
                                __client_states.emplace(client_id, State::START_DATA_TRANSMISSION_ACK_RECEIVED);
                            }
                        }
                        break;
                    case State::END_DATA_TRANSMISSION_ACK_WAITING:
                        /* code */
                        break;
                    case State::END_DATA_TRANSMISSION_ACK_RECEIVED:
                        /* code */
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    __size_of_data__ = data.get_filled_size(); // fix warning
                    __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                    gbase::ByteBuffer<std::byte> bb;

                    // send sot
                    bb.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    bb.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
                    // bb.append(data.get().get(), data.get_filled_size());

                    __client_states.emplace(client_id, START_DATA_TRANSMISSION_ACK_WAITING);
                    __data_waiting_to_sent.emplace(client_id, std::move(data));
                    return bb;
                }
            }; // send to server

            auto recieve(ClientId client_id, gbase::ByteBuffer<std::byte> &data) -> state
            {
                data.read<uint16_t>(reinterpret_cast<char *>(&__header_and_proto_version));
                data.read<uint16_t>(reinterpret_cast<char *>(&__size_of_data__));

                uint8_t ack_header = (uint8_t)(__header_and_proto_version >> 8);

                switch (ack_header)
                {
                case START_SESSION:
                    /* code */
                    break;
                case START_SESSION_ACK:
                    /* code */
                    break;
                case END_SESSION:
                    /* code */
                    break;
                case END_SESSION_ACK:
                    /* code */
                    break;
                case START_DATA_TRANSMISSION:
                    /* code */
                    break;
                case START_DATA_TRANSMISSION_ACK:
                    if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                    {
                        if (client_state == State::START_DATA_TRANSMISSION_ACK_WAITING)
                        {
                            __client_states.emplace(client_id, State::START_DATA_TRANSMISSION_ACK_RECEIVED);
                        }
                    }
                    break;
                case END_DATA_TRANSMISSION:
                    /* code */
                    break;
                case END_DATA_TRANSMISSION_ACK:
                    /* code */
                    break;

                default:
                    break;
                }

                // if (ack_header != START_DATA_TRANSMISSION || target_client_id != __client_id)
                // {
                //     GLOG_ERROR("Invalid ack received from server {}. unable to start data receiving transmission", ack_header);
                //     return;
                // }

                // bb.release();

                // __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                // bb.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                // bb.append(reinterpret_cast<const char *>(&__client_id), sizeof(uint16_t));
                // __method.send(std::move(bb));

                // bb.release();

                // while (bb.get_filled_size() < __size_of_data__)
                // {
                //     bb = __method.receive();
                // }

                return std::move(bb);
            };
        };
    } // namespace server

} // namespace gbase::net::gProtocol
