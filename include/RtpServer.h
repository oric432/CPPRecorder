#ifndef RTP_SERVER_H
#define RTP_SERVER_H

#include <boost/asio.hpp>

class RtpServer
{
public:
    static const int RTP_HEADER_SIZE = 12;
    static const int MAX_BUFFER_LENGTH = 256;

public:
private:
    boost::asio::ip::udp::socket m_socket;
};

#endif // RTP_SERVER_H