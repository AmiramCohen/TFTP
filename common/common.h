/**
 * @file common.h
 * @brief Common utilities and shared functions for TFTP Client & Server.
 *
 * This file contains common structures, error handling, and socket utilities
 * used by both the TFTP client and server implementations.
 */

# ifndef COMMON_H
# define COMMON_H


# include <stdbool.h>
# include <stdio.h>
# include <sys/time.h>
# include <unistd.h>
# include <errno.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <arpa/inet.h>
# include <libgen.h>


// ========================================
//         GENERAL CONFIGURATION
// ========================================
# define TFTP_NUM_ARGUMENTS         4       ///< Number of command-line arguments required
# define TFTP_PORT                  69      ///< Default TFTP port number

# define TFTP_MAX_RETRIES           3       ///< Maximum number of retries for lost packets
# define MAX_FILE_NAME              256     ///< Maximum filename length
# define TFTP_TIMEOUT               5       ///< Timeout duration for resending packets


// ========================================
//          PACKET CONFIGURATION
// ========================================
# define TFTP_OPCODE_SIZE           2       ///< Size of TFTP opcode field
# define TFTP_ERROR_CODE_SIZE       2       ///< Size of TFTP error code field
# define TFTP_MIN_ERROR_PACKET      (TFTP_OPCODE_SIZE + TFTP_ERROR_CODE_SIZE + 1)  
# define TFTP_BLOCK_NUMBER_SIZE     2       ///< Size of TFTP block number field
# define TFTP_DATA_HEADER_SIZE      (TFTP_OPCODE_SIZE + TFTP_BLOCK_NUMBER_SIZE)  
# define TFTP_MAX_DATA_SIZE         512     ///< Maximum data payload per packet 
# define TFTP_MAX_PACKET_SIZE       (TFTP_DATA_HEADER_SIZE + TFTP_MAX_DATA_SIZE)  


// ========================================
//          TFTP OPERATION MODES
// ========================================
# define DOWNLOAD                   "download"
# define UPLOAD                     "upload"
# define DELETE                     "delete"
# define UNSUPPORTED                "unsupported"


// ========================================
//              ANSI COLORS
// ========================================
# define ANSI_RED                   "\e[0;31m"
# define ANSI_GREEN                 "\e[0;32m"
# define ANSI_BLUE                  "\e[0;34m"
# define ANSI_PURPLE                "\e[0;35m"
# define ANSI_CYAN                  "\e[0;36m"
# define ANSI_RST                   "\e[0;37m"


// ========================================
//              ENUMERATIONS
// ========================================
/**
 * @enum TFTPOpcode
 * @brief Represents different TFTP operation codes.
 */
typedef enum { 
    TFTP_RRQ = 1,       ///< Read Request
    TFTP_WRQ,           ///< Write Request
    TFTP_DATA,          ///< Data Packet
    TFTP_ACK,           ///< Acknowledgment Packet
    TFTP_ERROR,         ///< Error Packet 
    TFTP_DRQ            ///< Delete Request
} TFTPOpcode;


/**
 * @enum TFTPError
 * @brief Represents different TFTP error codes.
 */
typedef enum {
    ERROR_UNDEFINED = 0,        ///< Undefined error
    ERROR_FILE_NOT_FOUND,       ///< File not found
    ERROR_ACCESS_VIOLATION,     ///< Access violation
    ERROR_DISK_FULL,            ///< Disk full or allocation exceeded
    ERROR_ILLEGAL_OPERATION,    ///< Illegal TFTP operation      
    ERROR_UNKNOWN_TID,          ///< Unknown transfer ID
    ERROR_FILE_EXISTS,          ///< File already exists
    ERROR_NO_SUCH_USER          ///< No such user
} TFTPError;


// ========================================
//              STRUCTURES
// ========================================
/**
 * @struct TFTPSocket
 * @brief Structure representing a TFTP socket.
 */
typedef struct {
    int sockfd;                     ///< Socket file descriptor
    struct sockaddr_in addr;        ///< Address struct
    socklen_t addr_len;             ///< Address length
} TFTPSocket;


/**
 * @struct TFTPRequest
 * @brief Structure representing a TFTP request.
 */
typedef struct {
    short int opcode;                               ///< TFTP operation code (RRQ, WRQ, DRQ)
    const char *operation;                          ///< Operation type
    char *filename;                                 ///< Requested filename
    FILE *filepath;                                 ///< File pointer
    const char *server_ip;                          ///< Server IP address (for clients)
    int timeout;                                    ///< Timeout for retries
    int block_size;                                 ///< Block size for data transfer
    unsigned char buffer[TFTP_MAX_PACKET_SIZE];     ///< Buffer for packet data
} TFTPRequest;


// ========================================
//          FUNCTION PROTOTYPES
// ========================================

/**
 * @brief Sets the timeout for a socket.
 * @param socket Pointer to the TFTPSocket struct.
 * @param timeout Timeout duration in seconds.
 * @return true on success, false on failure.
 */
bool set_socket_timeout(TFTPSocket *socket, int timeout);

/**
 * @brief Sends a data packet to the server or client.
 * @param socket Pointer to the TFTPSocket struct.
 * @param buffer The data buffer.
 * @param bytes_read The number of bytes read from the file.
 * @param block_number The block number being sent.
 * @return true if the packet was sent successfully, false otherwise.
 */
bool send_data_packet(TFTPSocket *socket, unsigned char *buffer, size_t bytes_read, int block_number);

/**
 * @brief Prints buffer content for debugging.
 * @param buffer Pointer to the buffer.
 * @param length Length of the buffer.
 */
void print_buffer(unsigned char *buffer, size_t length);

/**
 * @brief Prints an error message received from the server.
 * @param response The error response buffer.
 * @param length Length of the response buffer.
 */
void print_error(unsigned char *response, size_t length);

/**
 * @brief Sends a TFTP error packet.
 * @param socket Pointer to the TFTPSocket.
 * @param error_code The TFTP error code.
 * @param buffer The buffer to store the error packet.
 * @param custom_message A custom error message (optional).
 */
void send_error_packet(TFTPSocket *socket, TFTPError error_code, unsigned char *buffer, const char *custom_message);

/**
 * @brief Writes received data to a file.
 * @param buffer The buffer containing file data.
 * @param len Number of bytes to write.
 * @param file Pointer to the file where data will be written.
 * @return true if writing was successful, false otherwise.
 */
bool write_to_file(unsigned char* buffer, ssize_t len, FILE* file);

/**
 * @brief Retrieves a human-readable TFTP error message.
 * @param error_code The TFTP error code.
 * @return Corresponding error message string.
 */
const char *tftp_error_message(TFTPError error_code);

/**
 * @brief Checks if an acknowledgment (ACK) packet is valid.
 * @param buffer The received buffer.
 * @param block_number The expected block number.
 * @return true if valid, false otherwise.
 */
bool check_ack(unsigned char* buffer, int block_number);

/**
 * @brief Sends an acknowledgment (ACK) packet.
 * @param socket Pointer to the TFTPSocket struct.
 * @param buffer The buffer used for sending the ACK.
 * @param block_number The block number being acknowledged.
 * @return true if ACK was sent successfully, false otherwise.
 */
bool send_ack(TFTPSocket *socket, unsigned char* buffer, int block_number);

/**
 * @brief Receives a response from the server or client.
 *
 * Reads data from the socket into a buffer and returns the number of bytes received.
 * Handles timeout and connection errors.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param buffer Buffer where the received response will be stored.
 * @return Number of bytes received, or -1 on error.
 */
int get_response(TFTPSocket *socket, unsigned char* buffer);

/**
 * @brief Validates the acknowledgment packet and retries if necessary.
 *
 * Checks if the received acknowledgment is valid. If not, retries sending the
 * last data block up to TFTP_MAX_RETRIES times.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @param block_number Pointer to the current block number.
 * @param retry Pointer to the retry counter.
 * @param bytes_read Number of bytes read from the file for the current block.
 * @param is_upload true if this is an upload request, false for download.
 * @param success Pointer to a flag indicating if the operation was successful.
 * @return true if the ACK is valid or retrying is required, false if retries are exhausted.
 */
bool validate_and_retry(TFTPSocket *socket, TFTPRequest *request, int *block_number, int *retry, ssize_t bytes_read, bool is_upload, bool *success);

/**
 * @brief Processes and sends data packets for an upload request.
 *
 * Reads a block of data from the file, sends it, and waits for an acknowledgment.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @param block_number Pointer to the current block number.
 * @param retry Pointer to the retry counter.
 * @param success Pointer to a flag indicating if the operation was successful.
 * @return true if more data needs to be sent, false if the transfer is complete or failed.
 */
bool process_transfer_send(TFTPSocket *socket, TFTPRequest *request, int *block_number, int *retry, bool *success);

/**
 * @brief Processes and receives data packets for a download request.
 *
 * Waits for a data packet, writes it to the file, and sends an acknowledgment.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @param block_number Pointer to the current block number.
 * @param retry Pointer to the retry counter.
 * @param success Pointer to a flag indicating if the operation was successful.
 * @return true if more data needs to be received, false if the transfer is complete or failed.
 */
bool process_transfer_receive(TFTPSocket *socket, TFTPRequest *request, int *block_number, int *retry, bool *success);


# endif  // COMMON_H
