/**
 * @file tftp_client.h
 * @brief Header file defining TFTP client operations.
 *
 * This file contains function declarations for sending TFTP requests,
 * handling file uploads, downloads, and delete operations.
 */

# ifndef TFTP_CLIENT_H
# define TFTP_CLIENT_H


# include "prog.h"
# include "common.h"


// ========================================
//          FUNCTION PROTOTYPES
// ========================================

/**
 * @brief Sends a TFTP request (Read, Write, Delete) to the server.
 *
 * Constructs a request packet containing the operation, filename, and mode.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the request was sent successfully, false otherwise.
 */
bool send_request(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Handles a TFTP file download request.
 *
 * Sends a Read Request (RRQ) to the server and writes the received data to a file.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the download was successful, false otherwise.
 */
bool download_request(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Handles a TFTP file upload request.
 *
 * Sends a Write Request (WRQ) to the server and transmits file data in blocks.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the upload was successful, false otherwise.
 */
bool upload_request(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Handles a TFTP delete request.
 *
 * Sends a Delete Request (DRQ) to the server and waits for confirmation.
 *
 * @param client Pointer to the TFTPSocket representing the client connection.
 * @param request Pointer to the TFTPRequest containing request details.
 * @return true if the file was deleted successfully, false otherwise.
 */
bool delete_request(TFTPSocket *client, TFTPRequest *request);


# endif // TFTP_CLIENT_H
