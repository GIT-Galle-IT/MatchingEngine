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
    static constexpr uint8_t DATA_ARRIVAL = static_cast<uint8_t>(0x9);
    static constexpr uint8_t DATA_RECEIVED_BY_CLIENT = static_cast<uint8_t>(0x10);

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
            APPLICATION_DATA_TRANSMISSION_COMPLETED,
            APPLICATION_DATA_TRANSMITTING,
            APPLICATION_DATA_RECEIVING,
            APPLICATION_DATA_RECEPTION_COMPLETED,
            START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING,
            START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED,
            END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING,
            END_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED,

            CONNECTED,
            IDLE
        };

        enum TrasnmittingDataType : int
        {
            APPLICATION_DATA,
            PROTOCOL_DATA
        };

        template <class IPCMethod>
        class Protocol
        {

        private:
            struct
            {
                TrasnmittingDataType _data_type_;
                gbase::ByteBuffer<std::byte> _data_;
            } DataWithMetaData;

            using ClientId = int;
            using ListOfData = std::queue<DataWithMetaData>;

            std::flat_map<ClientId, State> __client_states;
            std::flat_map<ClientId, ListOfData> __data_waiting_to_sent;
            std::flat_map<ClientId, ListOfData> __data_waiting_to_receive;

        public:
            Protocol() = default;
            ~Protocol() = default;

            void onClientConnect(ClientId client_id)
            {
                __client_states.emplace(client_id, State::CONNECTED);
            }

            [[nodiscard]] auto applyOnSend(ClientId client_id, gbase::ByteBuffer<std::byte> &data) -> gbase::ByteBuffer<std::byte>
            {
                if (data.get_filled_size() > 0)
                {
                    // application has data to sent
                    __size_of_data__ = data.get_filled_size(); // fix warning
                    __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    gbase::ByteBuffer<std::byte> war_head;

                    // send sot
                    war_head.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    war_head.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));

                    if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(war__header_and_proto_version___head)});
                        q.push({TrasnmittingDataType::APPLICATION_DATA, std::move(data)});
                    }
                    else
                    {
                        ListOfDataToBeSent q;
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(war_head)});
                        q.push({TrasnmittingDataType::APPLICATION_DATA, std::move(data)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                }

                gbase::ByteBuffer<std::byte> ret_data;
                if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                {
                    switch (client_state)
                    {
                    case State::CONNECTED:
                    case State::IDLE:
                        if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                        {
                            if (q.front()._data_type_ == TrasnmittingDataType::PROTOCOL_DATA)
                            {
                                ret_data = q.front();
                                q.pop();

                                uint16_t proto_header_and_version_to_send = 0x0;
                                data.read<uint16_t>(reinterpret_cast<char *>(&proto_header_and_version_to_send));

                                uint8_t proto_header_to_send = proto_header >> 8;

                                if (proto_header_to_send & START_DATA_TRANSMISSION)
                                    __client_states.emplace(client_id, State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);
                                if (proto_header_to_send & END_DATA_TRANSMISSION)
                                    __client_states.emplace(client_id, State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);
                                if (proto_header_to_send & START_DATA_TRANSMISSION_ACK)
                                    __client_states.emplace(client_id, State::APPLICATION_DATA_TRANSMITTING);
                                if (proto_header_to_send & END_DATA_TRANSMISSION_ACK)
                                    __client_states.emplace(client_id, State::IDLE);

                                // else - sending acks
                            }
                        }
                        break;
                    case State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED:
                        if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                        {
                            if (q.front()._data_type_ == TrasnmittingDataType::APPLICATION_DATA)
                            {
                                __header_and_proto_version |= (uint16_t)DATA_ARRIVAL << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                                gbase::ByteBuffer<std::byte> data_with_hdr;
                                ret_data.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                                ret_data.append(q.front().get(), q.front().get_filled_size());
                                q.pop();

                                __client_states.emplace(client_id, State::APPLICATION_DATA_TRANSMITTING);
                            }
                        }
                        break;
                    case State::APPLICATION_DATA_TRANSMISSION_COMPLETED:
                        __client_states.emplace(client_id, State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);
                        __header_and_proto_version |= (uint16_t)END_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                        ret_data.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                        break;
                    case State::END_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED:
                        __client_states.emplace(client_id, State::IDLE);
                        break;
                    case State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING:   // concurrency control
                    case State::APPLICATION_DATA_TRANSMITTING:                   // concurrency control
                    case State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING: // concurrency control
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    GLOG_ERROR("No client found for id {}", client_id)
                }
                __size_of_data__ = 0x0;
                __header_and_proto_version = 0x0;
                return ret_data;
            };

            void recieve(ClientId client_id, gbase::ByteBuffer<std::byte> &data)
            {
                gbase::ByteBuffer<std::byte> ret_date;
                // WARNING : TODO make sure it is a header (handle)
                data.read<uint16_t>(reinterpret_cast<char *>(&__header_and_proto_version));

                uint8_t header = (uint8_t)(__header_and_proto_version >> 8);
                switch (header)
                {
                // client side intiations
                case START_SESSION: // client starts
                    __header_and_proto_version |= (uint16_t)START_SESSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                    gbase::ByteBuffer<std::byte> ack;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        ListOfDataToBeSent q;
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;
                case START_DATA_TRANSMISSION: // client starts
                    data.read<uint16_t>(reinterpret_cast<char *>(&__size_of_data__));

                    if (auto &q = __data_waiting_to_receive.find(client_id); q == __data_waiting_to_receive.end())
                    {
                        ListOfDataToBeSent q;
                        __data_waiting_to_receive.emplace(client_id, q);
                    }

                    __header_and_proto_version |= (uint16_t)START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                    gbase::ByteBuffer<std::byte> ack;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    ack.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
                    if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        ListOfDataToBeSent q;
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;
                case END_DATA_TRANSMISSION: // client ends
                    if (auto &q = __data_waiting_to_receive.find(client_id); q != __data_waiting_to_receive.end())
                    {
                        ret_data = q.front();
                        q.pop();
                    }

                    __header_and_proto_version |= (uint16_t)END_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                    gbase::ByteBuffer<std::byte> ack;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        ListOfDataToBeSent q;
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;
                case END_SESSION: // client ends
                    __header_and_proto_version |= (uint16_t)END_SESSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                    gbase::ByteBuffer<std::byte> ack;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version), sizeof(uint16_t));
                    if (auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        ListOfDataToBeSent q;
                        q.push({TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;

                // // server side initiated actions acknowledgment handling
                case START_DATA_TRANSMISSION_ACK:
                    if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                    {
                        if (client_state == State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING)
                        {
                            __client_states.emplace(client_id, State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED);
                        }
                    }
                    break;
                case DATA_ARRIVAL:
                    if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                    {
                        if (client_state == State::APPLICATION_DATA_RECEIVING)
                        {
                            if (auto &q = __data_waiting_to_receive.find(client_id); q != __data_waiting_to_receive.end())
                            {
                                data.read<uint16_t>(reinterpret_cast<char *>(&__size_of_data__));
                                void *app_data = malloc(__size_of_data__);
                                data.read<__size_of_data__>(reinterpret_cast<const char *>(&app_data));
                                q.back().append(static_cast<const char *>(&app_data), __size_of_data__);
                                __data_waiting_to_receive.emplace(client_id, q);
                                free(app_data);
                            }

                            __client_states.emplace(client_id, State::APPLICATION_DATA_RECEPTION_COMPLETED);
                        }
                    }
                    break;
                case DATA_RECEIVED_BY_CLIENT:
                    if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                    {
                        if (client_state == State::APPLICATION_DATA_TRANSMITTING)
                        {
                            __client_states.emplace(client_id, State::APPLICATION_DATA_TRANSMISSION_COMPLETED);
                        }
                    }
                    break;
                case END_DATA_TRANSMISSION_ACK:
                    if (auto client_state = __client_states.find(client_id); client_state != __client_states.end())
                    {
                        if (client_state == State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING)
                        {
                            __client_states.emplace(client_id, State::END_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED);
                        }
                    }
                    break;

                default:
                    break;
                }
                __size_of_data__ = 0x0;
                __header_and_proto_version__ = 0x0;
                return ret_data;
            };
        };
    } // namespace server

} // namespace gbase::net::gProtocol
