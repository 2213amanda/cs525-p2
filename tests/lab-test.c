#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <poll.h>
#include <stdint.h>

#define TEST_PORT_SIZE 16
#define TEST_TIMEOUT_MS 3000

static int test_relay_fd = -1;
static pid_t test_receiver_pid = -1;
static struct sockaddr_in test_receiver_addr;
static char test_output_path[128];

static int relay_fd;
static struct sockaddr_in relay_addr;

/* Test relay state for runSender */
static pid_t relay_pid = -1;
static char relay_port[16];


/*
 * Stop the child process and release test resources.
 */
static void testStopReceiver(void)
{
    if (test_receiver_pid > 0) {
        int status;
        pid_t result;

        /*
         * Give the receiver time to finish its FIN linger period.
         * Kill it if a test failed before completing the transfer.
         */
        result = waitpid(test_receiver_pid, &status, WNOHANG);

        if (result == 0) {
            kill(test_receiver_pid, SIGTERM);
            (void)waitpid(test_receiver_pid, &status, 0);
        }

        test_receiver_pid = -1;
    }

    if (test_relay_fd >= 0) {
        close(test_relay_fd);
        test_relay_fd = -1;
    }

    if (test_output_path[0] != '\0') {
        unlink(test_output_path);
        test_output_path[0] = '\0';
    }
}

void setUp(void)
{
    relay_fd = -1;
    relay_pid = -1;
    memset(&relay_addr, 0, sizeof(relay_addr));

    relay_port[0] = '\0';

    test_relay_fd = -1;
    test_receiver_pid = -1;
    test_output_path[0] = '\0';
    memset(&test_receiver_addr, 0, sizeof(test_receiver_addr));
}

void tearDown(void)
{
    /* Close the relay socket if it is open. */
    if (relay_fd >= 0) {
        close(relay_fd);
        relay_fd = -1;
    }

    /* Stop and reap the relay child process. */
    if (relay_pid > 0) {
        int status;

        /* Ask the child to terminate. */
        if (kill(relay_pid, SIGTERM) < 0 && errno != ESRCH) {
            perror("kill");
        }

        /* Reap the child, retrying if interrupted. */
        while (waitpid(relay_pid, &status, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno != ECHILD) {
                perror("waitpid");
            }

            break;
        }

        relay_pid = -1;
    }

    testStopReceiver();
}

//packetChecksum

void test_packetChecksum_empty(void)
{
    uint8_t data[] = {};

    TEST_ASSERT_EQUAL_HEX16(0xffff, packetChecksum(data, 0));
}


void test_packetChecksum_singleByte(void)
{
    uint8_t data[] = {0x12};

    /* Calculation: 0x1200
     * Complement: 0xedff
     */
    TEST_ASSERT_EQUAL_HEX16(0xedff, packetChecksum(data, 1));
}


void test_packetChecksum_twoBytes(void)
{
    uint8_t data[] = {0x12, 0x34};

    /* Calculation: 0x1234
     * Complement: 0xedcb
     */
    TEST_ASSERT_EQUAL_HEX16(0xedcb, packetChecksum(data, 2));
}


void test_packetChecksum_fourBytes(void)
{
    uint8_t data[] = {
        0x12, 0x34,
        0x56, 0x78
    };

    /* 0x1234 + 0x5678 = 0x68ac
     * Complement = 0x9753
     */
    TEST_ASSERT_EQUAL_HEX16(0x9753, packetChecksum(data, 4));
}


void test_packetChecksum_oddLength(void)
{
    uint8_t data[] = {
        0x12, 0x34, 0x56
    };

    /* Calculation:
     *   0x1234 + 0x5600 = 0x6834
     *
     * Complement:
     *   0x97cb
     */
    TEST_ASSERT_EQUAL_HEX16(0x97cb, packetChecksum(data, 3));
}

// makeDataPacket

void test_makeDataPacket_validPacket(void)
{
    Packet packet;
    uint8_t data[] = {0x10, 0x20, 0x30, 0x40};
    uint32_t seq = 5;
    uint16_t length = sizeof(data);

    int result = makeDataPacket(&packet, seq, data, length);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(seq, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(length, packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, length);
}

void test_makeDataPacket_emptyPayload(void)
{
    Packet packet;

    int result = makeDataPacket(&packet, 0, NULL, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeDataPacket_maxPayload(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD];

    memset(data, 0xAB, sizeof(data));

    int result = makeDataPacket(&packet, 10, data, MAX_PAYLOAD);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(10, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(MAX_PAYLOAD, packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, MAX_PAYLOAD);
}

void test_makeDataPacket_nullPacket(void)
{
    uint8_t data[] = {0x01, 0x02};

    int result = makeDataPacket(NULL, 0, data, sizeof(data));

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_nullDataWithNonzeroLength(void)
{
    Packet packet;

    int result = makeDataPacket(&packet, 0, NULL, 1);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_payloadTooLarge(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD + 1];

    int result = makeDataPacket(&packet, 0, data,
                                MAX_PAYLOAD + 1);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_makeDataPacket_zeroLengthWithNonNullData(void)
{
    Packet packet;
    uint8_t data[] = {0x55};

    int result = makeDataPacket(&packet, 7, data, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(7, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}


//test makeAckPacket

void test_makeAckPacket_validPacket(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, 5);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(5, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_zeroSequenceNumber(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_maxSequenceNumber(void)
{
    Packet packet;

    int result = makeAckPacket(&packet, UINT32_MAX);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, packet.type);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeAckPacket_nullPacket(void)
{
    int result = makeAckPacket(NULL, 5);

    TEST_ASSERT_NOT_EQUAL(0, result);
}


//test makeFinPacket

void test_makeFinPacket_validPacket(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, 10);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(10, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_zeroSequenceNumber(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_maxSequenceNumber(void)
{
    Packet packet;

    int result = makeFinPacket(&packet, UINT32_MAX);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
}

void test_makeFinPacket_nullPacket(void)
{
    int result = makeFinPacket(NULL, 10);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

// test encodePacket

void test_encodePacket_dataPacket(void)
{
    Packet packet;
    uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 5, data, sizeof(data)));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE + sizeof(data), packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(5, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), ntohs(length));

    TEST_ASSERT_EQUAL_MEMORY(data, &buffer[HEADER_SIZE], sizeof(data));
}

void test_encodePacket_ackPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 5));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(5, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(0, ntohs(length));
}

void test_encodePacket_finPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 10));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);

    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0, buffer[1]);

    uint32_t seq;
    uint16_t length;

    memcpy(&seq, &buffer[4], sizeof(seq));
    memcpy(&length, &buffer[8], sizeof(length));

    TEST_ASSERT_EQUAL_UINT32(10, ntohl(seq));
    TEST_ASSERT_EQUAL_UINT16(0, ntohs(length));
}

void test_encodePacket_emptyDataPacket(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeDataPacket(&packet, 0, NULL, 0));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(HEADER_SIZE, packet_size);
    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, buffer[0]);
}

void test_encodePacket_maxPayload(void)
{
    Packet packet;
    uint8_t data[MAX_PAYLOAD];
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 7, data, MAX_PAYLOAD));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT16(MAX_PACKET_SIZE, packet_size);
    TEST_ASSERT_EQUAL_MEMORY(data, &buffer[HEADER_SIZE], MAX_PAYLOAD);
}

void test_encodePacket_nullPacket(void)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    int result = encodePacket(
        NULL, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_nullBuffer(void)
{
    Packet packet;
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    int result = encodePacket(
        &packet, NULL, MAX_PACKET_SIZE, &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_nullPacketSize(void)
{
    Packet packet;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), NULL);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_bufferTooSmall(void)
{
    Packet packet;
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t buffer[HEADER_SIZE + sizeof(data) - 1];
    size_t packet_size = 0;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_invalidType(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = 99;
    packet.seq = 0;
    packet.length = 0;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_ackWithPayload(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_ACK;
    packet.seq = 1;
    packet.length = 1;
    packet.payload[0] = 0xAA;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_finWithPayload(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_FIN;
    packet.seq = 1;
    packet.length = 1;
    packet.payload[0] = 0xAA;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_encodePacket_payloadTooLarge(void)
{
    Packet packet = {0};
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size = 0;

    packet.type = PACKET_DATA;
    packet.seq = 0;
    packet.length = MAX_PAYLOAD + 1;

    int result = encodePacket(
        &packet, buffer, sizeof(buffer), &packet_size);

    TEST_ASSERT_NOT_EQUAL(0, result);
}


//test decodePacket

/* Helper: encode a packet into a valid wire-format buffer. */
static size_t makeEncodedTestPacket(
    Packet *source, uint8_t *buffer, size_t buffer_size)
{
    size_t packet_size = 0;

    int result = encodePacket(
        source, buffer, buffer_size, &packet_size);

    if (result != 0) {
        return 0;
    }

    return packet_size;
}

void test_decodePacket_validDataPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 5, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE + sizeof(data), size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(5, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), decoded.length);
    TEST_ASSERT_EQUAL_MEMORY(data, decoded.payload, sizeof(data));
}

void test_decodePacket_validAckPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 10));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(10, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_validFinPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&original, 12));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_FIN, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(12, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_emptyDataPacket(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 0, NULL, 0));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(HEADER_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(0, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(0, decoded.length);
}

void test_decodePacket_maxPayload(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[MAX_PAYLOAD];
    uint8_t buffer[MAX_PACKET_SIZE];

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 7, data, MAX_PAYLOAD));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_UINT32(MAX_PACKET_SIZE, size);
    TEST_ASSERT_EQUAL_INT(0, decodePacket(buffer, size, &decoded));

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, decoded.type);
    TEST_ASSERT_EQUAL_UINT32(7, decoded.seq);
    TEST_ASSERT_EQUAL_UINT16(MAX_PAYLOAD, decoded.length);
    TEST_ASSERT_EQUAL_MEMORY(data, decoded.payload, MAX_PAYLOAD);
}

void test_decodePacket_nullBuffer(void)
{
    Packet decoded;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(NULL, HEADER_SIZE, &decoded));
}

void test_decodePacket_nullPacket(void)
{
    Packet original;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, NULL));
}

void test_decodePacket_tooShort(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE] = {0};

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, HEADER_SIZE - 1, &decoded));
}

void test_decodePacket_tooLarge(void)
{
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE + 1] = {0};

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

void test_decodePacket_invalidType(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    buffer[0] = 99;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_nonzeroReservedByte(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 1));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    buffer[1] = 1;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_lengthMismatch(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 1, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Datagram is truncated by one byte. */
    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size - 1, &decoded));
}

void test_decodePacket_corruptedPayload(void)
{
    Packet original;
    Packet decoded;
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&original, 2, data, sizeof(data)));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Change payload without updating the checksum. */
    buffer[HEADER_SIZE] ^= 0x01;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_corruptedSequenceNumber(void)
{
    Packet original;
    Packet decoded;
    uint8_t buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&original, 5));

    size_t size = makeEncodedTestPacket(
        &original, buffer, sizeof(buffer));

    /* Change sequence number without updating checksum. */
    buffer[7] ^= 0x01;

    TEST_ASSERT_NOT_EQUAL(0, decodePacket(buffer, size, &decoded));
}

void test_decodePacket_ackWithPayload(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE + 1] = {0};

    buffer[0] = PACKET_ACK;
    buffer[8] = 0;
    buffer[9] = 1;
    buffer[HEADER_SIZE] = 0xAA;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

void test_decodePacket_finWithPayload(void)
{
    Packet decoded;
    uint8_t buffer[HEADER_SIZE + 1] = {0};

    buffer[0] = PACKET_FIN;
    buffer[8] = 0;
    buffer[9] = 1;
    buffer[HEADER_SIZE] = 0xAA;

    TEST_ASSERT_NOT_EQUAL(
        0, decodePacket(buffer, sizeof(buffer), &decoded));
}

// test senderInit

void test_senderInit_validArguments(void)
{
    SenderState sender;

    int result = senderInit(&sender, 8, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(8, sender.window);
    TEST_ASSERT_EQUAL_UINT32(250, sender.timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
}

void test_senderInit_minimumWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, 1, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(1, sender.window);
}

void test_senderInit_maximumWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, MAX_WINDOW, 250);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(MAX_WINDOW, sender.window);
}

void test_senderInit_nullPointer(void)
{
    int result = senderInit(NULL, 8, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_zeroWindow(void)
{
    SenderState sender;

    int result = senderInit(&sender, 0, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_windowTooLarge(void)
{
    SenderState sender;

    int result = senderInit(&sender, MAX_WINDOW + 1, 250);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_zeroTimeout(void)
{
    SenderState sender;

    int result = senderInit(&sender, 8, 0);

    TEST_ASSERT_NOT_EQUAL_INT(0, result);
}

void test_senderInit_resetsState(void)
{
    SenderState sender;

    /* Fill the state with nonzero values first. */
    memset(&sender, 0xFF, sizeof(sender));

    int result = senderInit(&sender, 4, 500);

    TEST_ASSERT_EQUAL_INT(0, result);

    TEST_ASSERT_EQUAL_UINT32(0, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(4, sender.window);
    TEST_ASSERT_EQUAL_UINT32(500, sender.timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);

    /* Verify the stored packet slots were cleared. */
    for (size_t i = 0; i < MAX_WINDOW; i++) {
        TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, sender.packets[i].type);
        TEST_ASSERT_EQUAL_UINT32(0, sender.packets[i].seq);
        TEST_ASSERT_EQUAL_UINT16(0, sender.packets[i].length);
    }

    TEST_ASSERT_EQUAL_UINT8(PACKET_DATA, sender.fin.type);
    TEST_ASSERT_EQUAL_UINT32(0, sender.fin.seq);
    TEST_ASSERT_EQUAL_UINT16(0, sender.fin.length);
}


// test senderSendData

/* Successfully sends the first DATA packet. */
void test_senderSendData_firstPacket(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A', 'B', 'C' };
    uint64_t now_ms = 1000;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), now_ms, &packet));

    TEST_ASSERT_EQUAL_INT(PACKET_DATA, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(sizeof(data), packet.length);
    TEST_ASSERT_EQUAL_MEMORY(data, packet.payload, sizeof(data));

    TEST_ASSERT_EQUAL_UINT32(1, sender.nextseqnum);
    TEST_ASSERT_EQUAL_UINT32(0, sender.base);

    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(now_ms, sender.timer_start);
}

/* Stores a copy of the packet for possible retransmission. */
void test_senderSendData_storesPacket(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 10, 20, 30, 40 };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 500, &packet));

    uint32_t index = packet.seq % MAX_WINDOW;

    TEST_ASSERT_EQUAL_INT(PACKET_DATA, sender.packets[index].type);
    TEST_ASSERT_EQUAL_UINT32(packet.seq, sender.packets[index].seq);
    TEST_ASSERT_EQUAL_UINT16(packet.length, sender.packets[index].length);
    TEST_ASSERT_EQUAL_MEMORY(
        packet.payload, sender.packets[index].payload, packet.length);
}

/* Advances the sequence number for each successfully created packet. */
void test_senderSendData_multiplePackets(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'X', 'Y' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_UINT32(1, packet.seq);

    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);

    /* Sending the second packet does not restart the timer. */
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Rejects a null sender. */
void test_senderSendData_nullSender(void)
{
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(NULL, data, sizeof(data), 100, &packet));
}

/* Rejects a null payload pointer. */
void test_senderSendData_nullData(void)
{
    SenderState sender;
    Packet packet;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, NULL, 1, 100, &packet));
}

/* Rejects a null output packet pointer. */
void test_senderSendData_nullPacket(void)
{
    SenderState sender;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, NULL));
}

/* Rejects an empty DATA payload. */
void test_senderSendData_zeroLength(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, 0, 100, &packet));
}

/* Rejects payloads larger than MAX_PAYLOAD. */
void test_senderSendData_payloadTooLarge(void)
{
    SenderState sender;
    Packet packet;
    uint8_t data[MAX_PAYLOAD + 1] = { 0 };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(
            &sender, data, MAX_PAYLOAD + 1, 100, &packet));
}

/* Rejects sending when the window is full. */
void test_senderSendData_fullWindow(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 2, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    /* A failed send must not advance the sequence number. */
    TEST_ASSERT_EQUAL_UINT32(2, sender.nextseqnum);
}

/* Rejects sending after FIN has been sent. */
void test_senderSendData_afterFinSent(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));
    sender.fin_sent = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
}

/* Rejects sending after FIN has been acknowledged. */
void test_senderSendData_afterFinAcked(void)
{
    SenderState sender;
    Packet packet;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));
    sender.fin_acked = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_UINT32(0, sender.nextseqnum);
}

//test senderHandleAck

/* Advances base when a valid cumulative ACK is received. */
void test_senderHandleAck_advancesBase(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 6;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 2;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 4, 200));

    TEST_ASSERT_EQUAL_UINT32(4, sender.base);
    TEST_ASSERT_EQUAL_UINT32(6, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(200, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Ignores a duplicate ACK without changing the sender state. */
void test_senderHandleAck_duplicateAck(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 1;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 2, 200));

    TEST_ASSERT_EQUAL_UINT32(2, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(1, sender.consecutive_timeouts);
}

/* Ignores an ACK older than base. */
void test_senderHandleAck_staleAck(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 3;
    sender.nextseqnum = 6;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 1, 200));

    TEST_ASSERT_EQUAL_UINT32(3, sender.base);
    TEST_ASSERT_EQUAL_UINT32(6, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Rejects an ACK beyond the next sequence number sent. */
void test_senderHandleAck_ackBeyondNext(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_NOT_EQUAL(0, senderHandleAck(&sender, 6, 200));

    TEST_ASSERT_EQUAL_UINT32(2, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
}

/* Stops the timer when all outstanding DATA packets are acknowledged. */
void test_senderHandleAck_allDataAcknowledged(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.timer_running = 1;
    sender.timer_start = 100;
    sender.consecutive_timeouts = 3;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 5, 200));

    TEST_ASSERT_EQUAL_UINT32(5, sender.base);
    TEST_ASSERT_EQUAL_UINT32(5, sender.nextseqnum);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Handles an ACK when no DATA packets are outstanding. */
void test_senderHandleAck_noOutstandingData(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.timer_running = 0;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 3, 200));

    TEST_ASSERT_EQUAL_UINT32(3, sender.base);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

/* Rejects a null sender pointer. */
void test_senderHandleAck_nullSender(void)
{
    TEST_ASSERT_NOT_EQUAL(0, senderHandleAck(NULL, 1, 200));
}

/* An advancing ACK resets the consecutive timeout count. */
void test_senderHandleAck_resetsTimeoutCount(void)
{
    SenderState sender;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 8, 250));

    sender.base = 0;
    sender.nextseqnum = 3;
    sender.timer_running = 1;
    sender.consecutive_timeouts = 5;

    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 1, 300));

    TEST_ASSERT_EQUAL_UINT32(1, sender.base);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

// tests for senderHandleTimeout

/* Retransmits every outstanding packet in sequence order. */
void test_senderHandleTimeout_retransmitsAllOutstanding(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A', 'B', 'C' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 120, &packet));

    /* The timer began with the first outstanding packet at time 100. */
    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(3, count);

    TEST_ASSERT_EQUAL_UINT32(0, retransmissions[0].seq);
    TEST_ASSERT_EQUAL_UINT32(1, retransmissions[1].seq);
    TEST_ASSERT_EQUAL_UINT32(2, retransmissions[2].seq);

    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_EQUAL_INT(PACKET_DATA, retransmissions[i].type);
        TEST_ASSERT_EQUAL_UINT16(sizeof(data), retransmissions[i].length);
        TEST_ASSERT_EQUAL_MEMORY(
            data, retransmissions[i].payload, sizeof(data));
    }
}

/* Retransmits only packets that have not been acknowledged. */
void test_senderHandleTimeout_retransmitsUnacknowledgedOnly(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'X' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 110, &packet));
    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 120, &packet));

    /* ACK 2 acknowledges packets 0 and 1. */
    TEST_ASSERT_EQUAL_INT(0, senderHandleAck(&sender, 2, 150));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 400, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_EQUAL_UINT32(2, retransmissions[0].seq);
}

/* Restarts the timer after retransmission. */
void test_senderHandleTimeout_restartsTimer(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(350, sender.timer_start);
}

/* Increments the consecutive timeout counter. */
void test_senderHandleTimeout_incrementsTimeoutCount(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, sender.consecutive_timeouts);

    /* A second timeout occurs 250 ms after the restarted timer. */
    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 600, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(2, sender.consecutive_timeouts);
}

/* Rejects a timeout that occurs before the timer expires. */
void test_senderHandleTimeout_timerNotExpired(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 349, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_UINT64(100, sender.timer_start);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

/* Rejects timeout handling when the timer is stopped. */
void test_senderHandleTimeout_timerNotRunning(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
}

/* Rejects a null sender. */
void test_senderHandleTimeout_nullSender(void)
{
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(NULL, 1000, retransmissions, &count));
}

/* Rejects a null retransmission array. */
void test_senderHandleTimeout_nullPackets(void)
{
    SenderState sender;
    size_t count = 0;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, NULL, &count));
}

/* Rejects a null count pointer. */
void test_senderHandleTimeout_nullCount(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 1000, retransmissions, NULL));
}

/* Rejects a timestamp earlier than the timer start. */
void test_senderHandleTimeout_timeWentBackward(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 500, &packet));

    TEST_ASSERT_NOT_EQUAL(
        0, senderHandleTimeout(&sender, 400, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
}

/* Handles the exact timer expiration boundary. */
void test_senderHandleTimeout_exactExpiration(void)
{
    SenderState sender;
    Packet packet;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 0;
    const uint8_t data[] = { 'A' };

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    TEST_ASSERT_EQUAL_INT(
        0, senderSendData(&sender, data, sizeof(data), 100, &packet));

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_EQUAL_UINT32(0, retransmissions[0].seq);
}

/* Clears the count and stops the timer when no packets are outstanding. */
void test_senderHandleTimeout_noOutstandingPackets(void)
{
    SenderState sender;
    Packet retransmissions[MAX_WINDOW];
    size_t count = 99;

    TEST_ASSERT_EQUAL_INT(0, senderInit(&sender, 4, 250));

    sender.base = 2;
    sender.nextseqnum = 2;
    sender.timer_running = 1;
    sender.timer_start = 100;

    TEST_ASSERT_EQUAL_INT(
        0, senderHandleTimeout(&sender, 350, retransmissions, &count));

    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
}

void test_senderHandleTimeout_exceedsConfiguredWindow(void)
{
    SenderState sender;
    Packet packets[MAX_WINDOW];
    size_t count = 0;

    /* Initialize a valid sender state. */
    TEST_ASSERT_EQUAL_INT(
        0, senderInit(&sender, 4, 100)
    );

    /* Set up an expired timer. */
    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 100;

    /* Outstanding packet count = 5, window = 4. */
    sender.base = 0;
    sender.nextseqnum = 5;
    sender.window = 4;

    TEST_ASSERT_EQUAL_INT(
        1,
        senderHandleTimeout(
            &sender, 1100, packets, &count
        )
    );

    TEST_ASSERT_EQUAL_UINT(0, count);
}

// test getTimeMs

/* Test that getTimeMs returns a nonzero timestamp. */
void test_getTimeMs_returns_nonzero(void)
{
    uint64_t now = getTimeMs();

    TEST_ASSERT_TRUE(now > 0);
}

/* Test that repeated calls do not move backward. */
void test_getTimeMs_is_monotonic(void)
{
    uint64_t first = getTimeMs();
    uint64_t second = getTimeMs();

    TEST_ASSERT_TRUE(second >= first);
}

/* Test that time advances or stays the same over a short interval. */
void test_getTimeMs_does_not_decrease_over_time(void)
{
    uint64_t first = getTimeMs();

    /* Perform work between the two calls. */
    volatile unsigned int i;
    for (i = 0; i < 100000U; i++) {
        /* Intentionally empty. */
    }

    uint64_t second = getTimeMs();

    TEST_ASSERT_TRUE(second >= first);
}

/* Test that the timestamp is expressed in milliseconds,
 * rather than seconds or nanoseconds.
 */
void test_getTimeMs_reasonable_timestamp(void)
{
    uint64_t now = getTimeMs();

    /* CLOCK_MONOTONIC normally reports elapsed time since
       an unspecified starting point, not Unix epoch time.
       Check only that the value is plausible and nonzero.
    */
    TEST_ASSERT_TRUE(now > 0);
}

// test senderTimerDue


void test_senderTimerDue_null_sender(void)
{
    uint64_t remaining = 100;

    int result = senderTimerDue(NULL, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(100, remaining);
}

void test_senderTimerDue_null_remaining(void)
{
    SenderState sender = {0};

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1100, NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderTimerDue_timer_not_running(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 0;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_full_timeout_remaining(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(250, remaining);
}

void test_senderTimerDue_partial_timeout_remaining(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1100, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(150, remaining);
}

void test_senderTimerDue_timer_expired(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1250, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_timer_expired_long_ago(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 2000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

void test_senderTimerDue_time_before_timer_start(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 900, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(250, remaining);
}

void test_senderTimerDue_one_ms_before_expiration(void)
{
    SenderState sender = {0};
    uint64_t remaining = 0;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 250;

    int result = senderTimerDue(&sender, 1249, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(1, remaining);
}

void test_senderTimerDue_zero_timeout(void)
{
    SenderState sender = {0};
    uint64_t remaining = 100;

    sender.timer_running = 1;
    sender.timer_start = 1000;
    sender.timeout_ms = 0;

    int result = senderTimerDue(&sender, 1000, &remaining);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT64(0, remaining);
}

// test senderSendFin

void test_senderSendFin_null_sender(void)
{
    Packet packet = {0};

    int result = senderSendFin(NULL, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_null_packet(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;

    int result = senderSendFin(&sender, 1000, NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_unacknowledged_data(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 2;
    sender.nextseqnum = 4;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

void test_senderSendFin_success(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.timeout_ms = 250;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(0, result);

    TEST_ASSERT_EQUAL_INT(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(3, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);

    TEST_ASSERT_EQUAL_INT(PACKET_FIN, sender.fin.type);
    TEST_ASSERT_EQUAL_UINT32(3, sender.fin.seq);
    TEST_ASSERT_EQUAL_UINT16(0, sender.fin.length);

    TEST_ASSERT_EQUAL_INT(1, sender.fin_sent);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);

    TEST_ASSERT_EQUAL_UINT64(1000, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT32(0, sender.consecutive_timeouts);
}

void test_senderSendFin_empty_file(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.timeout_ms = 250;

    int result = senderSendFin(&sender, 500, &packet);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(PACKET_FIN, packet.type);
    TEST_ASSERT_EQUAL_UINT32(0, packet.seq);
    TEST_ASSERT_EQUAL_UINT16(0, packet.length);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_sent);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderSendFin_already_sent(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 3;
    sender.nextseqnum = 3;
    sender.fin_sent = 1;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderSendFin_does_not_modify_state_on_failure(void)
{
    SenderState sender = {0};
    Packet packet = {0};

    sender.base = 1;
    sender.nextseqnum = 2;
    sender.timer_start = 500;
    sender.timer_running = 1;
    sender.fin_sent = 0;

    int result = senderSendFin(&sender, 1000, &packet);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT64(500, sender.timer_start);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_sent);
}

// test senderHandleFinAck

void test_senderHandleFinAck_null_sender(void)
{
    int result = senderHandleFinAck(NULL, 1, 1000);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderHandleFinAck_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.fin_sent = 0;

    int result = senderHandleFinAck(&sender, 1, 1000);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
}

void test_senderHandleFinAck_already_acknowledged(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin_acked = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 6, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(1000, sender.timer_start);
}

void test_senderHandleFinAck_wrong_ack_too_small(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 5, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderHandleFinAck_wrong_ack_too_large(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 7, 1100);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(1, sender.timer_running);
}

void test_senderHandleFinAck_success(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin_acked = 0;
    sender.fin.seq = 5;
    sender.timer_running = 1;
    sender.timer_start = 1000;

    int result = senderHandleFinAck(&sender, 6, 1100);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
    TEST_ASSERT_EQUAL_UINT64(0, sender.timer_start);
}

void test_senderHandleFinAck_empty_file(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 0;
    sender.timer_running = 1;
    sender.timer_start = 500;

    int result = senderHandleFinAck(&sender, 1, 600);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

void test_senderHandleFinAck_timer_already_stopped(void)
{
    SenderState sender = {0};

    sender.fin_sent = 1;
    sender.fin.seq = 3;
    sender.timer_running = 0;
    sender.timer_start = 0;

    int result = senderHandleFinAck(&sender, 4, 2000);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, sender.fin_acked);
    TEST_ASSERT_EQUAL_INT(0, sender.timer_running);
}

//test senderFinished

void test_senderFinished_null_sender(void)
{
    int result = senderFinished(NULL);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_no_packets_sent(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_data_not_fully_acknowledged(void)
{
    SenderState sender = {0};

    sender.base = 2;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_fin_not_acknowledged(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_senderFinished_success(void)
{
    SenderState sender = {0};

    sender.base = 5;
    sender.nextseqnum = 5;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderFinished_empty_file_complete(void)
{
    SenderState sender = {0};

    sender.base = 0;
    sender.nextseqnum = 0;
    sender.fin_sent = 1;
    sender.fin_acked = 1;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_senderFinished_data_acknowledged_but_fin_not_sent(void)
{
    SenderState sender = {0};

    sender.base = 10;
    sender.nextseqnum = 10;
    sender.fin_sent = 0;
    sender.fin_acked = 0;

    int result = senderFinished(&sender);

    TEST_ASSERT_EQUAL_INT(0, result);
}

// test receiverInit

void test_receiverInit_null_receiver(void)
{
    int result = receiverInit(NULL);

    TEST_ASSERT_EQUAL_INT(1, result);
}

void test_receiverInit_success(void)
{
    ReceiverState receiver = {0};

    /* Set non-default values to verify initialization. */
    receiver.expectedseqnum = 10;
    receiver.finished = 1;
    receiver.last_activity = 12345;

    int result = receiverInit(&receiver);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(0, receiver.finished);
    TEST_ASSERT_TRUE(receiver.last_activity > 0);
}

void test_receiverInit_resets_existing_state(void)
{
    ReceiverState receiver = {0};

    receiver.expectedseqnum = 25;
    receiver.finished = 1;
    receiver.last_activity = 99999;

    int result = receiverInit(&receiver);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(0, receiver.finished);
}

void test_receiverInit_sets_activity_timestamp(void)
{
    ReceiverState receiver = {0};

    uint64_t before = getTimeMs();
    int result = receiverInit(&receiver);
    uint64_t after = getTimeMs();

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_TRUE(receiver.last_activity >= before);
    TEST_ASSERT_TRUE(receiver.last_activity <= after);
}


// test receiverHandleData

/* Accept the expected DATA packet and advance the expected sequence number. */
void test_receiverHandleData_expectedPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {10, 20, 30};
    uint64_t old_activity = 0;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 0;
    receiver.finished = 0;
    receiver.last_activity = old_activity;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
    TEST_ASSERT_TRUE(receiver.last_activity != old_activity);
}

/* An in-order packet advances the receiver and returns a cumulative ACK. */
void test_receiverHandleData_multipleInOrderPackets(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {42};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    for (uint32_t seq = 0; seq < 3; seq++) {
        TEST_ASSERT_EQUAL_INT(
            0, makeDataPacket(&packet, seq, data, sizeof(data)));

        TEST_ASSERT_EQUAL_INT(
            0, receiverHandleData(&receiver, &packet, 1000 + seq, &ack));

        TEST_ASSERT_EQUAL_UINT32(seq + 1, receiver.expectedseqnum);
        TEST_ASSERT_EQUAL_UINT32(seq + 1, ack.seq);
        TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    }
}

/* A duplicate DATA packet must not advance expectedseqnum. */
void test_receiverHandleData_duplicatePacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {1, 2};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 1;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
}

/* An out-of-order packet must be discarded logically and ACK the expected seq. */
void test_receiverHandleData_outOfOrderPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {5, 6, 7};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 4, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
}

/* A future packet must not cause the receiver to skip a missing sequence. */
void test_receiverHandleData_missingPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {9};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 3, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
}

/* A zero-length DATA packet is valid if its sequence number is expected. */
void test_receiverHandleData_zeroLengthPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, NULL, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
}

/* A maximum-sized payload must be accepted. */
void test_receiverHandleData_maxPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[MAX_PAYLOAD];

    memset(data, 0xAB, sizeof(data));

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, MAX_PAYLOAD));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
}

/* A NULL receiver must return an error. */
void test_receiverHandleData_nullReceiver(void)
{
    Packet packet;
    Packet ack;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(NULL, &packet, 1000, &ack));
}

/* A NULL input packet must return an error. */
void test_receiverHandleData_nullPacket(void)
{
    ReceiverState receiver;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, NULL, 1000, &ack));
}

/* A NULL ACK output pointer must return an error. */
void test_receiverHandleData_nullAck(void)
{
    ReceiverState receiver;
    Packet packet;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 0, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, NULL));
}

/* Non-DATA packet types must be rejected. */
void test_receiverHandleData_nonDataPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_EQUAL_INT(0, makeAckPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
}

/* Payload lengths above MAX_PAYLOAD must be rejected. */
void test_receiverHandleData_oversizedPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    memset(&packet, 0, sizeof(packet));
    packet.type = PACKET_DATA;
    packet.seq = 0;
    packet.length = MAX_PAYLOAD + 1;

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
}

/* A finished receiver must reject additional DATA packets. */
void test_receiverHandleData_receiverFinished(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {1};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.finished = 1;
    receiver.expectedseqnum = 3;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 3, data, sizeof(data)));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
}

/* Repeated out-of-order packets should generate the same cumulative ACK. */
void test_receiverHandleData_repeatedOutOfOrderPackets(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;
    uint8_t data[] = {8};

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&packet, 5, data, sizeof(data)));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1000, &ack));
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleData(&receiver, &packet, 1001, &ack));
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
}


// test receiverHandleFin

void test_receiverHandleFin_nullReceiver(void)
{
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(NULL, &packet, 1000, &ack));
}

void test_receiverHandleFin_nullPacket(void)
{
    ReceiverState receiver;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, NULL, 1000, &ack));
}

void test_receiverHandleFin_nullAck(void)
{
    ReceiverState receiver;
    Packet packet;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, NULL));
}

void test_receiverHandleFin_nonFinPacket(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeDataPacket(&packet, 0, NULL, 0));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
}

void test_receiverHandleFin_nonzeroPayload(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    /* Simulate a malformed FIN packet. */
    packet.length = 1;

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(0, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
}

void test_receiverHandleFin_expectedSequence(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 3;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 3));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(4, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT64(1000, receiver.last_activity);

    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(4, ack.seq);
    TEST_ASSERT_EQUAL_UINT16(0, ack.length);
}

void test_receiverHandleFin_emptyFile(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 500, &ack));

    TEST_ASSERT_EQUAL_UINT32(1, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(1, ack.seq);
}

void test_receiverHandleFin_outOfOrderSequence(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 2;

    /* Receiver expects DATA sequence 2, but FIN claims sequence 4. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 4));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(2, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);

    /* ACK repeats the current expected sequence number. */
    TEST_ASSERT_EQUAL_UINT8(PACKET_ACK, ack.type);
    TEST_ASSERT_EQUAL_UINT32(2, ack.seq);
}

void test_receiverHandleFin_earlyFin(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 3;

    /* FIN sequence 2 is older than the receiver's expectation. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 2));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_FALSE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);
}

void test_receiverHandleFin_duplicateFin(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.expectedseqnum = 2;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 2));

    /* First FIN completes the transfer. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);

    /* A repeated FIN must receive the same final cumulative ACK. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 1100, &ack));

    TEST_ASSERT_TRUE(receiver.finished);
    TEST_ASSERT_EQUAL_UINT32(3, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_UINT32(3, ack.seq);
    TEST_ASSERT_EQUAL_UINT64(1100, receiver.last_activity);
}

void test_receiverHandleFin_unexpectedFinAfterCompletion(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 4;
    receiver.finished = 1;

    /* The accepted FIN would have had sequence number 3. */
    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 5));

    TEST_ASSERT_NOT_EQUAL(
        0, receiverHandleFin(&receiver, &packet, 1000, &ack));

    TEST_ASSERT_EQUAL_UINT32(4, receiver.expectedseqnum);
    TEST_ASSERT_TRUE(receiver.finished);
}

void test_receiverHandleFin_activityTimestamp(void)
{
    ReceiverState receiver;
    Packet packet;
    Packet ack;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 123;

    TEST_ASSERT_EQUAL_INT(0, makeFinPacket(&packet, 0));

    TEST_ASSERT_EQUAL_INT(
        0, receiverHandleFin(&receiver, &packet, 9876, &ack));

    TEST_ASSERT_EQUAL_UINT64(9876, receiver.last_activity);
}


// test receiverTimerExpired

void test_receiverTimerExpired_nullReceiver(void)
{
    TEST_ASSERT_NOT_EQUAL(0, receiverTimerExpired(NULL, 1000));
}

void test_receiverTimerExpired_noElapsedTime(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 1000));
}

void test_receiverTimerExpired_lessThanTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* 29,999 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 30999));
}

void test_receiverTimerExpired_exactlyAtTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* Exactly 30,000 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 31000));
}

void test_receiverTimerExpired_pastTimeout(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 1000;

    /* 30,001 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 31001));
}

void test_receiverTimerExpired_timeBeforeLastActivity(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 5000;

    /* A timestamp earlier than last_activity must not cause underflow. */
    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 4000));
}

void test_receiverTimerExpired_zeroLastActivity(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));
    receiver.last_activity = 0;

    TEST_ASSERT_EQUAL_INT(0, receiverTimerExpired(&receiver, 29999));
    TEST_ASSERT_EQUAL_INT(1, receiverTimerExpired(&receiver, 30000));
}

void test_receiverTimerExpired_largeTimestamps(void)
{
    ReceiverState receiver;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.last_activity = UINT64_MAX - 40000;

    /* 29,999 ms have elapsed. */
    TEST_ASSERT_EQUAL_INT(
        0, receiverTimerExpired(&receiver, UINT64_MAX - 10001));

    /* Exactly 40,000 ms have elapsed, exceeding the timeout. */
    TEST_ASSERT_EQUAL_INT(
        1, receiverTimerExpired(&receiver, UINT64_MAX));
}

void test_receiverTimerExpired_doesNotModifyState(void)
{
    ReceiverState receiver;
    uint32_t expected;
    int finished;
    uint64_t last_activity;

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    receiver.expectedseqnum = 5;
    receiver.finished = 0;
    receiver.last_activity = 1000;

    expected = receiver.expectedseqnum;
    finished = receiver.finished;
    last_activity = receiver.last_activity;

    (void)receiverTimerExpired(&receiver, 31000);

    TEST_ASSERT_EQUAL_UINT32(expected, receiver.expectedseqnum);
    TEST_ASSERT_EQUAL_INT(finished, receiver.finished);
    TEST_ASSERT_EQUAL_UINT64(last_activity, receiver.last_activity);
}

// test resolveAddr

void test_resolveAddr_validIPv4(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("127.0.0.1", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_EQUAL_INT(AF_INET, results->ai_family);
    TEST_ASSERT_EQUAL_INT(SOCK_DGRAM, results->ai_socktype);
    TEST_ASSERT_EQUAL_INT(IPPROTO_UDP, results->ai_protocol);

    freeaddrinfo(results);
}

void test_resolveAddr_validHostname(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("localhost", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);

    /* AF_UNSPEC permits either IPv4 or IPv6 results. */
    TEST_ASSERT_TRUE(
        results->ai_family == AF_INET ||
        results->ai_family == AF_INET6);

    TEST_ASSERT_EQUAL_INT(SOCK_DGRAM, results->ai_socktype);

    freeaddrinfo(results);
}

void test_resolveAddr_invalidHostname(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0,
        resolveAddr(
            "this-hostname-should-not-exist.invalid",
            "4250",
            &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullHost(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr(NULL, "4250", &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullPort(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr("127.0.0.1", NULL, &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_nullResults(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0, resolveAddr("127.0.0.1", "4250", NULL));
}

void test_resolveAddr_failureClearsResults(void)
{
    /* A failed resolution must not leave a stale output pointer. */
    struct addrinfo *results = (struct addrinfo *)1;

    TEST_ASSERT_NOT_EQUAL(
        0,
        resolveAddr(
            "this-hostname-should-not-exist.invalid",
            "4250",
            &results));

    TEST_ASSERT_NULL(results);
}

void test_resolveAddr_numericService(void)
{
    struct addrinfo *results = NULL;

    TEST_ASSERT_EQUAL_INT(
        0, resolveAddr("127.0.0.1", "4250", &results));

    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_NOT_NULL(results->ai_addr);

    freeaddrinfo(results);
}

// test createSocket

/* Test creating a valid UDP IPv4 socket. */
void test_createSocket_validUDPIPv4(void)
{
    struct addrinfo addr = {0};
    int sockfd;

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    sockfd = createSocket(&addr);

    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, sockfd);

    close(sockfd);
}

/* Test creating a valid UDP IPv6 socket. */
void test_createSocket_validUDPIPv6(void)
{
    struct addrinfo addr = {0};
    int sockfd;

    addr.ai_family = AF_INET6;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    sockfd = createSocket(&addr);

    /*
     * IPv6 may be disabled on some systems.
     * If socket creation succeeds, close the descriptor.
     */
    if (sockfd >= 0) {
        close(sockfd);
    }
}

/* Test that a NULL address is rejected. */
void test_createSocket_nullAddress(void)
{
    TEST_ASSERT_EQUAL_INT(-1, createSocket(NULL));
}

/* Test that an invalid address family fails. */
void test_createSocket_invalidFamily(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = -1;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = IPPROTO_UDP;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

/* Test that an invalid socket type fails. */
void test_createSocket_invalidSocketType(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = AF_INET;
    addr.ai_socktype = -1;
    addr.ai_protocol = IPPROTO_UDP;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

/* Test that an unsupported protocol fails. */
void test_createSocket_invalidProtocol(void)
{
    struct addrinfo addr = {0};

    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_DGRAM;
    addr.ai_protocol = -1;

    TEST_ASSERT_EQUAL_INT(-1, createSocket(&addr));
}

// test registerReceiver

/*
 * Start a relay process that receives one HELLO message
 * and replies with the specified response.
 */
static int startTestRelay(const char *response,
                          const char *expected_hello)
{
    relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (relay_fd < 0) {
        return -1;
    }

    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    if (bind(relay_fd,
             (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)) < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    socklen_t addr_len = sizeof(relay_addr);

    if (getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &addr_len) < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    relay_pid = fork();

    if (relay_pid < 0) {
        close(relay_fd);
        relay_fd = -1;
        return -1;
    }

    if (relay_pid == 0) {
        /* Child process acts as the relay. */
        char buffer[128];
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            relay_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len);

        if (received < 0) {
            _exit(2);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, expected_hello) != 0) {
            _exit(3);
        }

        ssize_t sent = sendto(
            relay_fd,
            response,
            strlen(response),
            0,
            (struct sockaddr *)&client_addr,
            client_len);

        _exit(sent == (ssize_t)strlen(response) ? 0 : 4);
    }

    return 0;
}

/* Create a client UDP socket. */
static int createTestClient(void)
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

/* Test successful registration. */
void test_registerReceiver_success(void)
{
    TEST_ASSERT_EQUAL_INT(
        0, startTestRelay("OK", "HELLO ta-2 recv"));

    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2");

    TEST_ASSERT_EQUAL_INT(0, result);

    close(sockfd);
}

/* Test that the relay's error response is rejected. */
void test_registerReceiver_relayError(void)
{
    TEST_ASSERT_EQUAL_INT(
        0, startTestRelay("ERR receiver required",
                          "HELLO ta-2 recv"));

    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null socket address. */
void test_registerReceiver_nullRelay(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd, NULL, sizeof(relay_addr), "ta-2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null session name. */
void test_registerReceiver_nullSession(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        NULL);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test an invalid session character. */
void test_registerReceiver_invalidSessionCharacter(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta_2");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a session name longer than 32 characters. */
void test_registerReceiver_sessionTooLong(void)
{
    int sockfd = createTestClient();
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerReceiver(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "abcdefghijklmnopqrstuvwxyz1234567");

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test an invalid socket descriptor. */
void test_registerReceiver_invalidSocket(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        registerReceiver(
            -1,
            (const struct sockaddr *)&relay_addr,
            sizeof(relay_addr),
            "ta-2"));
}

void test_registerReceiver_ipv6_address_check(void)
{
    int client_fd = -1;
    int test_relay_fd = -1;
    int wrong_fd = -1;
    pid_t child_pid = -1;

    struct sockaddr_in6 relay6 = {0};
    struct sockaddr_in6 wrong6 = {0};

    client_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    test_relay_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    wrong_fd = socket(AF_INET6, SOCK_DGRAM, 0);

    TEST_ASSERT_TRUE(client_fd >= 0);
    TEST_ASSERT_TRUE(test_relay_fd >= 0);
    TEST_ASSERT_TRUE(wrong_fd >= 0);

    relay6.sin6_family = AF_INET6;
    relay6.sin6_addr = in6addr_loopback;
    relay6.sin6_port = 0;

    wrong6.sin6_family = AF_INET6;
    wrong6.sin6_addr = in6addr_loopback;
    wrong6.sin6_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0, bind(test_relay_fd, (struct sockaddr *)&relay6, sizeof(relay6)));
    TEST_ASSERT_EQUAL_INT(
        0, bind(wrong_fd, (struct sockaddr *)&wrong6, sizeof(wrong6)));

    socklen_t relay_len = sizeof(relay6);
    socklen_t wrong_len = sizeof(wrong6);

    TEST_ASSERT_EQUAL_INT(
        0, getsockname(test_relay_fd, (struct sockaddr *)&relay6, &relay_len));
    TEST_ASSERT_EQUAL_INT(
        0, getsockname(wrong_fd, (struct sockaddr *)&wrong6, &wrong_len));

    child_pid = fork();
    TEST_ASSERT_TRUE(child_pid >= 0);

    if (child_pid == 0) {
        char buffer[128];
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            test_relay_fd, buffer, sizeof(buffer) - 1, 0,
            (struct sockaddr *)&client_addr, &client_len);

        if (received < 0) {
            _exit(2);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, "HELLO ta-2 recv") != 0) {
            _exit(3);
        }

        /* Send an invalid reply from a different IPv6 port. */
        if (sendto(wrong_fd, "OK", 2, 0,
                   (struct sockaddr *)&client_addr, client_len) != 2) {
            _exit(4);
        }

        /* Then send the valid reply from the expected relay address. */
        if (sendto(test_relay_fd, "OK", 2, 0,
                   (struct sockaddr *)&client_addr, client_len) != 2) {
            _exit(5);
        }

        _exit(0);
    }

    int result = registerReceiver(
        client_fd, (struct sockaddr *)&relay6, relay_len, "ta-2");

    TEST_ASSERT_EQUAL_INT(0, result);

    int status = 0;
    TEST_ASSERT_EQUAL_INT(child_pid, waitpid(child_pid, &status, 0));
    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));

    close(client_fd);
    close(test_relay_fd);
    close(wrong_fd);
}

void test_registerReceiver_fails_after_five_attempts(void)
{
    int client_fd = -1;
    int test_relay_fd = -1;
    pid_t child_pid = -1;

    struct sockaddr_in relay = {0};

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    test_relay_fd = socket(AF_INET, SOCK_DGRAM, 0);

    TEST_ASSERT_TRUE(client_fd >= 0);
    TEST_ASSERT_TRUE(test_relay_fd >= 0);

    relay.sin_family = AF_INET;
    relay.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0, bind(test_relay_fd, (struct sockaddr *)&relay, sizeof(relay)));

    socklen_t relay_len = sizeof(relay);
    TEST_ASSERT_EQUAL_INT(
        0, getsockname(test_relay_fd, (struct sockaddr *)&relay, &relay_len));

    child_pid = fork();
    TEST_ASSERT_TRUE(child_pid >= 0);

    if (child_pid == 0) {
        char buffer[128];

        for (int i = 0; i < 5; i++) {
            struct sockaddr_storage client_addr;
            socklen_t client_len = sizeof(client_addr);

            ssize_t received = recvfrom(
                test_relay_fd, buffer, sizeof(buffer) - 1, 0,
                (struct sockaddr *)&client_addr, &client_len);

            if (received < 0) {
                _exit(2);
            }

            buffer[received] = '\0';

            if (strcmp(buffer, "HELLO ta-2 recv") != 0) {
                _exit(3);
            }

            /* Reply with an invalid response on every attempt. */
            if (sendto(test_relay_fd, "NOPE", 4, 0,
                       (struct sockaddr *)&client_addr, client_len) != 4) {
                _exit(4);
            }
        }

        _exit(0);
    }

    int result = registerReceiver(
        client_fd, (struct sockaddr *)&relay, relay_len, "ta-2");

    TEST_ASSERT_EQUAL_INT(1, result);

    int status = 0;
    TEST_ASSERT_EQUAL_INT(child_pid, waitpid(child_pid, &status, 0));
    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));

    close(client_fd);
    close(test_relay_fd);
}

// test registerSender

/* Test successful sender registration. */
void test_registerSender_success(void)
{
    TEST_ASSERT_EQUAL_INT(
        0,
        startTestRelay(
            "OK",
            "HELLO ta-2 send 0.1 0.05 0"));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    TEST_ASSERT_EQUAL_INT(0, result);

    close(sockfd);
}

/* Test that the sender rejects an ERR response. */
void test_registerSender_relayError(void)
{
    TEST_ASSERT_EQUAL_INT(
        0,
        startTestRelay(
            "ERR receiver required",
            "HELLO ta-2 send 0.1 0.05 0"));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null relay address. */
void test_registerSender_nullRelay(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd, NULL, sizeof(relay_addr),
        "ta-2", 0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test a null session. */
void test_registerSender_nullSession(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        NULL,
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of invalid session characters. */
void test_registerSender_invalidSession(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta_2",
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a session longer than 32 characters. */
void test_registerSender_sessionTooLong(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "abcdefghijklmnopqrstuvwxyz1234567",
        0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a negative loss probability. */
void test_registerSender_negativeLoss(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        -0.1, 0.05, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a corruption probability greater than 0.5. */
void test_registerSender_corruptTooHigh(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1, 0.6, 0.0);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of a duplication probability greater than 0.5. */
void test_registerSender_dupTooHigh(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1, 0.05, 0.8);

    TEST_ASSERT_NOT_EQUAL(0, result);

    close(sockfd);
}

/* Test rejection of an invalid socket descriptor. */
void test_registerSender_invalidSocket(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        registerSender(
            -1,
            (const struct sockaddr *)&relay_addr,
            sizeof(relay_addr),
            "ta-2",
            0.1, 0.05, 0.0));
}

void test_registerSender_ipv6_success(void)
{
    /* Create an IPv6 UDP relay socket. */
    relay_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(relay_fd >= 0);

    struct sockaddr_in6 relay_addr6;
    memset(&relay_addr6, 0, sizeof(relay_addr6));

    relay_addr6.sin6_family = AF_INET6;
    relay_addr6.sin6_addr = in6addr_loopback;
    relay_addr6.sin6_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(relay_fd,
             (struct sockaddr *)&relay_addr6,
             sizeof(relay_addr6)));

    socklen_t relay_len = sizeof(relay_addr6);

    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr6,
                    &relay_len));

    /* Fork a child process to act as the relay. */
    relay_pid = fork();
    TEST_ASSERT_TRUE(relay_pid >= 0);

    if (relay_pid == 0) {
        char buffer[128];
        struct sockaddr_in6 client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t received = recvfrom(
            relay_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len);

        if (received < 0) {
            _exit(1);
        }

        buffer[received] = '\0';

        if (strcmp(buffer, "HELLO ta-2 send 0.1 0.05 0") != 0) {
            _exit(2);
        }

        const char response[] = "OK";

        ssize_t sent = sendto(
            relay_fd,
            response,
            sizeof(response) - 1,
            0,
            (struct sockaddr *)&client_addr,
            client_len);

        _exit(sent == (ssize_t)(sizeof(response) - 1) ? 0 : 3);
    }

    /* Create an IPv6 client socket. */
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    /* Register the sender using an IPv6 relay address. */
    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr6,
        sizeof(relay_addr6),
        "ta-2",
        0.1,
        0.05,
        0.0);

    close(sockfd);

    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_registerSender_wrongRelayAddress(void)
{
    /* Socket representing the expected relay address. */
    relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(relay_fd >= 0);

    memset(&relay_addr, 0, sizeof(relay_addr));
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(relay_fd,
             (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)));

    socklen_t relay_len = sizeof(relay_addr);
    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &relay_len));

    /* Create a different socket to send the response. */
    int wrong_fd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(wrong_fd >= 0);

    struct sockaddr_in wrong_addr;
    memset(&wrong_addr, 0, sizeof(wrong_addr));
    wrong_addr.sin_family = AF_INET;
    wrong_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    wrong_addr.sin_port = 0;

    TEST_ASSERT_EQUAL_INT(
        0,
        bind(wrong_fd,
             (struct sockaddr *)&wrong_addr,
             sizeof(wrong_addr)));

    socklen_t wrong_len = sizeof(wrong_addr);
    TEST_ASSERT_EQUAL_INT(
        0,
        getsockname(wrong_fd,
                    (struct sockaddr *)&wrong_addr,
                    &wrong_len));

    relay_pid = fork();
    TEST_ASSERT_TRUE(relay_pid >= 0);

    if (relay_pid == 0) {
        /* Keep relay_fd open so HELLO reaches the child. */

        for (int i = 0; i < 5; i++) {
            char buffer[128];
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            ssize_t received = recvfrom(
                relay_fd,
                buffer,
                sizeof(buffer),
                0,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            if (received < 0) {
                _exit(1);
            }

            /* Respond from a different port than relay_fd. */
            ssize_t sent = sendto(
                wrong_fd,
                "OK",
                2,
                0,
                (struct sockaddr *)&client_addr,
                client_len
            );

            if (sent != 2) {
                _exit(2);
            }
        }

        _exit(0);
    }

    close(wrong_fd);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    TEST_ASSERT_TRUE(sockfd >= 0);

    int result = registerSender(
        sockfd,
        (const struct sockaddr *)&relay_addr,
        sizeof(relay_addr),
        "ta-2",
        0.1,
        0.05,
        0.0);

    close(sockfd);

    /* No response came from the expected relay address. */
    TEST_ASSERT_NOT_EQUAL(0, result);
}

// runSender tests

/*
 * Create a temporary file containing the supplied data.
 * Returns 0 on success and -1 on failure.
 */
static int createTempFile(const char *contents,
                          char *path,
                          size_t path_size)
{
    const char *template_path = "/tmp/runSender_test_XXXXXX";
    int fd;

    if (path == NULL || path_size < strlen(template_path) + 1) {
        return -1;
    }

    strcpy(path, template_path);

    fd = mkstemp(path);
    if (fd < 0) {
        return -1;
    }

    if (contents != NULL) {
        size_t length = strlen(contents);
        size_t written = 0;

        while (written < length) {
            ssize_t n = write(fd, contents + written,
                              length - written);

            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }

                close(fd);
                unlink(path);
                return -1;
            }

            written += (size_t)n;
        }
    }

    if (close(fd) != 0) {
        unlink(path);
        return -1;
    }

    return 0;
}

/*
 * Start a local UDP relay that rejects sender registration.
 *
 * The relay replies "ERR test rejection" to each registration
 * datagram. The listening socket is bound to an available local port.
 *
 * Returns 0 on success and -1 on failure.
 */
static int startRejectingRelay(void)
{
    int sockfd;
    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        close(sockfd);
        return -1;
    }

    if (getsockname(sockfd, (struct sockaddr *)&address,
                    &address_len) < 0) {
        close(sockfd);
        return -1;
    }

    if (snprintf(relay_port, sizeof(relay_port), "%u",
                 (unsigned int)ntohs(address.sin_port)) >=
        (int)sizeof(relay_port)) {
        close(sockfd);
        return -1;
    }

    relay_pid = fork();

    if (relay_pid < 0) {
        close(sockfd);
        return -1;
    }

    if (relay_pid == 0) {
        /*
         * Child process: handle registration requests.
         * A timeout prevents the child from blocking indefinitely
         * if the test exits before all retries are sent.
         */
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        (void)setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                         &timeout, sizeof(timeout));

        for (int i = 0; i < 5; i++) {
            char request[256];
            struct sockaddr_storage client;
            socklen_t client_len = sizeof(client);

            ssize_t received =
                recvfrom(sockfd, request, sizeof(request), 0,
                         (struct sockaddr *)&client, &client_len);

            if (received < 0) {
                if (errno == EINTR) {
                    i--;
                    continue;
                }

                break;
            }

            static const char response[] = "ERR test rejection";

            (void)sendto(sockfd, response, sizeof(response) - 1,
                         0, (struct sockaddr *)&client, client_len);
        }

        close(sockfd);
        _exit(0);
    }

    /* Parent process no longer needs its copy of the socket. */
    close(sockfd);
    return 0;
}

/* ---------- Argument validation tests ---------- */

void test_runSender_nullRelay_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender(NULL, "4250", "ta-2", "input.txt",
                  8, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_nullFilename_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", NULL,
                  8, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_zeroWindow_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  0, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_windowAboveMaximum_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  MAX_WINDOW + 1, 250, 0.0, 0.0, 0.0)
    );
}

void test_runSender_zeroTimeout_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2", "input.txt",
                  8, 0, 0.0, 0.0, 0.0)
    );
}

/* ---------- File handling tests ---------- */

void test_runSender_missingFile_returnsFailure(void)
{
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "4250", "ta-2",
                  "/tmp/runSender_file_that_does_not_exist",
                  8, 250, 0.0, 0.0, 0.0)
    );
}

/* ---------- Address resolution tests ---------- */

void test_runSender_invalidPort_returnsFailure(void)
{
    char filename[64];

    TEST_ASSERT_EQUAL_INT(
        0, createTempFile("test data", filename, sizeof(filename))
    );

    /*
     * The service name is deliberately invalid, so address
     * resolution should fail before relay registration.
     */
    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", "invalid_port", "ta-2",
                  filename, 8, 250, 0.0, 0.0, 0.0)
    );

    unlink(filename);
}

/* ---------- Relay registration tests ---------- */

void test_runSender_relayRejectsRegistration_returnsFailure(void)
{
    char filename[64];

    TEST_ASSERT_EQUAL_INT(
        0, createTempFile("test data", filename, sizeof(filename))
    );

    TEST_ASSERT_EQUAL_INT(0, startRejectingRelay());

    TEST_ASSERT_NOT_EQUAL(
        0,
        runSender("127.0.0.1", relay_port, "ta-2",
                  filename, 8, 250, 0.0, 0.0, 0.0)
    );

    unlink(filename);
}


// test runReceiver

/*
 * Wait for a datagram on the mock relay socket.
 * Returns the received byte count, or -1 on failure/timeout.
 */
static ssize_t testRecvDatagram(uint8_t *buffer, size_t capacity,
                                struct sockaddr_in *source,
                                int timeout_ms)
{
    struct pollfd pfd;
    socklen_t source_len = sizeof(*source);

    pfd.fd = test_relay_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int result = poll(&pfd, 1, timeout_ms);

    if (result <= 0 || !(pfd.revents & POLLIN)) {
        return -1;
    }

    return recvfrom(test_relay_fd, buffer, capacity, 0,
                    (struct sockaddr *)source, &source_len);
}

/*
 * Send an encoded packet from the mock relay to the receiver.
 */
static int testSendPacket(const Packet *packet)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size;

    if (encodePacket(packet, buffer, sizeof(buffer),
                     &packet_size) != 0) {
        return -1;
    }

    ssize_t sent = sendto(
        test_relay_fd, buffer, packet_size, 0,
        (struct sockaddr *)&test_receiver_addr,
        sizeof(test_receiver_addr));

    return sent == (ssize_t)packet_size ? 0 : -1;
}

/*
 * Receive and decode an ACK from the receiver.
 */
static int testReceiveAck(uint32_t expected_seq)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    struct sockaddr_in source;
    Packet ack;

    ssize_t received = testRecvDatagram(
        buffer, sizeof(buffer), &source, TEST_TIMEOUT_MS);

    if (received < 0) {
        return -1;
    }

    if (decodePacket(buffer, (size_t)received, &ack) != 0) {
        return -1;
    }

    if (ack.type != PACKET_ACK ||
        ack.seq != expected_seq ||
        ack.length != 0) {
        return -1;
    }

    return 0;
}

/*
 * Start a mock relay and a child process running runReceiver().
 * The parent process completes the receiver's registration.
 */
static int testStartReceiver(void)
{
    struct sockaddr_in relay_addr;
    socklen_t relay_len = sizeof(relay_addr);
    char port[TEST_PORT_SIZE];
    uint8_t buffer[MAX_PACKET_SIZE];
    struct sockaddr_in source;
    ssize_t received;

    test_relay_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (test_relay_fd < 0) {
        return -1;
    }

    memset(&relay_addr, 0, sizeof(relay_addr));
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    relay_addr.sin_port = 0;

    if (bind(test_relay_fd, (struct sockaddr *)&relay_addr,
             sizeof(relay_addr)) < 0) {
        return -1;
    }

    if (getsockname(test_relay_fd,
                    (struct sockaddr *)&relay_addr,
                    &relay_len) < 0) {
        return -1;
    }

    if (snprintf(port, sizeof(port), "%u",
                 (unsigned int)ntohs(relay_addr.sin_port))
        >= (int)sizeof(port)) {
        return -1;
    }

    snprintf(test_output_path, sizeof(test_output_path),
             "/tmp/runReceiver-test-%ld.bin", (long)getpid());

    test_receiver_pid = fork();

    if (test_receiver_pid < 0) {
        return -1;
    }

    if (test_receiver_pid == 0) {
        /*
         * The receiver opens its own socket.
         * The inherited relay descriptor is unused by the child.
         */
        int result = runReceiver(
            "127.0.0.1", port, "ta-2", test_output_path);

        _exit(result == 0 ? 0 : 1);
    }

    /*
     * Receive the receiver's HELLO registration.
     */
    received = testRecvDatagram(
        buffer, sizeof(buffer), &source, TEST_TIMEOUT_MS);

    if (received < 0) {
        return -1;
    }

    buffer[(size_t)received < sizeof(buffer)
           ? (size_t)received : sizeof(buffer) - 1] = '\0';

    if (strcmp((char *)buffer, "HELLO ta-2 recv") != 0) {
        return -1;
    }

    test_receiver_addr = source;

    /*
     * Complete registration.
     */
    if (sendto(test_relay_fd, "OK", 2, 0,
               (struct sockaddr *)&test_receiver_addr,
               sizeof(test_receiver_addr)) != 2) {
        return -1;
    }

    return 0;
}


/*
 * Send FIN and check the final cumulative ACK.
 */
static int testSendFinAndFinish(uint32_t seq)
{
    Packet fin;

    if (makeFinPacket(&fin, seq) != 0) {
        return -1;
    }

    if (testSendPacket(&fin) != 0) {
        return -1;
    }

    /*
     * FIN advances the receiver's expected sequence number,
     * so the final ACK is seq + 1.
     */
    return testReceiveAck(seq + 1);
}

/*
 * A valid DATA packet followed by FIN should produce cumulative
 * ACKs and write the original payload to the output file.
 */
void test_runReceiver_validDataAndFin(void)
{
    const uint8_t content[] = {
        'H', 'e', 'l', 'l', 'o', '\0', 'U', 'D', 'P'
    };

    Packet data;
    FILE *file;
    uint8_t actual[sizeof(content)];
    size_t count;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, content, sizeof(content)));

    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(1));

    /*
     * The child exits after its FIN linger period.
     */
    int status;
    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    count = fread(actual, 1, sizeof(actual), file);

    TEST_ASSERT_EQUAL_size_t(sizeof(content), count);
    TEST_ASSERT_EQUAL_MEMORY(content, actual, sizeof(content));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

/*
 * An empty file consists of a FIN packet with sequence number 0.
 * The receiver should create an empty output file and exit cleanly.
 */
void test_runReceiver_emptyFile(void)
{
    FILE *file;
    int status;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());
    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(0));

    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    TEST_ASSERT_EQUAL_INT(EOF, fgetc(file));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}

/*
 * A corrupted datagram must be silently discarded.
 * The following valid DATA packet should still be accepted.
 */
void test_runReceiver_discardsCorruptedPacket(void)
{
    Packet data;
    uint8_t buffer[MAX_PACKET_SIZE];
    size_t packet_size;
    struct sockaddr_in source;

    const uint8_t content[] = { 'A', 'B', 'C' };

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, content, sizeof(content)));

    TEST_ASSERT_EQUAL_INT(
        0, encodePacket(&data, buffer, sizeof(buffer), &packet_size));

    /* Damage the packet without updating its checksum. */
    buffer[HEADER_SIZE] ^= 0x01;

    TEST_ASSERT_EQUAL_INT(
        (int)packet_size,
        (int)sendto(test_relay_fd, buffer, packet_size, 0,
                    (struct sockaddr *)&test_receiver_addr,
                    sizeof(test_receiver_addr)));

    /*
     * A corrupted packet must not produce an ACK.
     */
    uint8_t ack_buffer[MAX_PACKET_SIZE];

    TEST_ASSERT_TRUE(
        testRecvDatagram(ack_buffer, sizeof(ack_buffer),
                         &source, 200) < 0);

    /* Send the intact packet and verify it is acknowledged. */
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(1));
}

/*
 * Out-of-order DATA must not advance the receiver or be written.
 * The receiver should ACK the next expected sequence number.
 */
void test_runReceiver_outOfOrderData(void)
{
    const uint8_t first[] = { 'A' };
    const uint8_t second[] = { 'B' };
    Packet data;

    TEST_ASSERT_EQUAL_INT(0, testStartReceiver());

    /* Sequence 1 arrives before sequence 0. */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 1, second, sizeof(second)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(0));

    /* Sequence 0 is accepted. */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 0, first, sizeof(first)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(1));

    /*
     * Sequence 1 can now be accepted.
     */
    TEST_ASSERT_EQUAL_INT(
        0, makeDataPacket(&data, 1, second, sizeof(second)));
    TEST_ASSERT_EQUAL_INT(0, testSendPacket(&data));
    TEST_ASSERT_EQUAL_INT(0, testReceiveAck(2));

    TEST_ASSERT_EQUAL_INT(0, testSendFinAndFinish(2));

    int status;
    TEST_ASSERT_EQUAL_INT(
        test_receiver_pid,
        waitpid(test_receiver_pid, &status, 0));

    TEST_ASSERT_TRUE(WIFEXITED(status));
    TEST_ASSERT_EQUAL_INT(0, WEXITSTATUS(status));
    test_receiver_pid = -1;

    FILE *file = fopen(test_output_path, "rb");
    TEST_ASSERT_NOT_NULL(file);

    uint8_t actual[2];
    TEST_ASSERT_EQUAL_size_t(2, fread(actual, 1, sizeof(actual), file));
    TEST_ASSERT_EQUAL_UINT8('A', actual[0]);
    TEST_ASSERT_EQUAL_UINT8('B', actual[1]);
    TEST_ASSERT_EQUAL_INT(EOF, fgetc(file));
    TEST_ASSERT_EQUAL_INT(0, fclose(file));
}


/*
 * A complete transfer with packet loss, corruption, and duplication.
 * 
 */

/* Keep the test bounded and reproducible. */
#define TEST_DATA_SIZE (MAX_PAYLOAD * 100U + 37U)
#define TEST_QUEUE_CAPACITY 512U
#define TEST_RELAY_TIMEOUT_MS 25U
#define TEST_MAX_STEPS 2000000U

typedef struct {
    uint8_t data[MAX_PACKET_SIZE];
    size_t length;
} TestDatagram;

typedef struct {
    TestDatagram items[TEST_QUEUE_CAPACITY];
    size_t head;
    size_t count;
} TestChannel;

typedef struct {
    unsigned int dropped;
    unsigned int corrupted;
    unsigned int duplicated;
} TestFaultStats;

/* Two independent in-memory channels. */
static TestChannel data_channel;
static TestChannel ack_channel;

static TestFaultStats data_faults;
static TestFaultStats ack_faults;

/* Queue one datagram without applying additional faults. */
static int testQueuePush(TestChannel *channel,
                         const uint8_t *data,
                         size_t length)
{
    size_t index;

    if (channel == NULL || data == NULL ||
        length > MAX_PACKET_SIZE ||
        channel->count >= TEST_QUEUE_CAPACITY) {
        return 1;
    }

    index = (channel->head + channel->count) %
            TEST_QUEUE_CAPACITY;

    memcpy(channel->items[index].data, data, length);
    channel->items[index].length = length;
    channel->count++;

    return 0;
}

/* Remove one datagram from a channel. */
static int testQueuePop(TestChannel *channel,
                        TestDatagram *datagram)
{
    if (channel == NULL || datagram == NULL ||
        channel->count == 0U) {
        return 1;
    }

    *datagram = channel->items[channel->head];

    channel->head =
        (channel->head + 1U) % TEST_QUEUE_CAPACITY;

    channel->count--;

    return 0;
}

/*
 * Simulate a lossy, corrupting, duplicating network.
 *
 * Drop, corruption, and duplication are independent events,
 * each with probability 20%.
 *
 * A corrupted datagram has one randomly selected bit flipped.
 */
static int testTransmit(TestChannel *channel,
                        const uint8_t *data,
                        size_t length,
                        TestFaultStats *stats)
{
    uint8_t damaged[MAX_PACKET_SIZE];
    int drop;
    int corrupt;
    int duplicate;

    if (channel == NULL || data == NULL || stats == NULL ||
        length == 0U || length > MAX_PACKET_SIZE) {
        return 1;
    }

    /* One independent random trial per fault type. */
    drop = (rand() % 100) < 20;
    corrupt = (rand() % 100) < 20;
    duplicate = (rand() % 100) < 20;

    if (drop) {
        stats->dropped++;
        return 0;
    }

    memcpy(damaged, data, length);

    if (corrupt) {
        size_t byte_index = (size_t)rand() % length;
        unsigned int bit_index = (unsigned int)rand() % 8U;

        damaged[byte_index] ^=
            (uint8_t)(1U << bit_index);

        stats->corrupted++;
    }

    if (testQueuePush(channel, damaged, length) != 0) {
        return 1;
    }

    if (duplicate) {
        if (testQueuePush(channel, damaged, length) != 0) {
            return 1;
        }

        stats->duplicated++;
    }

    return 0;
}

static int testQueueEncodedPacket(
    TestChannel *channel,
    const Packet *packet,
    TestFaultStats *stats)
{
    uint8_t encoded[MAX_PACKET_SIZE];
    size_t encoded_length = 0;

    if (encodePacket(packet, encoded, sizeof(encoded),
                     &encoded_length) != 0) {
        return 1;
    }

    return testTransmit(
        channel,
        encoded,
        encoded_length,
        stats
    );
}

/*
 * Process one datagram from the sender-to-receiver channel.
 *
 * Valid in-order DATA is copied into the simulated destination file.
 * The receiver's cumulative ACK is transmitted through the reverse
 * channel, where it is subject to its own faults.
 */
static int testProcessData(TestChannel *channel,
                           ReceiverState *receiver,
                           uint8_t *output,
                           size_t output_capacity,
                           size_t *output_length,
                           uint64_t now_ms)
{
    TestDatagram datagram;
    Packet packet;
    Packet ack;
    uint32_t expected_before;

    if (testQueuePop(channel, &datagram) != 0) {
        return 0; /* No datagram available. */
    }

    /* Corrupted or malformed packets are discarded. */
    if (decodePacket(datagram.data, datagram.length,
                     &packet) != 0) {
        return 0;
    }

    if (packet.type == PACKET_DATA) {
        expected_before = receiver->expectedseqnum;

        if (receiverHandleData(receiver, &packet,
                               now_ms, &ack) != 0) {
            return 1;
        }

        /*
         * Write only a packet that was expected before handling it.
         * This models the receiver application writing accepted bytes.
         */
        if (packet.seq == expected_before) {
            if ((size_t)packet.length >
                output_capacity - *output_length) {
                return 1;
            }

            memcpy(output + *output_length,
                   packet.payload, packet.length);

            *output_length += packet.length;
        }
    } else if (packet.type == PACKET_FIN) {
        if (receiverHandleFin(receiver, &packet,
                              now_ms, &ack) != 0) {
            return 1;
        }
    } else {
        return 0; /* ACKs are not receiver-bound data. */
    }

    return testQueueEncodedPacket(&ack_channel, &ack, &ack_faults);
}

/* Process one reverse-channel datagram at the sender. */
static int testProcessAck(TestChannel *channel,
                          SenderState *sender,
                          uint64_t now_ms)
{
    TestDatagram datagram;
    Packet packet;

    if (testQueuePop(channel, &datagram) != 0) {
        return 0;
    }

    if (decodePacket(datagram.data, datagram.length,
                     &packet) != 0) {
        return 0;
    }

    if (packet.type != PACKET_ACK) {
        return 0;
    }

    if (sender->fin_sent && !sender->fin_acked) {
        /*
         * Ignore unrelated ACKs, including stale DATA ACKs.
         */
        (void)senderHandleFinAck(sender, packet.seq, now_ms);
    } else {
        (void)senderHandleAck(sender, packet.seq, now_ms);
    }

    return 0;
}

void test_completeTransfer_withLossCorruptionAndDuplication(void)
{
    static uint8_t source[TEST_DATA_SIZE];
    static uint8_t destination[TEST_DATA_SIZE];

    SenderState sender;
    ReceiverState receiver;

    size_t source_offset = 0U;
    size_t destination_length = 0U;

    uint64_t now_ms = 0U;
    unsigned int steps;

    memset(&data_channel, 0, sizeof(data_channel));
    memset(&ack_channel, 0, sizeof(ack_channel));
    memset(&data_faults, 0, sizeof(data_faults));
    memset(&ack_faults, 0, sizeof(ack_faults));
    memset(destination, 0, sizeof(destination));

    /*
     * Fixed seed: the same packet-fault sequence is generated
     * on each run with the same C library implementation.
     */
    srand(0x5255U);

    /* Use a nontrivial binary payload with varied byte values. */
    for (size_t i = 0U; i < sizeof(source); i++) {
        source[i] = (uint8_t)((i * 37U + i / 7U + 19U) % 256U);
    }

    TEST_ASSERT_EQUAL_INT(
        0, senderInit(&sender, 8U, TEST_RELAY_TIMEOUT_MS));

    TEST_ASSERT_EQUAL_INT(0, receiverInit(&receiver));

    /*
     * Drive both state machines until FIN is acknowledged.
     * Time advances deterministically rather than using a clock.
     */
    for (steps = 0U;
         steps < TEST_MAX_STEPS && !senderFinished(&sender);
         steps++, now_ms++) {

        /*
         * Fill the Go-Back-N window with DATA packets.
         */
        while (source_offset < sizeof(source) &&
               sender.nextseqnum - sender.base < sender.window) {

            size_t remaining = sizeof(source) - source_offset;
            uint16_t chunk_length =
                (uint16_t)(remaining > MAX_PAYLOAD
                           ? MAX_PAYLOAD : remaining);

            Packet packet;

            TEST_ASSERT_EQUAL_INT(
                0,
                senderSendData(&sender,
                               source + source_offset,
                               chunk_length,
                               now_ms,
                               &packet));

            TEST_ASSERT_EQUAL_INT(
                0,
                testQueueEncodedPacket(&data_channel, &packet, &data_faults));

            source_offset += chunk_length;
        }

        /*
         * Process at most one DATA/FIN and one ACK per iteration.
         * This permits queued duplicates and reordering caused by
         * retransmission to be exercised.
         */
        TEST_ASSERT_EQUAL_INT(
            0,
            testProcessData(&data_channel, &receiver,
                            destination, sizeof(destination),
                            &destination_length, now_ms));

        TEST_ASSERT_EQUAL_INT(
            0,
            testProcessAck(&ack_channel, &sender, now_ms));

        /*
         * Send FIN after all source bytes have been queued and all
         * DATA packets have been acknowledged.
         */
        if (source_offset == sizeof(source) &&
            sender.base == sender.nextseqnum &&
            !sender.fin_sent) {

            Packet fin;

            TEST_ASSERT_EQUAL_INT(
                0, senderSendFin(&sender, now_ms, &fin));

            TEST_ASSERT_EQUAL_INT(
                0, testQueueEncodedPacket(&data_channel, &fin, &data_faults));
        }

        /*
         * Expire the oldest outstanding DATA timer and retransmit
         * all unacknowledged DATA packets.
         */
        if (sender.timer_running &&
            now_ms >= sender.timer_start &&
            now_ms - sender.timer_start >= sender.timeout_ms) {

            if (sender.base < sender.nextseqnum) {
                Packet retransmissions[MAX_WINDOW];
                size_t retransmission_count = 0U;

                TEST_ASSERT_EQUAL_INT(
                    0,
                    senderHandleTimeout(&sender, now_ms,
                                        retransmissions,
                                        &retransmission_count));

                for (size_t i = 0U;
                     i < retransmission_count; i++) {
                    TEST_ASSERT_EQUAL_INT(
                        0,
                        testQueueEncodedPacket(&data_channel,
                                       &retransmissions[i],
                                       &data_faults));
                }
            } else if (sender.fin_sent && !sender.fin_acked) {
                /*
                 * FIN is not included in senderHandleTimeout's DATA
                 * retransmission window, so retry it separately.
                 */
                TEST_ASSERT_EQUAL_INT(
                    0,
                    testQueueEncodedPacket(&data_channel,
                                   &sender.fin, &data_faults));

                sender.timer_start = now_ms;
                sender.timer_running = 1;
            }
        }
    }

    /* Fail explicitly if the transfer did not complete. */
    TEST_ASSERT_TRUE_MESSAGE(
        senderFinished(&sender),
        "Transfer did not complete before the step limit");

    TEST_ASSERT_TRUE(receiver.finished);

    /* Every source byte must have been delivered, exactly once. */
    TEST_ASSERT_EQUAL_UINT(sizeof(source), source_offset);
    TEST_ASSERT_EQUAL_UINT(sizeof(source), destination_length);

    TEST_ASSERT_EQUAL_MEMORY(
        source, destination, sizeof(source));

    /*
     * With this fixed seed and sufficiently many packets, the test
     * should exercise each of the three fault types in both directions.
     */
    TEST_ASSERT_TRUE(data_faults.dropped > 0U);
    TEST_ASSERT_TRUE(data_faults.corrupted > 0U);
    TEST_ASSERT_TRUE(data_faults.duplicated > 0U);

    TEST_ASSERT_TRUE(ack_faults.dropped > 0U);
    TEST_ASSERT_TRUE(ack_faults.corrupted > 0U);
    TEST_ASSERT_TRUE(ack_faults.duplicated > 0U);
}




 

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_packetChecksum_empty);
  RUN_TEST(test_packetChecksum_singleByte);
  RUN_TEST(test_packetChecksum_twoBytes);
  RUN_TEST(test_packetChecksum_fourBytes);
  RUN_TEST(test_packetChecksum_oddLength);
  RUN_TEST(test_packetChecksum_oddLength);

    RUN_TEST(test_makeDataPacket_validPacket);
    RUN_TEST(test_makeDataPacket_emptyPayload);
    RUN_TEST(test_makeDataPacket_maxPayload);
    RUN_TEST(test_makeDataPacket_nullPacket);
    RUN_TEST(test_makeDataPacket_nullDataWithNonzeroLength);
    RUN_TEST(test_makeDataPacket_payloadTooLarge);
    RUN_TEST(test_makeDataPacket_zeroLengthWithNonNullData);

    RUN_TEST(test_makeAckPacket_validPacket);
    RUN_TEST(test_makeAckPacket_zeroSequenceNumber);
    RUN_TEST(test_makeAckPacket_maxSequenceNumber);
    RUN_TEST(test_makeAckPacket_nullPacket);

    RUN_TEST(test_makeFinPacket_validPacket);
    RUN_TEST(test_makeFinPacket_zeroSequenceNumber);
    RUN_TEST(test_makeFinPacket_maxSequenceNumber);
    RUN_TEST(test_makeFinPacket_nullPacket);

    RUN_TEST(test_encodePacket_dataPacket);
    RUN_TEST(test_encodePacket_ackPacket);
    RUN_TEST(test_encodePacket_finPacket);
    RUN_TEST(test_encodePacket_emptyDataPacket);
    RUN_TEST(test_encodePacket_maxPayload);
    RUN_TEST(test_encodePacket_nullPacket);
    RUN_TEST(test_encodePacket_nullBuffer);
    RUN_TEST(test_encodePacket_nullPacketSize);
    RUN_TEST(test_encodePacket_bufferTooSmall);
    RUN_TEST(test_encodePacket_invalidType);
    RUN_TEST(test_encodePacket_ackWithPayload);
    RUN_TEST(test_encodePacket_finWithPayload);
    RUN_TEST(test_encodePacket_payloadTooLarge);

    RUN_TEST(test_decodePacket_validDataPacket);
    RUN_TEST(test_decodePacket_validAckPacket);
    RUN_TEST(test_decodePacket_validFinPacket);
    RUN_TEST(test_decodePacket_emptyDataPacket);
    RUN_TEST(test_decodePacket_maxPayload);
    RUN_TEST(test_decodePacket_nullBuffer);
    RUN_TEST(test_decodePacket_nullPacket);
    RUN_TEST(test_decodePacket_tooShort);
    RUN_TEST(test_decodePacket_tooLarge);
    RUN_TEST(test_decodePacket_invalidType);
    RUN_TEST(test_decodePacket_nonzeroReservedByte);
    RUN_TEST(test_decodePacket_lengthMismatch);
    RUN_TEST(test_decodePacket_corruptedPayload);
    RUN_TEST(test_decodePacket_corruptedSequenceNumber);
    RUN_TEST(test_decodePacket_ackWithPayload);
    RUN_TEST(test_decodePacket_finWithPayload);

    RUN_TEST(test_senderInit_validArguments);
    RUN_TEST(test_senderInit_minimumWindow);
    RUN_TEST(test_senderInit_maximumWindow);
    RUN_TEST(test_senderInit_nullPointer);
    RUN_TEST(test_senderInit_zeroWindow);
    RUN_TEST(test_senderInit_windowTooLarge);
    RUN_TEST(test_senderInit_zeroTimeout);
    RUN_TEST(test_senderInit_resetsState);

    RUN_TEST(test_senderSendData_firstPacket);
    RUN_TEST(test_senderSendData_storesPacket);
    RUN_TEST(test_senderSendData_multiplePackets);
    RUN_TEST(test_senderSendData_nullSender);
    RUN_TEST(test_senderSendData_nullData);
    RUN_TEST(test_senderSendData_nullPacket);
    RUN_TEST(test_senderSendData_zeroLength);
    RUN_TEST(test_senderSendData_payloadTooLarge);
    RUN_TEST(test_senderSendData_fullWindow);
    RUN_TEST(test_senderSendData_afterFinSent);
    RUN_TEST(test_senderSendData_afterFinAcked);

    RUN_TEST(test_senderHandleAck_advancesBase);
    RUN_TEST(test_senderHandleAck_duplicateAck);
    RUN_TEST(test_senderHandleAck_staleAck);
    RUN_TEST(test_senderHandleAck_ackBeyondNext);
    RUN_TEST(test_senderHandleAck_allDataAcknowledged);
    RUN_TEST(test_senderHandleAck_noOutstandingData);
    RUN_TEST(test_senderHandleAck_nullSender);
    RUN_TEST(test_senderHandleAck_resetsTimeoutCount);

    RUN_TEST(test_senderHandleTimeout_retransmitsAllOutstanding);
    RUN_TEST(test_senderHandleTimeout_retransmitsUnacknowledgedOnly);
    RUN_TEST(test_senderHandleTimeout_restartsTimer);
    RUN_TEST(test_senderHandleTimeout_incrementsTimeoutCount);
    RUN_TEST(test_senderHandleTimeout_timerNotExpired);
    RUN_TEST(test_senderHandleTimeout_timerNotRunning);
    RUN_TEST(test_senderHandleTimeout_nullSender);
    RUN_TEST(test_senderHandleTimeout_nullPackets);
    RUN_TEST(test_senderHandleTimeout_nullCount);
    RUN_TEST(test_senderHandleTimeout_timeWentBackward);
    RUN_TEST(test_senderHandleTimeout_exactExpiration);
    RUN_TEST(test_senderHandleTimeout_noOutstandingPackets);
    RUN_TEST(test_senderHandleTimeout_exceedsConfiguredWindow);


    RUN_TEST(test_getTimeMs_returns_nonzero);
    RUN_TEST(test_getTimeMs_is_monotonic);
    RUN_TEST(test_getTimeMs_does_not_decrease_over_time);
    RUN_TEST(test_getTimeMs_reasonable_timestamp);

    RUN_TEST(test_senderTimerDue_null_sender);
    RUN_TEST(test_senderTimerDue_null_remaining);
    RUN_TEST(test_senderTimerDue_timer_not_running);
    RUN_TEST(test_senderTimerDue_full_timeout_remaining);
    RUN_TEST(test_senderTimerDue_partial_timeout_remaining);
    RUN_TEST(test_senderTimerDue_timer_expired);
    RUN_TEST(test_senderTimerDue_timer_expired_long_ago);
    RUN_TEST(test_senderTimerDue_time_before_timer_start);
    RUN_TEST(test_senderTimerDue_one_ms_before_expiration);
    RUN_TEST(test_senderTimerDue_zero_timeout);

    RUN_TEST(test_senderSendFin_null_sender);
    RUN_TEST(test_senderSendFin_null_packet);
    RUN_TEST(test_senderSendFin_unacknowledged_data);
    RUN_TEST(test_senderSendFin_success);
    RUN_TEST(test_senderSendFin_empty_file);
    RUN_TEST(test_senderSendFin_already_sent);
    RUN_TEST(test_senderSendFin_does_not_modify_state_on_failure);

    RUN_TEST(test_senderHandleFinAck_null_sender);
    RUN_TEST(test_senderHandleFinAck_fin_not_sent);
    RUN_TEST(test_senderHandleFinAck_already_acknowledged);
    RUN_TEST(test_senderHandleFinAck_wrong_ack_too_small);
    RUN_TEST(test_senderHandleFinAck_wrong_ack_too_large);
    RUN_TEST(test_senderHandleFinAck_success);
    RUN_TEST(test_senderHandleFinAck_empty_file);
    RUN_TEST(test_senderHandleFinAck_timer_already_stopped);

    RUN_TEST(test_senderFinished_null_sender);
    RUN_TEST(test_senderFinished_no_packets_sent);
    RUN_TEST(test_senderFinished_data_not_fully_acknowledged);
    RUN_TEST(test_senderFinished_fin_not_sent);
    RUN_TEST(test_senderFinished_fin_not_acknowledged);
    RUN_TEST(test_senderFinished_success);
    RUN_TEST(test_senderFinished_empty_file_complete);
    RUN_TEST(test_senderFinished_data_acknowledged_but_fin_not_sent);

    RUN_TEST(test_receiverInit_null_receiver);
    RUN_TEST(test_receiverInit_success);
    RUN_TEST(test_receiverInit_resets_existing_state);
    RUN_TEST(test_receiverInit_sets_activity_timestamp);

    RUN_TEST(test_receiverHandleData_expectedPacket);
    RUN_TEST(test_receiverHandleData_multipleInOrderPackets);
    RUN_TEST(test_receiverHandleData_duplicatePacket);
    RUN_TEST(test_receiverHandleData_outOfOrderPacket);
    RUN_TEST(test_receiverHandleData_missingPacket);
    RUN_TEST(test_receiverHandleData_zeroLengthPayload);
    RUN_TEST(test_receiverHandleData_maxPayload);
    RUN_TEST(test_receiverHandleData_nullReceiver);
    RUN_TEST(test_receiverHandleData_nullPacket);
    RUN_TEST(test_receiverHandleData_nullAck);
    RUN_TEST(test_receiverHandleData_nonDataPacket);
    RUN_TEST(test_receiverHandleData_oversizedPayload);
    RUN_TEST(test_receiverHandleData_receiverFinished);
    RUN_TEST(test_receiverHandleData_repeatedOutOfOrderPackets);    

    RUN_TEST(test_receiverHandleFin_nullReceiver);
    RUN_TEST(test_receiverHandleFin_nullPacket);
    RUN_TEST(test_receiverHandleFin_nullAck);
    RUN_TEST(test_receiverHandleFin_nonFinPacket);
    RUN_TEST(test_receiverHandleFin_nonzeroPayload);
    RUN_TEST(test_receiverHandleFin_expectedSequence);
    RUN_TEST(test_receiverHandleFin_emptyFile);
    RUN_TEST(test_receiverHandleFin_outOfOrderSequence);
    RUN_TEST(test_receiverHandleFin_earlyFin);
    RUN_TEST(test_receiverHandleFin_duplicateFin);
    RUN_TEST(test_receiverHandleFin_unexpectedFinAfterCompletion);
    RUN_TEST(test_receiverHandleFin_activityTimestamp);

    RUN_TEST(test_receiverTimerExpired_nullReceiver);
    RUN_TEST(test_receiverTimerExpired_noElapsedTime);
    RUN_TEST(test_receiverTimerExpired_lessThanTimeout);
    RUN_TEST(test_receiverTimerExpired_exactlyAtTimeout);
    RUN_TEST(test_receiverTimerExpired_pastTimeout);
    RUN_TEST(test_receiverTimerExpired_timeBeforeLastActivity);
    RUN_TEST(test_receiverTimerExpired_zeroLastActivity);
    RUN_TEST(test_receiverTimerExpired_largeTimestamps);
    RUN_TEST(test_receiverTimerExpired_doesNotModifyState);

    RUN_TEST(test_resolveAddr_validIPv4);
    RUN_TEST(test_resolveAddr_validHostname);
    RUN_TEST(test_resolveAddr_invalidHostname);
    RUN_TEST(test_resolveAddr_nullHost);
    RUN_TEST(test_resolveAddr_nullPort);
    RUN_TEST(test_resolveAddr_nullResults);
    RUN_TEST(test_resolveAddr_failureClearsResults);
    RUN_TEST(test_resolveAddr_numericService);

    RUN_TEST(test_createSocket_validUDPIPv4);
    RUN_TEST(test_createSocket_validUDPIPv6);
    RUN_TEST(test_createSocket_nullAddress);
    RUN_TEST(test_createSocket_invalidFamily);
    RUN_TEST(test_createSocket_invalidSocketType);
    RUN_TEST(test_createSocket_invalidProtocol);

    RUN_TEST(test_registerReceiver_success);
    RUN_TEST(test_registerReceiver_relayError);
    RUN_TEST(test_registerReceiver_nullRelay);
    RUN_TEST(test_registerReceiver_nullSession);
    RUN_TEST(test_registerReceiver_invalidSessionCharacter);
    RUN_TEST(test_registerReceiver_sessionTooLong);
    RUN_TEST(test_registerReceiver_invalidSocket);
    RUN_TEST(test_registerReceiver_ipv6_address_check);
    RUN_TEST(test_registerReceiver_fails_after_five_attempts);

    RUN_TEST(test_registerSender_success);
    RUN_TEST(test_registerSender_relayError);
    RUN_TEST(test_registerSender_nullRelay);
    RUN_TEST(test_registerSender_nullSession);
    RUN_TEST(test_registerSender_invalidSession);
    RUN_TEST(test_registerSender_sessionTooLong);
    RUN_TEST(test_registerSender_negativeLoss);
    RUN_TEST(test_registerSender_corruptTooHigh);
    RUN_TEST(test_registerSender_dupTooHigh);
    RUN_TEST(test_registerSender_invalidSocket);
    RUN_TEST(test_registerSender_ipv6_success);
    RUN_TEST(test_registerSender_wrongRelayAddress);

    RUN_TEST(test_runSender_nullRelay_returnsFailure);
    RUN_TEST(test_runSender_nullFilename_returnsFailure);
    RUN_TEST(test_runSender_zeroWindow_returnsFailure);
    RUN_TEST(test_runSender_windowAboveMaximum_returnsFailure);
    RUN_TEST(test_runSender_zeroTimeout_returnsFailure);
    RUN_TEST(test_runSender_missingFile_returnsFailure);
    RUN_TEST(test_runSender_invalidPort_returnsFailure);    
    RUN_TEST(test_runSender_relayRejectsRegistration_returnsFailure);
    
    RUN_TEST(test_runReceiver_validDataAndFin);
    RUN_TEST(test_runReceiver_emptyFile);
    RUN_TEST(test_runReceiver_discardsCorruptedPacket);
    RUN_TEST(test_runReceiver_outOfOrderData);  

    RUN_TEST(test_completeTransfer_withLossCorruptionAndDuplication);

  return UNITY_END();
}
