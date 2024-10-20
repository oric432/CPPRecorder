#include "RtpClient.h"

RtpClient::RtpClient(boost::asio::io_context &io_ctx, const std::string &host, const std::string &port)
    : m_socket(io_ctx, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), m_timestamp(0), m_sequenceNumber(0), m_ssrc(generateSSRC())
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

void RtpClient::sendAudioData(const float *audioBuffer, size_t length)
{
    std::cout << "hello " << m_endpoint << '\n';

    // Allocate buffer for the RTP packet
    size_t packetBufferSize = RtpClient::RTP_HEADER_SIZE + length * sizeof(float);
    uint8_t packetBuffer[packetBufferSize];

    createRTPPacket(0, m_sequenceNumber, m_timestamp, m_ssrc, audioBuffer, length, packetBuffer, packetBufferSize);

    m_socket.async_send_to(boost::asio::buffer(packetBuffer, packetBufferSize), m_endpoint,
                           [](const boost::system::error_code &ec, std::size_t bytes_sent)
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

void RtpClient::createRTPPacket(uint8_t payloadType, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc, const float *audioData, size_t length, uint8_t *packetBuffer, size_t packetBufferSize)
{
    // Ensure that the buffer is large enough for the RTP header + audio data
    size_t headerSize = RtpClient::RTP_HEADER_SIZE;
    size_t audioSize = length * sizeof(float);

    if (packetBufferSize < headerSize + audioSize)
    {
        throw std::runtime_error("Buffer size is too small for RTP packet");
    }

    // RTP Header (12 bytes)
    // Version = 2, no padding, no extension, no CSRC
    packetBuffer[0] = (2 << 6) | (0 << 5) | (0 << 4) | (0); // version 2, padding 0, extension 0, CSRC count 0
    packetBuffer[1] = (false << 7) | (payloadType & 0x7F);  // Marker (1 bit) and Payload Type (7 bits)

    // Sequence number (16 bits)
    packetBuffer[2] = (sequenceNumber >> 8) & 0xFF;
    packetBuffer[3] = sequenceNumber & 0xFF;

    // Timestamp (32 bits)
    packetBuffer[4] = (timestamp >> 24) & 0xFF;
    packetBuffer[5] = (timestamp >> 16) & 0xFF;
    packetBuffer[6] = (timestamp >> 8) & 0xFF;
    packetBuffer[7] = timestamp & 0xFF;

    // SSRC (32 bits)
    packetBuffer[8] = (ssrc >> 24) & 0xFF;
    packetBuffer[9] = (ssrc >> 16) & 0xFF;
    packetBuffer[10] = (ssrc >> 8) & 0xFF;
    packetBuffer[11] = ssrc & 0xFF;

    // Copy audio data into the packet buffer
    std::memcpy(packetBuffer + headerSize, audioData, audioSize);
}