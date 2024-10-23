#include "RtpClient.h"
#include "Utils.h"

RtpClient::RtpClient(boost::asio::io_context &io_ctx, const std::string &host, const std::string &port)
    : m_socket(io_ctx, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), m_timestamp(0), m_sequenceNumber(0), m_ssrc(generateSSRC()),
      m_resetTimer(io_ctx, std::chrono::seconds(10))
{
    boost::asio::ip::udp::resolver resolver(io_ctx);
    boost::asio::ip::udp::resolver::results_type endpoints =
        resolver.resolve(boost::asio::ip::udp::v4(), host, port);

    m_endpoint = *endpoints.begin();
}

RtpClient::RtpClient(RtpClient &&other) noexcept
    : m_socket(std::move(other.m_socket)),
      m_endpoint(std::move(other.m_endpoint)),
      m_sequenceNumber(other.m_sequenceNumber),
      m_timestamp(other.m_timestamp),
      m_ssrc(other.m_ssrc),
      m_resetTimer(std::move(other.m_resetTimer))
{
}

RtpClient &RtpClient::operator=(RtpClient &&other) noexcept
{
    if (this != &other)
    {
        m_socket = std::move(other.m_socket);
        m_endpoint = std::move(other.m_endpoint);
        m_sequenceNumber = other.m_sequenceNumber;
        m_timestamp = other.m_timestamp;
        m_ssrc = other.m_ssrc;
        m_resetTimer = std::move(other.m_resetTimer);
    }
    return *this;
}

RtpClient::~RtpClient()
{
    m_socket.close();
}

void RtpClient::sendAudioData(const float *audioBuffer, size_t length)
{
    std::cout << "hello " << m_endpoint << '\n';

    // Allocate buffer for the RTP packet
    size_t packetBufferSize = RtpClient::RTP_HEADER_SIZE + length * sizeof(float);
    uint8_t packetBuffer[packetBufferSize];

    createRTPPacket(0, m_sequenceNumber, m_timestamp, m_ssrc, audioBuffer, length, packetBuffer, packetBufferSize);

    m_socket.async_send_to(boost::asio::buffer(packetBuffer, packetBufferSize), m_endpoint,
                           [this, length](const boost::system::error_code &ec, std::size_t bytes_sent)
                           {
                               if (!ec)
                               {
                                   std::cout << "Sent RTP packet: " << bytes_sent << " Bytes" << '\n';
                               }
                               else
                               {
                                   std::cout << ec.message() << '\n';
                               }
                           });
    m_sequenceNumber++;
    m_timestamp += length;
}

void RtpClient::startResetTimer()
{

    m_resetTimer.async_wait([this](const boost::system::error_code &ec)
                            {
            if (!ec) {
                m_sequenceNumber = 0;
                m_timestamp = 0;
                m_resetTimer.expires_at(m_resetTimer.expiry() + std::chrono::seconds(10));
                startResetTimer();
            } });
}

void RtpClient::setTimestamp(uint32_t timestamp)
{
    m_timestamp = timestamp;
}

uint32_t RtpClient::getTimestamp()
{
    return m_timestamp;
}
