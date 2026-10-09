#include "lab.h"

#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>


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
    if (sender == NULL) {
        return 1;
    }

    /*
     * An ACK cannot acknowledge a sequence number beyond the
     * next sequence number that has been sent.
     */
    if (ack_seq > sender->nextseqnum) {
        return 1;
    }

    /*
     * Duplicate or stale ACKs do not advance the sender's base.
     */
    if (ack_seq <= sender->base) {
        return 0;
    }

    /*
     * Advance base to the next packet the receiver expects.
     * Because ACKs are cumulative, all packets before ack_seq
     * are now acknowledged.
     */
    sender->base = ack_seq;

    /*
     * If no DATA packets remain unacknowledged, stop the timer.
     * Otherwise, restart the timer for the oldest outstanding
     * packet.
     */
    if (sender->base == sender->nextseqnum) {
        sender->timer_running = 0;
        sender->timer_start = 0;
    } else {
        sender->timer_start = now_ms;
        sender->timer_running = 1;
    }

    /* A valid advancing ACK resets the consecutive timeout count. */
    sender->consecutive_timeouts = 0;

    return 0;
}

int senderHandleTimeout(SenderState *sender, uint64_t now_ms, Packet *packets, size_t *count)
{
    uint32_t seq;
    size_t num_packets = 0;

    /* Validate arguments. */
    if (sender == NULL || packets == NULL || count == NULL) {
        return 1;
    }

    *count = 0;

    /* A timeout can only be handled when the timer is running. */
    if (!sender->timer_running) {
        return 1;
    }

    /* Check whether the retransmission timer has expired. */
    if (now_ms < sender->timer_start ||
        now_ms - sender->timer_start < sender->timeout_ms) {
        return 1;
    }

    /* No outstanding packets means there is nothing to retransmit. */
    if (sender->base == sender->nextseqnum) {
        sender->timer_running = 0;
        sender->timer_start = 0;
        return 0;
    }

    /* Ensure the outstanding packet count is within the configured window. */
    if (sender->nextseqnum - sender->base > sender->window ||
        sender->nextseqnum - sender->base > MAX_WINDOW) {
        return 1;
    }

    /*
     * Go-Back-N: retransmit every outstanding DATA packet,
     * starting with the oldest unacknowledged packet.
     */
    for (seq = sender->base; seq < sender->nextseqnum; seq++) {
        uint32_t index = seq % MAX_WINDOW;

        packets[num_packets] = sender->packets[index];
        num_packets++;
    }

    *count = num_packets;

    /* Restart the timer for the retransmitted window. */
    sender->timer_start = now_ms;
    sender->timer_running = 1;

    /* Record the timeout for retry-limit handling. */
    sender->consecutive_timeouts++;

    return 0;
}

int senderTimerDue(const SenderState *sender, uint64_t now_ms, uint64_t *remaining_ms)
{
    uint64_t elapsed;

    /* Validate arguments. */
    if (sender == NULL || remaining_ms == NULL) {
        return 1;
    }

    *remaining_ms = 0;

    /* No active timer means there is no deadline to report. */
    if (!sender->timer_running) {
        return 1;
    }

    /* Guard against a timestamp earlier than the timer start. */
    if (now_ms < sender->timer_start) {
        *remaining_ms = sender->timeout_ms;
        return 0;
    }

    elapsed = now_ms - sender->timer_start;

    /* The timer has expired. */
    if (elapsed >= sender->timeout_ms) {
        *remaining_ms = 0;
        return 0;
    }

    /* Calculate the time remaining until expiration. */
    *remaining_ms = (uint64_t)sender->timeout_ms - elapsed;

    return 0;
}

int senderSendFin(SenderState *sender, uint64_t now_ms, Packet *packet)
{
    /* Validate arguments. */
    if (sender == NULL || packet == NULL) {
        return 1;
    }

    /* Do not send FIN while DATA packets remain unacknowledged. */
    if (sender->base != sender->nextseqnum) {
        return 1;
    }

    /* Do not send FIN more than once. */
    if (sender->fin_sent) {
        return 1;
    }

    /* Create the FIN packet using the next sequence number. */
    if (makeFinPacket(packet, sender->nextseqnum) != 0) {
        return 1;
    }

    /* Store a copy for retransmission if the FIN is lost. */
    sender->fin = *packet;

    /* Record that FIN has been sent. */
    sender->fin_sent = 1;
    sender->fin_acked = 0;

    /* Start the retransmission timer. */
    sender->timer_start = now_ms;
    sender->timer_running = 1;

    /* Reset the consecutive timeout counter for the FIN phase. */
    sender->consecutive_timeouts = 0;

    return 0;
}

int senderHandleFinAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms)
{
    (void)now_ms; /* Not needed for this implementation. */

    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 1;
    }

    /* FIN must have been sent before its ACK can be accepted. */
    if (!sender->fin_sent) {
        return 1;
    }

    /* Ignore an ACK if FIN has already been acknowledged. */
    if (sender->fin_acked) {
        return 1;
    }

    /* The ACK must acknowledge the FIN sequence number. */
    if (ack_seq != sender->fin.seq + 1U) {
        return 1;
    }

    /* Mark FIN as acknowledged. */
    sender->fin_acked = 1;

    /* Stop the retransmission timer. */
    sender->timer_running = 0;
    sender->timer_start = 0;

    return 0;
}

int senderFinished(const SenderState *sender)
{
    /* Validate the sender pointer. */
    if (sender == NULL) {
        return 0;
    }

    /* All DATA packets and the FIN packet must be acknowledged. */
    if (sender->base == sender->nextseqnum &&
        sender->fin_sent &&
        sender->fin_acked) {
        return 1;
    }

    return 0;
}

int receiverInit(ReceiverState *receiver)
{
    /* Validate the receiver pointer. */
    if (receiver == NULL) {
        return 1;
    }

    /* Initialize the receiver state. */
    receiver->expectedseqnum = 0;
    receiver->finished = 0;
    receiver->last_activity = getTimeMs(); //FIXME is this ok..?

    return 0;
}

int receiverHandleData(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    if (receiver == NULL || packet == NULL || ack == NULL) {
        return 1;
    }

    /* Only DATA packets can be handled by this function. */
    if (packet->type != PACKET_DATA) {
        return 1;
    }

    /* Reject malformed packets with an oversized payload. */
    if (packet->length > MAX_PAYLOAD) {
        return 1;
    }

    /* Do not process data after the receiver has finished. */
    if (receiver->finished) {
        return 1;
    }

    /*
     * Go-Back-N receiver behavior:
     * accept only the next expected sequence number.
     */
    if (packet->seq == receiver->expectedseqnum) {
        receiver->expectedseqnum++;
        receiver->last_activity = now_ms;
    }

    /*
     * For an expected packet, duplicate, or out-of-order packet,
     * acknowledge the next expected sequence number. This is a
     * cumulative ACK: it tells the sender which sequence number
     * the receiver is waiting for next.
     */
    if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
        return 1;
    }

    return 0;
}

int receiverHandleFin(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack)
{
    if (receiver == NULL || packet == NULL || ack == NULL) {
        return 1;
    }

    /* Only FIN packets can be handled by this function. */
    if (packet->type != PACKET_FIN) {
        return 1;
    }

    /* FIN packets must not contain a payload. */
    if (packet->length != 0) {
        return 1;
    }

    /*
     * If the receiver has already finished, acknowledge a repeated
     * FIN with the final cumulative ACK during the linger period.
     */
    if (receiver->finished) {
        if (packet->seq == receiver->expectedseqnum - 1) {
            if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
                return 1;
            }

            receiver->last_activity = now_ms;
            return 0;
        }

        return 1;
    }

    /*
     * Accept FIN only when its sequence number equals the next
     * expected DATA sequence number.
     */
    if (packet->seq != receiver->expectedseqnum) {
        /* FIN arrived too early or out of order; ACK current expectation. */
        if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
            return 1;
        }

        return 0;
    }

    /*
     * The FIN is in order. Advance the expected sequence number,
     * mark the transfer as finished, and generate the final ACK.
     */
    receiver->expectedseqnum++;
    receiver->finished = 1;
    receiver->last_activity = now_ms;

    if (makeAckPacket(ack, receiver->expectedseqnum) != 0) {
        return 1;
    }

    return 0;
}

int receiverTimerExpired(const ReceiverState *receiver, uint64_t now_ms)
{
    /* Reject an invalid receiver pointer. */
    if (receiver == NULL) {
        return 1;
    }

    /* Avoid unsigned underflow if the supplied time is earlier. */
    if (now_ms < receiver->last_activity) {
        return 0;
    }

    /* Expire after 30 seconds without activity. */
    if (now_ms - receiver->last_activity >= 30000) {
        return 1;
    }

    return 0;
}

uint64_t getTimeMs(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return ((uint64_t)ts.tv_sec * UINT64_C(1000))
           + ((uint64_t)ts.tv_nsec / UINT64_C(1000000));
}

int resolveAddr(const char *host, const char *port, struct addrinfo **results)
{
    struct addrinfo hints;
    int status;

    /* Validate arguments. */
    if (host == NULL || port == NULL || results == NULL) {
        return 1;
    }

    /* Initialize the output pointer before attempting resolution. */
    *results = NULL;

    /* Configure address resolution for UDP. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;       /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;    /* UDP */
    hints.ai_protocol = IPPROTO_UDP;

    /* Resolve the host and service (port). */
    status = getaddrinfo(host, port, &hints, results);

    if (status != 0) {
        *results = NULL;
        return 1;
    }

    return 0;
}

int createSocket(const struct addrinfo *addr)
{
    int sockfd;

    if (addr == NULL) {
        return -1;
    }

    sockfd = socket(addr->ai_family,
                    addr->ai_socktype,
                    addr->ai_protocol);

    if (sockfd == -1) {
        return -1;
    }

    return sockfd;
}

int registerReceiver(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session)
{
    char hello[64];
    char response[256];

    /* Validate arguments. */
    if (sockfd < 0 || relay == NULL || session == NULL) {
        return 1;
    }

    /* Validate the session length: 1 to 32 characters. */
    size_t session_len = strlen(session);

    if (session_len < 1 || session_len > 32) {
        return 1;
    }

    /* Validate session characters: a-z, 0-9, and hyphen. */
    for (size_t i = 0; i < session_len; i++) {
        char c = session[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-')) {
            return 1;
        }
    }

    /* Construct the registration message without a newline. */
    int hello_len = snprintf(hello, sizeof(hello),
                             "HELLO %s recv", session);

    if (hello_len < 0 || (size_t)hello_len >= sizeof(hello)) {
        return 1;
    }

    /* Set a one-second receive timeout. */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        return 1;
    }

    /* Retry registration up to five times. */
    for (int attempt = 0; attempt < 5; attempt++) {

        ssize_t sent = sendto(sockfd,
                              hello,
                              (size_t)hello_len,
                              0,
                              relay,
                              relay_len);

        if (sent < 0 || sent != hello_len) {
            continue;
        }

        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sockfd,
                                    response,
                                    sizeof(response) - 1,
                                    0,
                                    (struct sockaddr *)&sender_addr,
                                    &sender_len);

        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK ||
                errno == EINTR) {
                continue;
            }

            return 1;
        }

        response[received] = '\0';

        /* Ignore datagrams that did not come from the relay. */
        if (sender_len != relay_len ||
            sender_addr.ss_family != relay->sa_family) {
            continue;
        }

        int same_address = 0;

        if (relay->sa_family == AF_INET) {
            const struct sockaddr_in *expected =
                (const struct sockaddr_in *)relay;
            const struct sockaddr_in *actual =
                (const struct sockaddr_in *)&sender_addr;

            same_address =
                expected->sin_port == actual->sin_port &&
                expected->sin_addr.s_addr == actual->sin_addr.s_addr;
        } else if (relay->sa_family == AF_INET6) {
            const struct sockaddr_in6 *expected =
                (const struct sockaddr_in6 *)relay;
            const struct sockaddr_in6 *actual =
                (const struct sockaddr_in6 *)&sender_addr;

            same_address =
                expected->sin6_port == actual->sin6_port &&
                memcmp(&expected->sin6_addr, &actual->sin6_addr,
                       sizeof(expected->sin6_addr)) == 0 &&
                expected->sin6_scope_id == actual->sin6_scope_id;
        }

        if (!same_address) {
            continue;
        }

        /* Registration succeeded. */
        if (strcmp(response, "OK") == 0) {
            return 0;
        }

        /* The relay explicitly rejected registration. */
        if (strncmp(response, "ERR ", 4) == 0) {
            fprintf(stderr, "Receiver registration failed: %s\n",
                    response + 4);
            return 1;
        }

        /* Unexpected response: retry registration. */
    }

    fprintf(stderr,
            "Receiver registration failed: no valid OK response "
            "after 5 attempts\n");

    return 1;
}

int registerSender(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session, double loss, double corrupt, double dup)
{
    char hello[128];
    char response[256];
    size_t session_len;
    int hello_len;

    /* Validate arguments. */
    if (sockfd < 0 || relay == NULL || session == NULL) {
        return 1;
    }

    /* Validate session length. */
    session_len = strlen(session);

    if (session_len < 1 || session_len > 32) {
        return 1;
    }

    /* Validate session characters. */
    for (size_t i = 0; i < session_len; i++) {
        char c = session[i];

        if (!((c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-')) {
            return 1;
        }
    }

    /* Validate probabilities: each must be between 0 and 0.5. */
    if (loss < 0.0 || loss > 0.5 ||
        corrupt < 0.0 || corrupt > 0.5 ||
        dup < 0.0 || dup > 0.5) {
        return 1;
    }

    /* Construct the HELLO message without a newline. */
    hello_len = snprintf(hello, sizeof(hello),
                         "HELLO %s send %.6g %.6g %.6g",
                         session, loss, corrupt, dup);

    if (hello_len < 0 || (size_t)hello_len >= sizeof(hello)) {
        return 1;
    }

    /* Set a one-second receive timeout. */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return 1;
    }

    /* Attempt registration up to five times. */
    for (int attempt = 0; attempt < 5; attempt++) {
        ssize_t sent = sendto(sockfd,
                              hello,
                              (size_t)hello_len,
                              0,
                              relay,
                              relay_len);

        if (sent < 0 || sent != hello_len) {
            continue;
        }

        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        ssize_t received = recvfrom(sockfd,
                                    response,
                                    sizeof(response) - 1,
                                    0,
                                    (struct sockaddr *)&sender_addr,
                                    &sender_len);

        if (received < 0) {
            if (errno == EINTR ||
                errno == EAGAIN ||
                errno == EWOULDBLOCK) {
                continue;
            }

            perror("recvfrom");
            return 1;
        }

        response[received] = '\0';

        /* Verify that the response came from the relay. */
        if (sender_len != relay_len ||
            sender_addr.ss_family != relay->sa_family) {
            continue;
        }

        int same_address = 0;

        if (relay->sa_family == AF_INET) {
            const struct sockaddr_in *expected =
                (const struct sockaddr_in *)relay;
            const struct sockaddr_in *actual =
                (const struct sockaddr_in *)&sender_addr;

            same_address =
                expected->sin_port == actual->sin_port &&
                expected->sin_addr.s_addr == actual->sin_addr.s_addr;
        } else if (relay->sa_family == AF_INET6) {
            const struct sockaddr_in6 *expected =
                (const struct sockaddr_in6 *)relay;
            const struct sockaddr_in6 *actual =
                (const struct sockaddr_in6 *)&sender_addr;

            same_address =
                expected->sin6_port == actual->sin6_port &&
                expected->sin6_scope_id == actual->sin6_scope_id &&
                memcmp(&expected->sin6_addr,
                       &actual->sin6_addr,
                       sizeof(expected->sin6_addr)) == 0;
        }

        if (!same_address) {
            continue;
        }

        /* Relay accepted registration. */
        if (strcmp(response, "OK") == 0) {
            return 0;
        }

        /* Relay explicitly rejected registration. */
        if (strncmp(response, "ERR ", 4) == 0) {
            fprintf(stderr, "Sender registration failed: %s\n",
                    response + 4);
            return 1;
        }

        /* Unexpected response: retry. */
    }

    fprintf(stderr,
            "Sender registration failed after 5 attempts\n");

    return 1;
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

