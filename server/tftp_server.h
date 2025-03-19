/**
 * @file tftp_server.h
 * @brief Header file defining TFTP server operations.
 *
 * This file contains function declarations for processing TFTP requests,
 * including file upload, download, and delete operations.
 */

# ifndef TFTP_SERVER_H
# define TFTP_SERVER_H


# include "common.h"
# include "prog.h"
# include <pwd.h>
# include <grp.h>


// ========================================
//          FUNCTION PROTOTYPES
// ========================================

/**
 * @brief Handles an incoming TFTP request.
 *
 * Determines the operation type (RRQ, WRQ, DRQ) and calls the appropriate function.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void handle_request(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Handles a TFTP file upload request.
 *
 * Receives file data from the client and writes it to the local filesystem.
 * Sends acknowledgments for received data blocks.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void upload_request(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Handles a TFTP file download request.
 *
 * Reads a file from the local filesystem and sends it to the client in blocks.
 * Waits for acknowledgments from the client.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void download_request(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Handles a TFTP file delete request.
 *
 * Attempts to delete the specified file and sends an acknowledgment or error response.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request details.
 */
void delete_request(TFTPSocket *server, TFTPRequest *request);


# endif // TFTP_SERVER_H