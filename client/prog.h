/**
 * @file prog.h
 * @brief Header file for handling TFTP client request validation and execution.
 *
 * This file contains function declarations for validating user arguments,
 * handling TFTP operations (upload, download, delete), and managing socket initialization.
 */

# ifndef PROG_H
# define PROG_H


# include "common.h"


# define MODE "octet"   ///< Default transfer mode for TFTP


// ========================================
//              ENUMERATIONS
// ========================================

/**
 * @enum INPUT
 * @brief Enum defining command-line argument positions.
 */
typedef enum { 
    COMMAND = 0,        ///< The command (client executable name)
    OPERATION,          ///< The TFTP operation (upload/download/delete)
    FILE_PATH,          ///< The file path for the requested operation
    SERVER_IP           ///< The IP address of the TFTP server
} INPUT;


// ========================================
//          FUNCTION PROTOTYPES
// ========================================

/**
 * @brief Validates command-line arguments for TFTP requests.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param request Pointer to the TFTPRequest structure to store request details.
 * @return true if arguments are valid, false otherwise.
 */
bool validate_arguments(int argc, char *argv[], TFTPRequest *request);

/**
 * @brief Checks if the correct number of arguments is provided.
 * @param argc Number of command-line arguments.
 * @return true if the number of arguments is correct, false otherwise.
 */
bool validate_argument_count(int argc);

/**
 * @brief Validates the requested TFTP operation (upload, download, delete).
 * @param operation The requested operation string.
 * @param request Pointer to the TFTPRequest structure where the opcode will be stored.
 * @return true if the operation is valid, false otherwise.
 */
bool validate_operation(const char *operation, TFTPRequest *request);

/**
 * @brief Validates if the filename is valid and extracts it from the file path.
 * @param filepath The full file path.
 * @param request Pointer to the TFTPRequest structure where the filename will be stored.
 * @return true if the filename is valid, false otherwise.
 */
bool validate_filename(const char *filepath, TFTPRequest *request);

/**
 * @brief Checks file accessibility for upload operations.
 * @param filepath Path to the file.
 * @param request Pointer to the TFTPRequest structure where the file pointer will be stored.
 * @return true if the file exists and is readable, false otherwise.
 */
bool validate_upload_file(const char *filepath, TFTPRequest *request);

/**
 * @brief Ensures that a file does not already exist before download.
 * @param request Pointer to the TFTPRequest structure containing filename details.
 * @return true if the file does not exist, false otherwise.
 */
bool validate_download_file(TFTPRequest *request);

/**
 * @brief Validates file access permissions for read operations.
 * @param filepath Path to the file being accessed.
 * @param file Pointer to a FILE* where the opened file handle will be stored.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_file_access(const char *filepath, FILE **file);

/**
 * @brief Initializes the TFTP client socket.
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @return true if the socket was initialized successfully, false otherwise.
 */
bool initialize_socket(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Main function for executing the requested TFTP operation.
 *
 * Calls the appropriate function (upload_request(), download_request(), or delete_request())
 * based on the operation provided by the user.
 *
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @return true if the operation was successful, false otherwise.
 */
bool prog(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Cleans up allocated resources.
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 */
void cleanup(TFTPSocket *client, TFTPRequest *request);

/**
 * @brief Extracts the filename from a given file path.
 *
 * Takes a full file path and extracts only the filename.
 *
 * @param path The full file path.
 * @return A dynamically allocated string containing the filename, or NULL on failure.
 */
char *get_filename_from_path(const char *path);


# endif // PROG_H
