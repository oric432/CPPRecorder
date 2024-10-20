#ifndef RTP_SERVER_H
#define RTP_SERVER_H

#include <boost/asio.hpp>
#include <iostream>
#include "RtpServerManager.h"

class RtpServer
{
public:
    static const int RTP_HEADER_SIZE = 12;
    static const int MAX_BUFFER_LENGTH = 256;

public:
    RtpServer(boost::asio::io_context &io_ctx, unsigned short port);

    void recieveDataFromClient();
    void sendDataToClient();

private:
    boost::asio::ip::udp::socket m_socket;
    enum
    {
        MAX_LENGTH = RTP_HEADER_SIZE + MAX_BUFFER_LENGTH * sizeof(float)
    };
    uint8_t m_data[MAX_LENGTH];
    RtpServerManager m_rtpServerManager;
};

#endif // RTP_SERVER_H