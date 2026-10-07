#include "lab.h"

#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


uint16_t packetChecksum(const uint8_t *data, size_t length)
{
    return 0;
}

int encodePacket(const Packet *packet, uint8_t *buffer, size_t buffer_size, size_t *packet_size)
{
    return 0;
}

int decodePacket(const uint8_t *buffer, size_t buffer_size, Packet *packet)
{
    return 0;
}

int makeDataPacket(Packet *packet, uint32_t seq, const uint8_t *data, uint16_t length)
{
    return 0;
}

int makeAckPacket(Packet *packet, uint32_t seq)
{
    return 0;
}

int makeFinPacket(Packet *packet, uint32_t seq)
{
    return 0;
}

int senderInit(SenderState *sender, uint32_t window, uint32_t timeout_ms)
{
    return 0;
}

int senderSendData(SenderState *sender, const uint8_t *data, uint16_t length, uint64_t now_ms, Packet *packet)
{
    return 0;
}

int senderHandleAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms)
{
    return 0;
}

int senderHandleTimeout(SenderState *sender, uint64_t now_ms, Packet *packets, size_t *count)
{
    return 0;
}

int senderTimerDue(const SenderState *sender, uint64_t now_ms, uint64_t *remaining_ms)
{
    return 0;
}

int senderSendFin(SenderState *sender, uint64_t now_ms, Packet *packet)
{
    return 0;
}

int senderHandleFinAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms)
{
    return 0;
}

int senderFinished(const SenderState *sender)
{
    return 0;
}

int receiverInit(ReceiverState *receiver)
{
    return 0;
}

int receiverHandleData(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    return 0;
}

int receiverHandleFin(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    return 0;
}

int receiverTimerExpired(const ReceiverState *receiver, uint64_t now_ms)
{
    return 0;
}

uint64_t getTimeMs(void)
{
    return 0;
}

int resolveAddr(const char *host, const char *port, struct addrinfo **results)
{
    return 0;
}

int createSocket(const struct addrinfo *addr)
{
    return 0;
}

int registerReceiver(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session)
{
    return 0;
}

int registerSender(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session, double loss, double corrupt, double dup)
{
    return 0;
}

int runSender(const char *relay, const char *port, const char *session, const char *filename, uint32_t window, uint32_t timeout_ms, double loss, double corrupt, double dup)
{
    return 0;
}

int runReceiver(const char *relay, const char *port, const char *session, const char *filename)
{
    return 0;
}

void printUsage(void)
{
    printf("Usage: myapp send -s <session> [-w window] [-T timeout-ms] [-l loss]\n"
           "                  [-c corrupt] [-d dup] [-p port] <relay> <file>\n"
           "       myapp recv -s <session> [-p port] <relay> <file>\n"
           "\n"
           "  -s <session>     session name shared by the sender and the receiver\n"
           "  -w <window>      Go-Back-N window size in packets, 1 to 64 (default: 8)\n"
           "  -T <timeout-ms>  retransmission timeout in milliseconds (default: 250)\n"
           "  -l <loss>        probability the relay drops a packet (default: 0)\n"
           "  -c <corrupt>     probability the relay flips a bit (default: 0)\n"
           "  -d <dup>         probability the relay duplicates a packet (default: 0)\n"
           "  -p <port>        relay port (default: 4250)\n"
           "  <relay>          host name or address of the relay\n"
           "  <file>           file to send, or file to write what is received\n");
}

