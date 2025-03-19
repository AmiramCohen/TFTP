/**
 * @file common.c
 * @brief Implements shared utilities for TFTP client and server.
 *
 * This file contains functions for handling sockets, sending/receiving packets,
 * validating ACKs, and processing file transfers.
 */

# include "common.h"

/**
 * @brief Prints the contents of a buffer in hexadecimal format.
 * @param buffer Pointer to the buffer.
 * @param length Length of the buffer.
 */
void print_buffer(unsigned char *buffer, size_t length) 
{
    printf("Buffer Content (%zu bytes):\n", length);
    
    for (size_t i = 0; i < length; i++) 
    {
        printf("%02X ", buffer[i]);
        
        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    if (length % 16 != 0)
        printf("\n");
}

/**
 * @brief Prints an error message received.
 * @param response The error response buffer.
 * @param length Length of the response buffer.
 */
void print_error(unsigned char *response, size_t length) 
{
    if (!response || length < TFTP_MIN_ERROR_PACKET) 
    {
        fprintf(stderr, ANSI_RED "Invalid error packet received\n" ANSI_RST);
        return;
    }

    int error_code = (response[2] << 8) | response[3];
    
    if (length < (TFTP_MIN_ERROR_PACKET + 1)) 
    {
        fprintf(stderr, ANSI_RED "Error packet too small to contain message\n" ANSI_RST);
        return;
    }

    char *error_message = (char *)(response + TFTP_OPCODE_SIZE + TFTP_ERROR_CODE_SIZE);

    printf(ANSI_RED "Error %d: %s\n" ANSI_RST, error_code, error_message);
}

/**
 * @brief Sets the timeout for a socket.
 * @param socket Pointer to the TFTPSocket struct.
 * @param timeout Timeout duration in seconds.
 * @return true on success, false on failure.
 */
bool set_socket_timeout(TFTPSocket *socket, int timeout) 
{
    if (!socket) return false;

    struct timeval tv = { .tv_sec = timeout, .tv_usec = 0 };
    if (setsockopt(socket->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) 
    {
        fprintf(stderr, ANSI_RED "Error: Failed to set socket timeout\n" ANSI_RST);
        return false;
    }

    return true;
}

/**
 * @brief Sends a data packet to the server or client.
 * @param socket Pointer to the TFTPSocket struct.
 * @param buffer The data buffer.
 * @param bytes_read The number of bytes read from the file.
 * @param block_number The block number being sent.
 * @return true if the packet was sent successfully, false otherwise.
 */
bool send_data_packet(TFTPSocket *socket, unsigned char *buffer, size_t bytes_read, int block_number) 
{
    buffer[0] = 0;  
    buffer[1] = TFTP_DATA;  
    buffer[2] = block_number >> 8;  
    buffer[3] = block_number & 0xFF;  

    /* 
        Uncomment this for debug:
        
        fprintf(stdout, ANSI_BLUE "Sending ACK for block %d\n" ANSI_RST, block_number);
    */

    if (sendto(socket->sockfd, buffer, bytes_read + TFTP_DATA_HEADER_SIZE, 0,
        (struct sockaddr *)&socket->addr, socket->addr_len) < 0)
        {
            fprintf(stderr, ANSI_RED "Error sending ACK\n" ANSI_RST);
            return false;
        }

    return true;
}

/**
 * @brief Writes received data to a file.
 *
 * This function ensures that the received data is correctly written to the file.
 *
 * @param buffer The buffer containing file data.
 * @param len Number of bytes to write.
 * @param file Pointer to the file where data will be written.
 * @return true if writing was successful, false otherwise.
 */
bool write_to_file(unsigned char* buffer, ssize_t len, FILE* file) 
{
    if (len < 0) return false;

    if (fwrite(buffer, 1, (size_t) len, file) != (size_t) len) 
    {
        if (errno == ENOSPC)
            send_error_packet(NULL, ERROR_DISK_FULL, buffer, NULL);
        else if (errno == EACCES || errno == EPERM)
            send_error_packet(NULL, ERROR_ACCESS_VIOLATION, buffer, NULL);
        else
            send_error_packet(NULL, ERROR_UNDEFINED, buffer, NULL);

        print_error(buffer, TFTP_MAX_PACKET_SIZE);
        return false;
    }

    return true;
}

/**
 * @brief Sends a TFTP error packet.
 *
 * Constructs and sends an error packet with a specified error code and message.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param error_code The TFTP error code.
 * @param buffer The buffer used to store the error packet.
 * @param custom_message An optional custom error message.
 */
void send_error_packet(TFTPSocket *socket, TFTPError error_code, unsigned char *buffer, const char *custom_message)
{
    if (!socket || !buffer) return;

    buffer[0] = 0;
    buffer[1] = TFTP_ERROR;
    buffer[2] = (error_code >> 8) & 0xFF;
    buffer[3] = error_code & 0xFF;

    const char *error_message = tftp_error_message(error_code);

    if (custom_message)
    {
        snprintf((char *)buffer + TFTP_DATA_HEADER_SIZE, 
            TFTP_MAX_PACKET_SIZE - TFTP_DATA_HEADER_SIZE, 
            "%s - %s", error_message, custom_message);
    }
    else
    {
        snprintf((char *)buffer + TFTP_DATA_HEADER_SIZE, 
            TFTP_MAX_PACKET_SIZE - TFTP_DATA_HEADER_SIZE, 
            "%s", error_message);
    }

    ssize_t sent_len = sendto(socket->sockfd, buffer, 
        TFTP_DATA_HEADER_SIZE + strlen((char *)buffer + TFTP_DATA_HEADER_SIZE) + 1, 
        0, (struct sockaddr *)&socket->addr, socket->addr_len);

    if (sent_len < 0)
        fprintf(stderr, ANSI_RED "Error sending error packet\n" ANSI_RST);
}

/**
 * @brief Retrieves a human-readable TFTP error message.
 *
 * Returns a descriptive error message based on the provided error code.
 *
 * @param error_code The TFTP error code.
 * @return Corresponding error message string.
 */
const char *tftp_error_message(TFTPError error_code) 
{
    switch (error_code) 
    {
        case ERROR_FILE_NOT_FOUND:      return "File not found";
        case ERROR_ACCESS_VIOLATION:    return "Access violation";
        case ERROR_DISK_FULL:           return "Disk full or allocation exceeded";
        case ERROR_ILLEGAL_OPERATION:   return "Illegal TFTP operation";
        case ERROR_UNKNOWN_TID:         return "Unknown transfer ID";
        case ERROR_FILE_EXISTS:         return "File already exists";
        case ERROR_NO_SUCH_USER:        return "No such user";
        default:                        return "Undefined error";
    }
}

/**
 * @brief Checks if an acknowledgment (ACK) packet is valid.
 *
 * Validates whether the received ACK matches the expected block number.
 *
 * @param buffer The received buffer.
 * @param block_number The expected block number.
 * @return true if the ACK is valid, false otherwise.
 */
bool check_ack(unsigned char* buffer, int block_number)
{
    int received_block = (buffer[2] << 8) | buffer[3];

    if ((buffer[2] == (block_number >> 8)) && (buffer[3] == (block_number & 0xFF)))
        return true;
    
    if (received_block != block_number)
    {
        /* 
            Uncomment this for debug:
        
            fprintf(stderr, ANSI_RED "ACK received for block %d, but expected block %d\n" ANSI_RST, received_block, block_number);
        */
        
        return false;
    }

    return false;
}

/**
 * @brief Sends an acknowledgment (ACK) packet.
 *
 * Constructs and sends an acknowledgment for the specified block number.
 *
 * @param socket Pointer to the TFTPSocket structure.
 * @param buffer The buffer used for sending the ACK.
 * @param block_number The block number being acknowledged.
 * @return true if ACK was sent successfully, false otherwise.
 */
bool send_ack(TFTPSocket *socket, unsigned char* buffer, int block_number) 
{
    buffer[0] = 0;
    buffer[1] = TFTP_ACK;
    buffer[2] = block_number >> 8;  
    buffer[3] = block_number & 0xFF; 

    /* 
        Uncomment this for debug:
        
        fprintf(stdout, ANSI_BLUE "Sending ACK for block %d\n" ANSI_RST, block_number);
    */

    ssize_t sent_len = sendto(socket->sockfd, buffer, TFTP_DATA_HEADER_SIZE, 0, 
        (struct sockaddr *)&socket->addr, socket->addr_len);

    if (sent_len < 0) 
    {
        fprintf(stderr, ANSI_RED "Error sending ACK for block %d: %s\n" ANSI_RST, block_number, strerror(errno));
        return false;
    }

    return true;
}

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
int get_response(TFTPSocket *socket, unsigned char* buffer) 
{   
    ssize_t len = recvfrom(socket->sockfd, buffer, TFTP_MAX_PACKET_SIZE, 0,
        (struct sockaddr *)&socket->addr, &socket->addr_len);

    if (len < 0) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
            fprintf(stderr, ANSI_RED "Timeout: No response received after %d seconds.\n" ANSI_RST, TFTP_TIMEOUT);
        else
            fprintf(stderr, ANSI_RED "Error receiving response" ANSI_RST);
        return -1;
    }

    /*
        Uncomment this for debug:

        int received_opcode = buffer[1];
        fprintf(stdout, ANSI_GREEN "Received packet: Opcode = %d, Length = %ld\n" ANSI_RST, received_opcode, len);
    */

    switch (buffer[1])
    {
        case TFTP_ACK:      return 1;
        case TFTP_DATA:     return len;
        case TFTP_ERROR:    { print_error(buffer, TFTP_MAX_PACKET_SIZE); return -1; }
        
        default:            { fprintf(stderr, ANSI_RED "Unexpected response received\n" ANSI_RST);
                             return -1; }
    }
}

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
 * @param is_upload true if this is an upload request, false for downloads.
 * @param success Pointer to a flag indicating if the operation was successful.
 * @return true if the ACK is valid or retrying is required, false if retries are exhausted.
 */
bool validate_and_retry(TFTPSocket *server, TFTPRequest *request, int *block_number, int *retry, ssize_t bytes_read, bool is_upload, bool *success)
{
    if (!check_ack(request->buffer, *block_number))
    {
        *success = false;
        (*retry)++;

        if (*retry >= TFTP_MAX_RETRIES)  
        {
            fprintf(stderr, ANSI_RED "Failed to %s '%s' after %d retries.\n" ANSI_RST, 
                is_upload ? "upload" : "download", request->filename, TFTP_MAX_RETRIES);
            
            return false;
        }

        fprintf(stderr, ANSI_RED "Invalid ACK for block %d... retrying [%d]\n" ANSI_RST, *block_number, *retry);

        if (is_upload)
        {
            if (fseek(request->filepath, -bytes_read, SEEK_CUR) != 0)
            {
                fprintf(stderr, ANSI_RED "Failed to reset file position\n" ANSI_RST);
                return false;
            }
            send_data_packet(server, request->buffer, bytes_read, *block_number);
        }
        else
            send_ack(server, request->buffer, *block_number);
        return true;  
    }

    if (!is_upload)
        send_ack(server, request->buffer, *block_number);

    *retry = 0;
    (*block_number)++;
    *success = true;
    
    return true;
}

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
bool process_transfer_send(TFTPSocket *socket, TFTPRequest *request, int *block_number, int *retry, bool *success)
{    
    ssize_t bytes_read = fread(request->buffer + TFTP_DATA_HEADER_SIZE, 1, TFTP_MAX_DATA_SIZE, request->filepath);
    if (bytes_read <= 0) 
        return false;
    
    if (!send_data_packet(socket, request->buffer, bytes_read, *block_number))
        return false;
    
    if (get_response(socket, request->buffer) < 0) 
        return false;
            
    if (!validate_and_retry(socket, request, block_number, retry, bytes_read, true, success))
        return false;

    if ((bytes_read < TFTP_MAX_DATA_SIZE) && *success)
    {
        printf(ANSI_GREEN "File '%s' sended successfully!\n" ANSI_RST, request->filename);
        return false; 
    }

    return true;
}

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
bool process_transfer_receive(TFTPSocket *socket, TFTPRequest *request, int *block_number, int *retry, bool *success)
{
    ssize_t len = get_response(socket, request->buffer);
    if (len < 0) return false;
    
    if (!validate_and_retry(socket, request, block_number, retry, 0, false, success))
        return false;  

    if (!write_to_file(request->buffer + TFTP_DATA_HEADER_SIZE,
        len - TFTP_DATA_HEADER_SIZE, request->filepath))
    {
        fprintf(stderr, ANSI_RED "File write failed for '%s'\n" ANSI_RST, request->filename);
        return false;
    }
    
    if ((len < TFTP_MAX_PACKET_SIZE) && *success)
    {
        printf(ANSI_GREEN "File '%s' received successfully!\n" ANSI_RST, request->filename);
        return false;
    }
    
    return true;
}
