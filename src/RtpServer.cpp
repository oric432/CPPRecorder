#include "RtpServer.h"
#include "Utils.h"

RtpServer::RtpServer(boost::asio::io_context &io_ctx, unsigned short port) : m_socket(io_ctx, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),
                                                                             m_rtpServerManager(io_ctx)
{
    std::cout << "Server is up and running" << std::endl;
}
RtpServer::~RtpServer()
{
    m_socket.close();
}

void RtpServer::recieveDataFromClient()
{
    m_socket.async_receive(
        boost::asio::buffer(m_data, MAX_LENGTH),
        [this](const boost::system::error_code &ec, std::size_t bytes_recvd)
        {
            if (!ec && bytes_recvd > 0)
            {
                std::cout << "Recieved rtp packet: " << bytes_recvd << " Bytes" << "\n";

                rtpClientInfo client;
                uint32_t ssrc;

                decodeRTPPacket(m_data, bytes_recvd, client.sequenceNumber, client.timestamp, ssrc);

                m_rtpServerManager.addClient(ssrc, client);

                std::cout << "Client Data: \n"
                          << client.sequenceNumber << "\n"
                          << client.timestamp << "\n";

                m_rtpServerManager.manageBuffer(ssrc, m_data + RTP_HEADER_SIZE, bytes_recvd - RTP_HEADER_SIZE);
            }

            recieveDataFromClient();
        });
}

void RtpServer::sendDataToClient()
{
}