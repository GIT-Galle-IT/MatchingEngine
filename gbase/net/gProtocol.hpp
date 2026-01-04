#include <type_traits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ByteBuffer.hpp>
#include <flat_map> // C++23 header
#include <queue>
#include <bitset>

#include <logging/gLog.h>

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

        class Protocol
        {

        private:
            struct DataWithMetaData
            {
                size_t __size_of_data__ = 0x0;
                TrasnmittingDataType _data_type_;
                gbase::ByteBuffer<std::byte> _data_;
            };

            using ClientId = int;
            using QueueOfData = std::queue<DataWithMetaData>;

            std::flat_map<ClientId, State> __client_states;
            std::flat_map<ClientId, QueueOfData> __data_waiting_to_sent;
            std::flat_map<ClientId, QueueOfData> __data_waiting_to_receive;

        public:
            Protocol() = default;
            ~Protocol() = default;

            // for testing
            [[nodiscard]] auto getClientStates() const -> std::flat_map<ClientId, State>
            {
                return __client_states;
            }

            [[nodiscard]] auto getDataWaitingToSent() const -> std::flat_map<ClientId, QueueOfData>
            {
                return __data_waiting_to_sent;
            }

            [[nodiscard]] auto getDataWaitingToRecieve() const -> std::flat_map<ClientId, QueueOfData>
            {
                return __data_waiting_to_receive;
            }

            void onClientConnect(ClientId client_id)
            {
                __client_states.emplace(client_id, State::CONNECTED);
            }

            [[nodiscard]] auto send(ClientId client_id, gbase::ByteBuffer<std::byte> &data) -> gbase::ByteBuffer<std::byte>
            {
                GLOG_INFO("Server protocol send called client {}", client_id)
                if (data.get_filled_size() > 0)
                {
                    // application has data to sent
                    __size_of_data__ = data.get_filled_size(); // fix warning
                    __header_and_proto_version__ |= (uint16_t)START_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    gbase::ByteBuffer<std::byte> war_head;

                    // send sot
                    war_head.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                    war_head.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));

                    if (const auto &q = __data_waiting_to_sent.find(client_id); q != __data_waiting_to_sent.end())
                    {
                        q->second.push({__size_of_data__, TrasnmittingDataType::PROTOCOL_DATA, std::move(war_head)});
                        q->second.push({__size_of_data__, TrasnmittingDataType::APPLICATION_DATA, std::move(data)});
                    }
                    else
                    {
                        QueueOfData new_q;
                        new_q.push({__size_of_data__, TrasnmittingDataType::PROTOCOL_DATA, std::move(war_head)});
                        new_q.push({__size_of_data__, TrasnmittingDataType::APPLICATION_DATA, std::move(data)});
                        __data_waiting_to_sent.emplace(client_id, new_q);
                    }
                }

                gbase::ByteBuffer<std::byte> empty_data;
                if (const auto &itr = __client_states.find(client_id); itr != __client_states.end())
                {
                    State client_state = itr->second;
                    switch (client_state)
                    {
                    case State::CONNECTED:
                    case State::IDLE:
                        if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                        {
                            auto &q = itr->second;
                            if (q.front()._data_type_ == TrasnmittingDataType::PROTOCOL_DATA)
                            {
                                gbase::ByteBuffer<std::byte> pending_data{std::move(q.front()._data_)};
                                q.front()._data_.release();
                                q.pop();

                                uint16_t proto_header_and_version_to_send = 0x0;
                                pending_data.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&proto_header_and_version_to_send));

                                uint8_t proto_header_to_send = proto_header_and_version_to_send >> 8;

                                if (proto_header_to_send == START_DATA_TRANSMISSION)
                                    __client_states[client_id] = State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING;
                                if (proto_header_to_send == END_DATA_TRANSMISSION)
                                    __client_states[client_id] = State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING;
                                if (proto_header_to_send == START_DATA_TRANSMISSION_ACK) // sends to client acking thatcliant starting data transmission
                                    __client_states[client_id] = State::APPLICATION_DATA_RECEIVING;
                                if (proto_header_to_send == END_DATA_TRANSMISSION_ACK)
                                    __client_states[client_id] = State::IDLE;

                                // else - sending acks
                                return pending_data;
                            }
                        }
                        break;
                    case State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED:
                        if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                        {
                            auto &pending_data = itr->second.front();
                            if (pending_data._data_type_ == TrasnmittingDataType::APPLICATION_DATA)
                            {
                                __header_and_proto_version__ |= (uint16_t)DATA_ARRIVAL << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                                __size_of_data__ = pending_data._data_.get_filled_size();
                                gbase::ByteBuffer<std::byte> data_with_hdr;
                                data_with_hdr.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                                data_with_hdr.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
                                data_with_hdr.append(pending_data._data_.get().get(), pending_data._data_.get_filled_size());
                                pending_data._data_.release();
                                itr->second.pop();

                                __client_states[client_id] = State::APPLICATION_DATA_TRANSMITTING;
                                return data_with_hdr;
                            }
                            // else invalid protocol flow
                        }
                        break;
                    case State::APPLICATION_DATA_TRANSMISSION_COMPLETED:
                        __client_states[client_id] = State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING;
                        __header_and_proto_version__ |= (uint16_t)END_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
                        empty_data.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
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
                __header_and_proto_version__ = 0x0;
                return empty_data;
            };

            auto recieve(ClientId client_id, gbase::ByteBuffer<std::byte> &data) -> gbase::ByteBuffer<std::byte>
            {
                // WARNING : TODO make sure it is a header (handle)
                data.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
                gbase::ByteBuffer<std::byte> ack;

                uint8_t header = (uint8_t)(__header_and_proto_version__ >> 8);
                __header_and_proto_version__ = 0x0;
                gbase::ByteBuffer<std::byte> empty_data;
                switch (header)
                {
                // client side intiations
                case START_SESSION: // client starts
                    __header_and_proto_version__ |= (uint16_t)START_SESSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                    if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                    {
                        itr->second.push({sizeof(uint16_t), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        QueueOfData q;
                        q.push({sizeof(uint16_t), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;
                case START_DATA_TRANSMISSION: // client starts
                    data.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__size_of_data__));

                    if (const auto &itr = __data_waiting_to_receive.find(client_id); itr == __data_waiting_to_receive.end())
                    {
                        QueueOfData q;
                        __data_waiting_to_receive.emplace(client_id, q);
                    }

                    __header_and_proto_version__ |= (uint16_t)START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                    ack.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
                    if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                    {
                        itr->second.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        QueueOfData q;
                        q.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;
                case DATA_ARRIVAL:
                    if (const auto &itr = __client_states.find(client_id); itr != __client_states.end())
                    {
                        auto &client_state = itr->second;
                        if (client_state == State::APPLICATION_DATA_RECEIVING)
                        {
                            if (const auto &itr = __data_waiting_to_receive.find(client_id); itr != __data_waiting_to_receive.end())
                            {
                                data.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__size_of_data__));

                                char *app_data = new char[__size_of_data__+1];

                                data.read(sizeof(uint16_t) << 1, __size_of_data__, app_data);
                                app_data[__size_of_data__] = '\0';

                                gbase::ByteBuffer<std::byte> received_data;
                                received_data.append(static_cast<const char *>(app_data), __size_of_data__);
                                itr->second.push({ack.get_filled_size(), TrasnmittingDataType::APPLICATION_DATA, std::move(received_data)});

                                // __data_waiting_to_receive.emplace(client_id, q);
                                delete[] app_data;
                            }

                            __client_states[client_id] = State::APPLICATION_DATA_RECEPTION_COMPLETED;
                        }
                    }
                    break;
                case END_DATA_TRANSMISSION: // client ends
                    __header_and_proto_version__ |= (uint16_t)END_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                    if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                    {
                        itr->second.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        QueueOfData q;
                        q.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    __client_states[client_id] = State::IDLE;
                    if (const auto &itr = __data_waiting_to_receive.find(client_id); itr != __data_waiting_to_receive.end())
                    {
                        auto &q = itr->second;
                        gbase::ByteBuffer<std::byte> pending_data{std::move(q.front()._data_)};
                        q.front()._data_.release();
                        q.pop();

                        return pending_data;
                    }
                    break;
                case END_SESSION: // client ends
                    __header_and_proto_version__ |= (uint16_t)END_SESSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;

                    // send sot
                    ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__), sizeof(uint16_t));
                    if (const auto &itr = __data_waiting_to_sent.find(client_id); itr != __data_waiting_to_sent.end())
                    {
                        itr->second.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                    }
                    else
                    {
                        QueueOfData q;
                        q.push({ack.get_filled_size(), TrasnmittingDataType::PROTOCOL_DATA, std::move(ack)});
                        __data_waiting_to_sent.emplace(client_id, q);
                    }
                    break;

                // // server side initiated actions acknowledgment handling
                case START_DATA_TRANSMISSION_ACK:
                    if (const auto &itr = __client_states.find(client_id); itr != __client_states.end())
                    {
                        auto &client_state = itr->second;
                        if (client_state == State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING)
                        {
                            __client_states[client_id] = State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED;
                        }
                    }
                    break;
                case DATA_RECEIVED_BY_CLIENT:
                    if (const auto &itr = __client_states.find(client_id); itr != __client_states.end())
                    {
                        const auto &client_state = itr->second;
                        if (client_state == State::APPLICATION_DATA_TRANSMITTING)
                        {
                            __client_states[client_id] = State::APPLICATION_DATA_TRANSMISSION_COMPLETED;
                        }
                    }
                    break;
                case END_DATA_TRANSMISSION_ACK:
                    if (const auto &itr = __client_states.find(client_id); itr != __client_states.end())
                    {
                        const auto &client_state = itr->second;
                        if (client_state == State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING)
                        {
                            __client_states[client_id] = State::IDLE;
                        }
                    }
                    break;

                default:
                    break;
                }
                __size_of_data__ = 0x0;
                __header_and_proto_version__ = 0x0;
                return empty_data;
            };
        };
    } // namespace server

} // namespace gbase::net::gProtocol
