#include "RtpClient.h"

RtpClient::RtpClient(boost::asio::io_context &io_ctx, const std::string &host, const std::string &port)
    : m_socket(io_ctx, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), m_timestamp(0), m_sequenceNumber(0), m_ssrc(generateSSRC())
{
    keepAlive(io_ctx);
    boost::asio::ip::udp::resolver resolver(io_ctx);
    boost::asio::ip::udp::resolver::results_type endpoints =
        resolver.resolve(boost::asio::ip::udp::v4(), host, port);

    m_endpoint = *endpoints.begin();
}

void RtpClient::keepAlive(boost::asio::io_context &io_ctx)
{
    std::cout << "what";
}

RtpClient::RtpClient(RtpClient &&other) noexcept
    : m_socket(std::move(other.m_socket)),
      m_endpoint(std::move(other.m_endpoint)),
      m_sequenceNumber(other.m_sequenceNumber),
      m_timestamp(other.m_timestamp),
      m_ssrc(other.m_ssrc)
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
    }
    return *this;
}

RtpClient::~RtpClient()
{
    m_socket.close();
}

void RtpClient::sendAudioData(const std::vector<float> &audioBuffer, size_t length)
{
    std::cout << "hello " << m_endpoint << '\n';
    std::vector<uint8_t> rtpPacket = createRTPPacket(0, m_sequenceNumber, m_timestamp, m_ssrc, audioBuffer, false);

    m_socket.async_send_to(boost::asio::buffer(rtpPacket, RtpClient::RTP_HEADER_SIZE + length), m_endpoint,
                           [](const boost::system::error_code &ec, std::size_t /*bytes_sent*/)
                           {
                               std::cout << "something" << '\n';
                               if (!ec)
                               {
                                   std::cout << "Sent rtp packet" << '\n';
                               }
                               else
                               {
                                   std::cout << ec.message() << '\n';
                               }
                           });

    m_sequenceNumber++;
    m_timestamp += length;
}

std::vector<uint8_t> RtpClient::createRTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const std::vector<float> &audioData, bool marker)
{
    std::vector<uint8_t> packet;

    // RTP Header (12 bytes)
    std::array<uint8_t, RtpClient::RTP_HEADER_SIZE> header;

    // Version = 2, no padding, no extension, no CSRC
    header[0] = (2 << 6) | (0 << 5) | (0 << 4) | (0); // version 2, padding 0, extension 0, CSRC count 0
    header[1] = (marker << 7) | (payloadType & 0x7F); // Marker (1 bit) and Payload Type (7 bits)

    // Sequence number (16 bits)
    header[2] = (sequenceNumber >> 8) & 0xFF;
    header[3] = sequenceNumber & 0xFF;

    // Timestamp (32 bits)
    header[4] = (timestamp >> 24) & 0xFF;
    header[5] = (timestamp >> 16) & 0xFF;
    header[6] = (timestamp >> 8) & 0xFF;
    header[7] = timestamp & 0xFF;

    // SSRC (32 bits)
    header[8] = (ssrc >> 24) & 0xFF;
    header[9] = (ssrc >> 16) & 0xFF;
    header[10] = (ssrc >> 8) & 0xFF;
    header[11] = ssrc & 0xFF;

    // Add the RTP header to the packet
    packet.insert(packet.end(), header.begin(), header.end());

    // Append the audio data (converted to bytes)
    const uint8_t *audioBytes = reinterpret_cast<const uint8_t *>(audioData.data());
    packet.insert(packet.end(), audioBytes, audioBytes + audioData.size() * sizeof(float));

    return packet;
}
