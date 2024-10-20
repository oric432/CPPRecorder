#include "RtpServer.h"

RtpServer::RtpServer(boost::asio::io_context &io_ctx, unsigned short port) : m_socket(io_ctx, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    std::cout << "Server is up and running" << std::endl;
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
            }

            recieveDataFromClient();
        });
}

void RtpServer::sendDataToClient()
{
}