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
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);

    gbase::net::gProtocol::v1::server::Protocol server_protocol;
    int client_id = 1;

    // client connect
    server_protocol.onClientConnect(client_id);

    auto client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::CONNECTED);

    // apply send data protocol to client - this will return protocol data to send over the wire
    gbase::ByteBuffer<std::byte> bytes_to_sent_over_wire{std::move(server_protocol.send(1, bb))};

    uint16_t __header_and_proto_version__ = 0x0;
    bytes_to_sent_over_wire.read<sizeof(uint16_t)>(reinterpret_cast<char *>(&__header_and_proto_version__));
    uint8_t header = (__header_and_proto_version__ >> 8);
    EXPECT_EQ(header, gbase::net::gProtocol::v1::START_DATA_TRANSMISSION);
    
    // next to sent is application data
    auto data_waiting_to_sent = server_protocol.getDataWaitingToSent();
    for (; !data_waiting_to_sent[client_id].empty(); data_waiting_to_sent[client_id].pop())
        EXPECT_EQ(data_waiting_to_sent[client_id].front()._data_type_, gbase::net::gProtocol::v1::server::TrasnmittingDataType::APPLICATION_DATA);

    // client is waiting for ack to start data transmission
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::START_APPLICATION_DATA_TRANSMISSION_ACK_WAITING);

    // client sent ack for data transmission
    gbase::ByteBuffer<std::byte> client_ack;
    uint16_t __header_and_proto_version__CLIENT = 0x0;
    __header_and_proto_version__CLIENT |= (uint16_t)gbase::net::gProtocol::v1::START_DATA_TRANSMISSION_ACK << 8 | __G_PROTOCOL_MAJOR_VERSION__;
    client_ack.append(reinterpret_cast<const char *>(&__header_and_proto_version__CLIENT), sizeof(uint16_t));

    server_protocol.recieve(1, client_ack);
    client_states = server_protocol.getClientStates();
    EXPECT_EQ(client_states[client_id], gbase::net::gProtocol::v1::server::State::START_APPLICATION_DATA_TRANSMISSION_ACK_RECEIVED);
    
}
