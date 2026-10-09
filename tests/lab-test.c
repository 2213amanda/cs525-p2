#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include <string.h>


void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void) {
  printf("Tearing down tests...\n");
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
  return UNITY_END();
}
