#include "Setup.hpp"
#include <ByteBuffer.hpp>
#include <demos/message.h>
#include <gProtocol.hpp>
#include <bitset>

/*

                std::bitset<16> x(__header_and_proto_version__);
                                std::cout << x << std::endl;
*/

TEST_F(BaseTest, protocol_send_data_server_to_client_testing)
{
    uint16_t __header_and_proto_version__ = 0x0;
    uint8_t header = 0x0;
    uint16_t __size_of_data__ = 0x0;
    
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};
    gbase::ByteBuffer<std::byte> bb_test;
    message.serialize(bb_test);

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);

    gbase::net::gProtocol::v1::server::Protocol server_protocol;
    int client_id = 1;

    // 1. client connect
    server_protocol.onClientConnect(client_id);

    auto client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::CONNECTED);

    // 2. apply send data protocol to client - this will return protocol data to send over the wire
    EXPECT_EQ(bb.get_filled_size(), 55 /*bytes*/); // should moved
    gbase::ByteBuffer<std::byte> bytes_to_sent_over_wire1{std::move(server_protocol.send(1, bb))};
    EXPECT_EQ(bb.get_filled_size(), 0 /*bytes*/); // should moved

    bytes_to_sent_over_wire1.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::START_DATA_TRANSMISSION);
    
    // 3. next to sent is application data
    auto data_waiting_to_sent = server_protocol.getDataWaitingToSent();
    for (; !data_waiting_to_sent[client_id].empty(); data_waiting_to_sent[client_id].pop())
        EXPECT_EQ(data_waiting_to_sent[client_id].front()._data_type_, gbase::net::gProtocol::v1::server::TrasnmittingDataType::APPLICATION_DATA);

    // 4. server is waiting for ack to start data transmission
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);

    // 5. client sent ack for data transmission
    gbase::ByteBuffer<std::byte> client_ack;
    uint16_t __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    // 6. client sent ack for data transmission - server received ack
    server_protocol.recieve(1, client_ack);
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED);

    // 7. after ack received send the data in next itr
    auto bytes_to_sent_over_wire2{std::move(server_protocol.send(1, bb))}; // bb is empty
    bytes_to_sent_over_wire2.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::DATA_ARRIVAL);

    bytes_to_sent_over_wire2.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__size_of_data__));
    EXPECT_EQ(__size_of_data__, bb_test.get_filled_size());
    
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::APPLICATION_DATA_TRANSMITTING);

    // 8. client sent ack for data received
    __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::DATA_RECEIVED_BY_CLIENT << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_ack.release();
    client_ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    server_protocol.recieve(1, client_ack);
    EXPECT_EQ(client_ack.get_filled_size(), 2 /*bytes*/); // should not be moved
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::APPLICATION_DATA_TRANSMISSION_COMPLETED);

    // 9. send end data transmission
    auto bytes_to_sent_over_wire3{std::move(server_protocol.send(1, bb))}; // bb is empty
    bytes_to_sent_over_wire3.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::END_DATA_TRANSMISSION);

    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::END_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);

    // 10. client sent ack for end data trasnmission
    __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::END_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_ack.release();
    client_ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    server_protocol.recieve(1, client_ack);
    EXPECT_EQ(client_ack.get_filled_size(), 2 /*bytes*/); // should not be moved
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::IDLE);

}

TEST_F(BaseTest, protocol_send_data_client_to_s_testing)
{
    uint16_t __header_and_proto_version__ = 0x0;
    uint8_t header = 0x0;
    uint16_t __size_of_data__ = 0x0;
    
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};
    gbase::ByteBuffer<std::byte> bb_test;
    message.serialize(bb_test);

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);

    gbase::ByteBuffer<std::byte> bb_empty;

    gbase::net::gProtocol::v1::server::Protocol server_protocol;
    int client_id = 1;

    // 1. server side setup
    server_protocol.onClientConnect(client_id);

    auto client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::CONNECTED);

    // 2. apply receive from client
    gbase::ByteBuffer<std::byte> client_data;
    uint16_t __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::START_SESSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_data.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    server_protocol.recieve(1, client_data);
    EXPECT_EQ(client_data.get_filled_size(), 2 /*bytes*/); // should not be moved
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::CONNECTED);
    
    // 3. next call send in the iteration
    auto bytes_to_sent_over_wire1{std::move(server_protocol.send(1, bb_empty))}; // bb is empty
    bytes_to_sent_over_wire1.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::START_SESSION_ACK);

    // 4. server receive from client that this is a start od data transmission
    __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::START_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_data.release();
    __size_of_data__ = bb.get_filled_size();
    client_data.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));
    client_data.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));

    server_protocol.recieve(1, client_data);
    EXPECT_EQ(client_data.get_filled_size(), 4 /*bytes*/); // should not be moved
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::CONNECTED);
    
    // 5. next call send in the iteration
    auto bytes_to_sent_over_wire2{std::move(server_protocol.send(1, bb_empty))}; // bb is empty
    bytes_to_sent_over_wire2.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::START_DATA_TRANSMISSION_ACK);

    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::APPLICATION_DATA_RECEIVING);

    // 6. receive data from client
    gbase::ByteBuffer<std::byte> data_with_hdr;
    __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::DATA_ARRIVAL << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    __size_of_data__ = bb.get_filled_size();
    
    data_with_hdr.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));
    data_with_hdr.append(reinterpret_cast<const char *>(&__size_of_data__), sizeof(uint16_t));
    data_with_hdr.append(bb.get().get(), bb.get_filled_size());
    
    server_protocol.recieve(1, data_with_hdr);
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::APPLICATION_DATA_RECEPTION_COMPLETED);
    

    // 7. receive data transmission complete
    __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::END_DATA_TRANSMISSION << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_data.release();
    client_data.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    server_protocol.recieve(1, client_data);
    EXPECT_EQ(client_data.get_filled_size(), 2 /*bytes*/); // should not be moved
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::IDLE);
    std::cout << "complete" << __size_of_data__ << std::endl;

    // 8. next call send in the iteration
    auto bytes_to_sent_over_wire3{std::move(server_protocol.send(1, bb_empty))}; // bb is empty
    bytes_to_sent_over_wire3.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::END_DATA_TRANSMISSION_ACK);

    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::IDLE);
    
}
