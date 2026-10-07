#ifndef LAB_H
#define LAB_H

#define _POSIX_C_SOURCE 200112L

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_PAYLOAD 1024
#define HEADER_SIZE 10
#define MAX_PACKET_SIZE 1034
#define MAX_WINDOW 64

typedef enum {
    PACKET_DATA = 0,
    PACKET_ACK = 1,
    PACKET_FIN = 2
} PacketType;

typedef struct {
    uint8_t type;
    uint32_t seq;
    uint16_t length;
    uint8_t payload[MAX_PAYLOAD];
} Packet;

typedef struct {
    uint32_t base;
    uint32_t nextseqnum;
    uint32_t window;
    uint32_t timeout_ms;
    uint32_t consecutive_timeouts;
    uint64_t timer_start;
    int timer_running;

    Packet packets[MAX_WINDOW];
    Packet fin;
    int fin_sent;
    int fin_acked;
} SenderState;

typedef struct {
    uint32_t expectedseqnum;
    int finished;
    uint64_t last_activity;
} ReceiverState;

/* Packet functions */

/** * @brief calculates the internet checksum of a packet buffer
*
* Treats the buffer as 16-bit words and calculates the checksum.
* If the buffer has an odd number of bytes a zero byte is added for calculation.
*
* @param data buffer containing the data to checksum
* @param length the number of bytes in the buffer
* @return the 16-bit internet checksum
*/
uint16_t packetChecksum(const uint8_t *data, size_t length);

/** * @brief encodes a packet to be able to send over the network
*
* Writes the packet header and payload into the buffer using network byte 
* order for all multi-byte fields. The checksum is calculated and placed
* inot the encoded packet.
*
* @param packet the packet structure to encode 
* @param buffer the buffer that will contain the encoded packet
* @param buffer_size size of the output buffer
* @param packet_size pointer to where the encoded packet will be stored
* @return 0 on success nonzero on failure
 */
int encodePacket(const Packet *packet, uint8_t *buffer, size_t buffer_size, size_t *packet_size);

/** *@brief decodes and validates a recieved packet
*
* Checks the packet size, type, reserved field, payload length, and 
* checksum before copying decoded fields into packet structure.
* Invalid packets are rejected without producing a valid packet.
*
* @param buffer the buffer containing the recieved datagram
* @param buffer_size number of bytes actually received
* @param packet the packet structure where the decoded packet will be stored
* @return 0 on success nonzero if the packet is invalid
 */
int decodePacket(const uint8_t *buffer, size_t buffer_size, Packet *packet);

/** *@brief creates a data packet
*
* Initializes a packet structure as a data packet with the 
* specified sequence number and payload
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the data packet
* @param data the payload to place in the datapacket
* @param length the number of payload bytes
* @return 0 on success, nonzero if the arguments or payload length are invalid
 */
int makeDataPacket(Packet *packet, uint32_t seq, const uint8_t *data, uint16_t length);

/** @brief creates an ack packet
*
* Initializes a packet strucure as an ack containing the specified
* cumulative acknowledgement sequence number
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the next packet expected by reciever
* @return 0 on success, nonzero of the packet pointer is invalid
 */
int makeAckPacket(Packet *packet, uint32_t seq);

/** *@brief creates a fin packet
*
* Initializes a packet structure as a fin packet with
* the specified sequence number and payload
*
* @param packet the packet structure to initialize
* @param seq the sequence number of the fin packet
* @return 0 on success, nonzero if the packet pointer is invalid
*/
int makeFinPacket(Packet *packet, uint32_t seq);



/* Sender state machine */

/** *@brief initializes the sender go-back-n state machine
*
* Sets the sender's base and next sequence number to zero, initializes
* the configured window and timeout values, clears the retransmission
* state and prepares the sender to trasmit data packets
*
* @param sender the sender state structure to init
* @param window the max number of packets allowed in the send window
* @param timeout_ms the retransmission timeout in ms
* @return 0 on success nonzero if args are invalid
 */
int senderInit(SenderState *sender, uint32_t window, uint32_t timeout_ms);

/** *@brief processes new data for transmission by snder
*
* creates the next data packet when a space is available in the sender's
* window, stores a copy for possible retransmission, advances nextseqnum,
* and starts the timer when the first unacked packet enters the window.
* the function does not send the packet itself
*
* @param sender the sender state machine
* @param data payload data for the new data packet
* @param length the number of payload bites
* @param now_ms current time in ms
* @param packet the structure where the packet to transmit will be stored
* @return 0 on success nonzero if window is full or data is invalid
 */
int senderSendData(SenderState *sender, const uint8_t *data, uint16_t length, uint64_t now_ms, Packet *packet);

/** * @brief processes a cumulative ack recieved by the sender
*
* advances base when the ack acknowledges packets that are previously
* unacknowledged. duplicate or older acks are ignored. the retransmission 
* timer is stopped when all outstanding packets are acknowledged and is
* restarted when unacknowledged packets remain.
*
* @param sender the sender state machine
* @param ack_seq the sequence number contained in the recieved ack
* @param now_ms the current time in ms
* @return 0 if ack was processed, nonzero on error
 */
int senderHandleAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms);

/** * @brief processes a sender retransmission timeout
*
* resets the retransmission timer and produces copies of every unacknowledged packet
* from base through nextseqnum - 1 so that the caller can retransmit the entire go-back-n window
*
* @param sender the sender state machine
* @param now_ms the current time in ms
* @param packets array where the packets requiring restransmission will be stored
* @param count pointer where the number of packets to retransmit will be stored
* @return 0 on success, nonzero if the sender must give up
 */
int senderHandleTimeout(SenderState *sender, uint64_t now_ms, Packet *packets, size_t *count);

/** * @brief determines how much time remains on the sender's timer
*
* calculates the time remaining before the retransmission timer expires
*
* @param sender the sender state machine
* @param now_ms curren time in ms
* @param remaining_ms pointer where the remaining timer duration will be stored
* @return 0 if a timer is active, nonzero if no timer is running
 */
int senderTimerDue(const SenderState *sender, uint64_t now_ms, uint64_t *remaining_ms);

/** * @brief creates the sender's fin packet after all data is acknowledged
*
* creates the fin packet using the next sequence number, records that fin has been
* sent, and starts the retransmission timer
*
* @param sender the sender state machine
* @param now_ms the current time in ms
* @param packet the structure where the fin packet will be stored
* @return 0 on success, nonzero if data packets remain unacknowledged
 */
int senderSendFin(SenderState *sender, uint64_t now_ms, Packet *packet);

/** * @brief process the ack for the sender's fin packet
*
* marks the fin as acknowledged and stops the retransmission timer when the 
* recieved ack acknowledges the fin
*
* @param sender the sender state machine
* @param ack_seq sequence number contained in the recieved ack
* @param now_ms the current time in ms
* @return 0 if the fin was acknowledged, nonzero otherwise
 */
int senderHandleFinAck(SenderState *sender, uint32_t ack_seq, uint64_t now_ms);

/** * @brief determines whether the sender has completed the transfer
*
* checks whether all data packets and the fin packet has been acknowledged
*
* @param sender the sender state machine
* @return nonzero if the transfer is complete, otherwise zero
 */
int senderFinished(const SenderState *sender);



/* Reciever state machine */

/** * @brief initializes the receiver go-back-n state machine
*
* sets the receiver's expected sequence number to 0 and initializes
* its transfer and activity state
*
* @param receiver the receiver state machine to initialize
* @param return 0 for success, nonzero value if receiver is null
 */
int receiverInit(ReceiverState *receiver);

/** * @brief processes a data packet recieved by the receiver
*
* accepts and delivers the packet when its sequence number matches expectedseqnum
* an accepted packet advances expectedseqnum and produces a cumulative ack. duplicate
* and out of order packets are discarded and cause an ack for the current expected
* sequence number to be produced
*
* @param receiver the receiver state machine
* @param packet the recieved data packet
* @param now_ms the current time in ms
* @param ack the structure where the ack to send will be stored
 */
int receiverHandleData(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack);
 
/** * @brief processes a fin packet received by the reciever 
*
* accepts the fin when its sequence number matches expetedseqnum, advances
* the expected sequence number, creates the final ack, and marks the reciever
* as finished. a repeated fin is answered with the same cumulative ack
* during the reciever's linger period
*
* @param reciever the reciever state machine
* @param packet the recieved fin packet
* @param now_ms the current time in ms
* @param ack structure where the ack to send will be stored
* @return 0 on success, nonzero on error
 */
int receiverHandleFin(ReceiverState *receiver, const Packet *packet, uint64_t now_ms, Packet *ack);

/** * @brief determines whether the recevier has exceeded its inactivity timeout
*
* compared the supplied current time with the receiver's last activity time to 
* determine whether 30 seconds have elapsed without a valid packet
*
* @param receiver the receiver state machine
* @param now_ms the curren time in ms
* @return nonzero if the receiver has timed out, otherwise zero 
 */
int receiverTimerExpired(const ReceiverState *receiver, uint64_t now_ms);



/* I/O */

/** * @brief gets the current time in milliseconds
*
* uses CLOCK_MONOTONIC to botain a clock value that does not change when the 
* system wall clock is adjusted
*
* @return current time in ms
 */
uint64_t getTimeMs(void);

/** * @brief resolves a relay hostname or address into socket addresses
*
* uses getaddrinfo to resolve the supplied relay host and port so that the caller
* can connect without assuming the relay uses a particular address format
*
* @param host the relay hostname or address
* @param port the relay port as a string
* @param results pointer where the resulting address list will be stored
* @return 0 on success, nonzero if address resolution fails
 */
int resolveAddr(const char *host, const char *port, struct addrinfo **results);

/** * @brief creates a udp socket for a resolved address
*
* creates a udp socket using the address family and protocol information contained 
* in the supplied addrinfo structure
*
* @param addr resolved address information used to create the socket
* @return socket file descriptor on success, or -1 on failure
 */
int createSocket(const struct addrinfo *addr);

/** * @brief registers the reciever with the relay
*
* sends the receiver hello message to the relay and waits for an ok response.
* if no response is attempted within one second the hello retried up to 5 times
*
* @param sockfd UDP socket used for the transfer
* @param relay address of the relay
* @param relay_len length of the relay address structure
* @param session the session name shared with the sender
* @return 0 when registration succeeds, nonzero when registration fails
 */
int registerReceiver(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session);

/** * @brief registers the sender with the sender with the relay
* 
* sends the sender hello message containing the session name and packet loss,
* corruption, and duplication probabilities, then waits for an OK response. if no 
* response is recieved within one second, hello is retried up to 5 times
*
* @param sockfd UDP socket used for the transfer
* @param relay address of the relay
* @param relay_len the legnth of the relay address structure
* @param session the session name shared with the receiver
* @param loss probability that the relay drops a packet
* @param corrupt the probability that the relay corrupts a packet
* @param dup the probability that the relay duplicates a packet
* @return 0 when registration succeeds, nonzeor when it fails
 */
int registerSender(int sockfd, const struct sockaddr *relay, socklen_t relay_len, const char *session, double loss, double corrupt, double dup);

/** * @brief runs the complete sender transfer
*
* opens the input file, registers the sender with the relay, drives the sender
* state machine using UDP and poll, transmits the data and fin packets, handles 
* cumulative acks and retransmission timeouts, and closes all resources when 
* the transfer completes or fails
*
* @param relay relay hostname or address
* @param port relay udp port
* @param session session name shared with the reciever
* @param filename input file to transmit
* @param window go-back-n window size in packets
* @param timeout_ms retransmission timeout in ms
* @param loss packet loss probability requested from the relay
* @param corrupt packet corruption probability requested form the relay
* @param dup packet duplication probability requested from the relay
* @return 0 on successful transfer, nonzero on network or transfer failure
 */
int runSender(const char *relay, const char *port, const char *session, const char *filename, uint32_t window, uint32_t timeout_ms, double loss, 
            double corrupt, double dup);

/** * @brief runs the complete reciever transfer
*
* opens the output file, registers the receiver with the relay, recieves and 
* validates packets, drives the receiver state machine, writes accepted data payloads
* in order, sends cumulative acks, and lingers after fin to handle retransmitted fin packets
*
* @param relay the relay hostname or address
* @param port relay udp port
* @param session session name shared with the sender
* @param filename output file where recieved data will be written
* @return 0 on succesful transfer, nonzero on network or transfer failure
 */            
int runReceiver(const char *relay, const char *port, const char *session, const char *filename);

/* Command line */

/** * @brief pritns the program's command-line usage info
*
* prints the required send and recieve command formats along with the available command line options and their default values
*
* @return none
 */
void printUsage(void);

#endif 