#include "lab.h"

#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


uint16_t packetChecksum(const uint8_t *data, size_t length){

    uint32_t sum = 0;

    while (length > 1) {
        /* Combine two bytes into one 16-bit word.*/
        sum += ((uint16_t)data[0] << 8) | data[1];

        data += 2;
        length -= 2;
    }

    if (length > 0) {
        sum += (uint16_t)data[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xffffU) + (sum >> 16);
    }

    return (uint16_t)~sum;
}


int encodePacket(const Packet *packet, uint8_t *buffer, size_t buffer_size, size_t *packet_size)
{
    uint16_t network_length;
    uint32_t network_seq;
    size_t total_size;

    /* Validate pointers. */
    if (packet == NULL || buffer == NULL || packet_size == NULL) {
        return 1;
    }

    /* Validate packet type and payload length. */
    if (packet->type != PACKET_DATA &&
        packet->type != PACKET_ACK &&
        packet->type != PACKET_FIN) {
        return 1;
    }

    if (packet->length > MAX_PAYLOAD) {
        return 1;
    }

    /* ACK and FIN packets must not contain payloads. */
    if ((packet->type == PACKET_ACK || packet->type == PACKET_FIN) &&
        packet->length != 0) {
        return 1;
    }

    /* Calculate the encoded packet size. */
    total_size = HEADER_SIZE + packet->length;

    if (buffer_size < total_size) {
        return 1;
    }

    /* Initialize the header. */
    buffer[0] = packet->type;
    buffer[1] = 0; /* Reserved byte. */
    buffer[2] = 0; /* Checksum high byte. */
    buffer[3] = 0; /* Checksum low byte. */

    /* Encode the sequence number in network byte order. */
    network_seq = htonl(packet->seq);
    memcpy(&buffer[4], &network_seq, sizeof(network_seq));

    /* Encode the payload length in network byte order. */
    network_length = htons(packet->length);
    memcpy(&buffer[8], &network_length, sizeof(network_length));

    /* Copy the payload, if present. */
    if (packet->length > 0) {
        memcpy(&buffer[HEADER_SIZE], packet->payload, packet->length);
    }

    /* Calculate the checksum with bytes 2 and 3 set to zero. */
    uint16_t checksum = packetChecksum(buffer, total_size);

    /* Store the checksum in network byte order. */
    uint16_t network_checksum = htons(checksum);
    memcpy(&buffer[2], &network_checksum, sizeof(network_checksum));

    /* Report the encoded packet size. */
    *packet_size = total_size;

    return 0;
}

int decodePacket(const uint8_t *buffer, size_t buffer_size, Packet *packet)
{
    uint16_t network_checksum;
    uint16_t received_checksum;
    uint16_t network_length;
    uint16_t length;
    uint32_t network_seq;
    uint32_t seq;
    uint8_t type;
    uint8_t reserved;

    /* Validate pointers and minimum header size. */
    if (buffer == NULL || packet == NULL) {
        return 1;
    }

    if (buffer_size < HEADER_SIZE || buffer_size > MAX_PACKET_SIZE) {
        return 1;
    }

    /* Read the single-byte header fields. */
    type = buffer[0];
    reserved = buffer[1];

    /* Validate packet type and reserved byte. */
    if (type != PACKET_DATA &&
        type != PACKET_ACK &&
        type != PACKET_FIN) {
        return 1;
    }

    if (reserved != 0) {
        return 1;
    }

    /* Read the checksum and convert from network byte order. */
    memcpy(&network_checksum, &buffer[2], sizeof(network_checksum));
    received_checksum = ntohs(network_checksum);

    /* Read and convert the sequence number. */
    memcpy(&network_seq, &buffer[4], sizeof(network_seq));
    seq = ntohl(network_seq);

    /* Read and convert the payload length. */
    memcpy(&network_length, &buffer[8], sizeof(network_length));
    length = ntohs(network_length);

    /* Reject payloads that exceed the protocol maximum. */
    if (length > MAX_PAYLOAD) {
        return 1;
    }

    /* Ensure the datagram size matches its declared payload length. */
    if (buffer_size != HEADER_SIZE + (size_t)length) {
        return 1;
    }

    /* ACK and FIN packets must have empty payloads. */
    if ((type == PACKET_ACK || type == PACKET_FIN) && length != 0) {
        return 1;
    }

    /*
     * Verify the checksum without modifying the received buffer.
     * The checksum field must be zero during calculation.
     */
    uint8_t checksum_buffer[MAX_PACKET_SIZE];

    memcpy(checksum_buffer, buffer, buffer_size);
    checksum_buffer[2] = 0;
    checksum_buffer[3] = 0;

    uint16_t calculated_checksum =
        packetChecksum(checksum_buffer, buffer_size);

    if (calculated_checksum != received_checksum) {
        return 1;
    }

    /*
     * All validation has succeeded.
     * Only now copy the decoded fields into the output packet.
     */
    packet->type = type;
    packet->seq = seq;
    packet->length = length;

    if (length > 0) {
        memcpy(packet->payload, &buffer[HEADER_SIZE], length);
    }

    return 0;
}

int makeDataPacket(Packet *packet, uint32_t seq, const uint8_t *data, uint16_t length)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Validate the payload length. */
    if (length > MAX_PAYLOAD) {
        return 1;
    }

    /* A nonempty payload requires a valid data pointer. */
    if (length > 0 && data == NULL) {
        return 1;
    }

    /* Initialize the packet header fields. */
    packet->type = PACKET_DATA;
    packet->seq = seq;
    packet->length = length;

    /* Copy the payload when there is data to copy. */
    if (length > 0) {
        memcpy(packet->payload, data, length);
    }

    return 0;
}

int makeAckPacket(Packet *packet, uint32_t seq)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Initialize the ACK packet header fields. */
    packet->type = PACKET_ACK;
    packet->seq = seq;
    packet->length = 0;

    return 0;
}

int makeFinPacket(Packet *packet, uint32_t seq)
{
    /* Validate the packet pointer. */
    if (packet == NULL) {
        return 1;
    }

    /* Initialize the FIN packet header fields. */
    packet->type = PACKET_FIN;
    packet->seq = seq;
    packet->length = 0;

    return 0;
}

int senderInit(SenderState *sender, uint32_t window, uint32_t timeout_ms)
{
    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 1;
    }

    /* Validate the window size. */
    if (window == 0 || window > MAX_WINDOW) {
        return 1;
    }

    /* Validate the retransmission timeout. */
    if (timeout_ms == 0) {
        return 1;
    }

    /* Initialize the sender state. */
    sender->base = 0;
    sender->nextseqnum = 0;
    sender->window = window;
    sender->timeout_ms = timeout_ms;
    sender->consecutive_timeouts = 0;
    sender->timer_start = 0;
    sender->timer_running = 0;

    /* Clear stored packets used for retransmission. */
    memset(sender->packets, 0, sizeof(sender->packets));

    /* Initialize FIN state. */
    memset(&sender->fin, 0, sizeof(sender->fin));
    sender->fin_sent = 0;
    sender->fin_acked = 0;

    return 0;
}

int senderSendData(SenderState *sender, const uint8_t *data, uint16_t length, uint64_t now_ms, Packet *packet)
{
    uint32_t window_index;

    /* Validate arguments. */
    if (sender == NULL || packet == NULL ||
        data == NULL || length == 0 || length > MAX_PAYLOAD) {
        return 1;
    }

    /* Reject sending if the window is full. */
    if (sender->nextseqnum - sender->base >= sender->window) {
        return 1;
    }

    /* Do not send data after the FIN packet has been sent. */
    if (sender->fin_sent || sender->fin_acked) {
        return 1;
    }

    /*
     * Use a circular buffer to store packets in the retransmission
     * window. This allows the sender to support windows up to MAX_WINDOW.
     */
    window_index = sender->nextseqnum % MAX_WINDOW;

    /* Create the next DATA packet. */
    if (makeDataPacket(packet, sender->nextseqnum, data, length) != 0) {
        return 1;
    }

    /* Keep a copy so the packet can be retransmitted after a timeout. */
    sender->packets[window_index] = *packet;

    /* Advance the next sequence number. */
    sender->nextseqnum++;

    /*
     * Start the timer only when the first unacknowledged packet
     * enters an otherwise empty window.
     */
    if (sender->base == sender->nextseqnum - 1) {
        sender->timer_start = now_ms;
        sender->timer_running = 1;
    }

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

