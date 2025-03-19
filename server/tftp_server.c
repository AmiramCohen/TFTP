/**
 * @file tftp_server.c
 * @brief Implements TFTP server request handling.
 *
 * This file contains functions for processing TFTP requests, handling file transfers,
 * and managing delete operations.
 */

# include "tftp_server.h"

/**
 * @file tftp_server.c
 * @brief Implements TFTP server request handling.
 *
 * This file contains functions for processing TFTP requests, handling file transfers,
 * and managing delete operations.
 */
void handle_request(TFTPSocket *server, TFTPRequest *request) 
{
    if (!set_socket_timeout(server, request->timeout)) return;

    switch (request->opcode) 
    {
        case TFTP_RRQ:  // RRQ (Request to Downoad)
            download_request(server, request);
            break;

        case TFTP_WRQ:  // WRQ (Request to Upload)
            upload_request(server, request);
            break;

        case TFTP_DRQ:  // DRQ (Request to Delete)
            delete_request(server, request);
            break;
        
        default:
            send_error_packet(server, ERROR_ILLEGAL_OPERATION, request->buffer, NULL);
            print_error(request->buffer, TFTP_MAX_PACKET_SIZE);
            break;
    }

    set_socket_timeout(server, 0);
}

/**
 * @brief Handles a TFTP file upload request.
 *
 * Opens the file for writing, acknowledges the request, and receives data blocks from the client.
 * If an error occurs or the upload fails, the file is removed.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void upload_request(TFTPSocket *server, TFTPRequest *request) 
{            
    printf("File to be uploaded: '%s'\n", request->filename);
    bool success = false;
    int block_number = 0;
    
    request->filepath = fopen(request->filename, "wb");
    if (!request->filepath) 
    {
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, "Error opening file for writing");
        print_error(request->buffer, TFTP_MAX_PACKET_SIZE);
        return;
    }

    if (!send_ack(server, request->buffer, block_number++))
    {
        fclose(request->filepath);
        request->filepath = NULL;
        return;
    }

    int retry = -2;

    while (process_transfer_receive(server, request, &block_number, &retry, &success));

    if (!success || request->buffer[1] == TFTP_ERROR)
        remove(request->filename);
    
    fclose(request->filepath);
    request->filepath = NULL;
    
    return; 
}

/**
 * @brief Handles a TFTP file download request.
 *
 * Sends the requested file in data blocks to the client.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void download_request(TFTPSocket *server, TFTPRequest *request) 
{
    printf("File to be downloaded: '%s'\n", request->filename);
    bool success = false;
    
    int block_number = 1;
    int retry = 0;

    while (process_transfer_send(server, request, &block_number, &retry, &success));
    
    fclose(request->filepath);
    request->filepath = NULL;
    
    return;
}

/**
 * @brief Handles a TFTP file delete request.
 *
 * Attempts to delete the specified file and sends an acknowledgment or error response.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void delete_request(TFTPSocket *server, TFTPRequest *request) 
{
    printf("File to be deleted: '%s'\n", request->filename);

    if (remove(request->filename) == 0) 
    {
        fprintf(stderr, ANSI_GREEN "File '%s' deleted successfully!\n" ANSI_RST, request->filename);
        send_ack(server, request->buffer, 0); 
    }
    
    else 
    {
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, strerror(errno));
        fprintf(stderr, ANSI_RED "File '%s' not deleted\n" ANSI_RST, request->filename);
    }
}
