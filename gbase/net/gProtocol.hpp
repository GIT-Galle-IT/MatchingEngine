constexpr int __G_PROTOCOL_MAJOR_VERSION__ = 1;
/*
    gProtocol - Defines how application handles message delivery to another process via network.
        Version 1 
            supports application level message delivery via TCP/IP protocol.
            Defines how to identify start and end of message byte stream

        Frame
            - header - 1 byte
                - start of message  = 0x1
                - message partition = 0x2
                - end of message    = 0x3
            - packet - size of data
                - data
*/
namespace gbase::net::gProtocol
{
    class ProtocolV1
    {
    private:
        /* data */
    public:
        ProtocolV1();
        ~ProtocolV1();
    };

} // namespace gbase::net::gProtocol
