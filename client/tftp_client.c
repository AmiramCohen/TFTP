/**
 * @file tftp_client.c
 * @brief Implements TFTP client-side request handling.
 *
 * This file contains functions for sending TFTP requests, handling uploads,
 * downloads, and delete requests.
 */

# include "tftp_client.h"

/**
 * @brief Sends a TFTP request (Read, Write, Delete) to the server.
 *
 * Constructs a request packet containing the operation, filename, and mode.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPReques containing request details.
 * @return true if the request was sent successfully, false otherwise.
 */
bool send_request(TFTPSocket *client, TFTPRequest *request) 
{    
    if (!client || !request || !request->filename) 
    {
        fprintf(stderr, ANSI_RED "Invalid request parameters\n" ANSI_RST);
        return false;
    }

    size_t filename_len = strlen(request->filename);
    size_t mode_len = strlen(MODE);
    
    request->buffer[0] = 0;
    request->buffer[1] = request->opcode;

    memcpy(request->buffer + TFTP_OPCODE_SIZE, request->filename, filename_len);
    request->buffer[TFTP_OPCODE_SIZE + filename_len] = '\0';

    memcpy(request->buffer + TFTP_OPCODE_SIZE + filename_len + 1, MODE, mode_len + 1);

    ssize_t sent_bytes = sendto(client->sockfd, request->buffer, 
        TFTP_OPCODE_SIZE + filename_len + 1 + strlen(MODE) + 1, 0,
        (struct sockaddr *)&client->addr, client->addr_len);

    if (sent_bytes < 0) 
    {
        fprintf(stderr, ANSI_RED "Failed to send request: %s\n" ANSI_RST, strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Handles a TFTP file download request.
 *
 * Sends a Read Request (RRQ) to the server and writes the received data to a file.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the download was successful, false otherwise.
 */
bool download_request(TFTPSocket *client, TFTPRequest *request) 
{
    printf("File to be downloaded: '%s'\n", request->filename);
    bool success = false;

    if (!send_request(client, request)) 
    {
        cleanup(client, request);
        return false;
    }

    request->filepath = fopen(request->filename, "wb");
    if (!request->filepath) 
    {
        send_error_packet(client, ERROR_UNDEFINED, request->buffer, "Error opening file for writing");
        print_error(request->buffer, TFTP_MAX_PACKET_SIZE);
        cleanup(client, request);
        return false;
    }

    int block_number = 1;
    int retry = -1;

    while (process_transfer_receive(client, request, &block_number, &retry, &success));
    
    if (!success || request->buffer[1] == TFTP_ERROR)
        remove(request->filename);

    cleanup(client, request);
    return success;
}

/**
 * @brief Handles a TFTP file upload request.
 *
 * Sends a Write Request (WRQ) to the server and transmits file data in blocks.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the upload was successful, false otherwise.
 */
bool upload_request(TFTPSocket *client, TFTPRequest *request) 
{
    printf("File to be uploaded: '%s'\n", request->filename);
    bool success = false;

    if (!send_request(client, request)) 
    {
        cleanup(client, request);
        return false;
    }
    
    if (get_response(client, request->buffer) < 0) 
    {
        cleanup(client, request);
        return false;
    }
    
    int block_number = 1;
    int retry = 0;

    while (process_transfer_send(client, request, &block_number, &retry, &success));

    cleanup(client, request);
    return success;
}

/**
 * @brief Handles a TFTP delete request.
 *
 * Sends a Delete Request (DRQ) to the server and waits for confirmation.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the file was deleted successfully, false otherwise.
 */
bool delete_request(TFTPSocket *client, TFTPRequest *request)
{
    if (!send_request(client, request))
    {
        cleanup(client, request);
        return false;
    }
    
    printf("File to be deleted: '%s'\n", request->filename);

    if (get_response(client, request->buffer) < 0)
    {
        cleanup(client, request);
        return false;
    }

    printf(ANSI_GREEN "File '%s' deleted successfully!\n" ANSI_RST, request->filename);
    
    cleanup(client, request);
    return true;
}

