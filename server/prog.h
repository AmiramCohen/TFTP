/**
 * @file prog.h
 * @brief Header file for handling TFTP server request validation and execution.
 *
 * This file contains function declarations for initializing the server,
 * validating incoming requests, managing user privileges, and handling
 * file operations (upload, download, delete).
 */

# ifndef PROG_H
# define PROG_H


# include "common.h"


# define MODE   "octet"


// ========================================
//          FUNCTION PROTOTYPES
// ========================================

/**
 * @brief Initializes the TFTP server socket.
 *
 * Creates a UDP socket, binds it to the default TFTP port, and prepares it for incoming requests.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @return true if initialization succeeds, false on failure.
 */
bool initialize_socket(TFTPSocket *server);

/**
 * @brief Main execution function for processing incoming TFTP requests.
 *
 * This function runs an infinite loop where it listens for requests,
 * validates them, and calls handle_request for further processing.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 */
void prog(TFTPSocket *server);

/**
 * @brief Initializes a TFTPRequest structure for a new request.
 *
 * This function first cleans up resources from a previous request
 * and then resets the structure to its default state so that it is ready for a new request.
 *
 * @param request Pointer to the TFTPRequest structure.
 */
void init_request(TFTPRequest *request);

/**
 * @brief Releases dynamic resources in a TFTPRequest structure.
 *
 * Frees the memory allocated for the filename and closes the file pointer if open,
 * then sets the respective pointers to NULL.
 *
 * @param request Pointer to the TFTPRequest structure.
 */
void cleanup_request(TFTPRequest *request);

/**
 * @brief Drops root privileges after binding the server to port 69.
 *
 * The server initially runs as root to bind to port 69 but should
 * drop privileges to a non-root user for security reasons.
 *
 * @return true if privileges are successfully dropped, false otherwise.
 */
bool drop_privileges();

/**
 * @brief Validates an incoming TFTP request.
 *
 * Ensures the request is well-formed before processing.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @param len Length of the received request packet.
 * @return true if the request is valid, false otherwise.
 */
bool validate_request(TFTPSocket *server, TFTPRequest *request, ssize_t len);

/**
 * @brief Validates the number of arguments in the TFTP request packet.
 *
 * Ensures that the packet size is at least the minimum required for a valid request.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @param len Length of the received request packet.
 * @return true if the request size is valid, false otherwise.
 */
bool validate_request_count(TFTPSocket *server, TFTPRequest *request, ssize_t len);

/**
 * @brief Validates the requested TFTP operation.
 *
 * Extracts the opcode from the request packet and ensures it is a valid operation.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the operation is valid, false otherwise.
 */
bool validate_operation(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates the filename extracted from the TFTP request.
 *
 * Ensures that the filename does not exceed the maximum allowed length and is correctly formatted.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the filename is valid, false otherwise.
 */
bool validate_filename(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates the file for a download operation.
 *
 * Ensures that the file exists and is accessible before sending data.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_download_file(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates and prepares a file for upload.
 *
 * Ensures that the file does not already exist to prevent accidental overwrites.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is ready for upload, false otherwise.
 */
bool validate_upload_file(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates file access permissions for read operations.
 *
 * Ensures that the file exists and has the required read permissions.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_file_access(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates file access for delete operations.
 *
 * Ensures that the file exists before attempting to delete it.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file exists and can be deleted, false otherwise.
 */
bool validate_delete_file(TFTPSocket *server, TFTPRequest *request);

/**
 * @brief Validates the mode of the TFTP request.
 *
 * Ensures that the requested mode is "octet" (binary transfer).
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the mode is valid, false otherwise.
 */
bool validate_mode(TFTPSocket *server, TFTPRequest *request);


# endif // PROG_H
