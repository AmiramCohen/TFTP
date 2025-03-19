/**
 * @file prog.c
 * @brief Handles TFTP client request validation and execution.
 *
 * This file contains functions for validating user arguments, handling
 * TFTP operations (upload, download, delete), and managing socket initialization.
 */

# include "prog.h"
# include "tftp_client.h"

/**
 * @brief Validates command-line arguments for TFTP requests.
 *
 * This function ensures the correct number of arguments, validates the
 * requested operation, and checks file access permissions before initiating
 * a TFTP transaction.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param request Pointer to the TFTPRequest structure to store request details.
 * @return true if arguments are valid, false otherwise.
 */
bool validate_arguments(int argc, char *argv[], TFTPRequest *request) 
{
    if (!validate_argument_count(argc))
        return false;

    if (!validate_operation(argv[OPERATION], request))
        return false;

    request->server_ip = argv[SERVER_IP];
    request->timeout = TFTP_TIMEOUT;
    request->block_size = TFTP_MAX_DATA_SIZE;
    request->filepath = NULL;

    if (!validate_filename(argv[FILE_PATH], request))
        return false;

    if (request->opcode == TFTP_WRQ)
        return validate_upload_file(argv[FILE_PATH], request);

    if (request->opcode == TFTP_RRQ)
        return validate_download_file(request);

    return true;
}

/**
 * @brief Validates the number of arguments passed to the program.
 * @param argc Number of command-line arguments.
 * @return true if the number of arguments is correct, false otherwise.
 */
bool validate_argument_count(int argc) 
{
    if (argc != TFTP_NUM_ARGUMENTS) 
    {
        fprintf(stderr, ANSI_RED "Invalid command\nUsage: <operation> <filename> <server_ip>\n" ANSI_RST);
        return false;
    }
    
    return true;
}

/**
 * @brief Validates the requested TFTP operation (upload, download, delete).
 *
 * @param operation The requested operation string.
 * @param request Pointer to the TFTPRequest structure where the opcode will be stored.
 * @return true if the operation is valid, false otherwise.
 */
bool validate_operation(const char *operation, TFTPRequest *request)
{
    if (strcmp(operation, UPLOAD) == 0)
        request->opcode = TFTP_WRQ;
    else if (strcmp(operation, DOWNLOAD) == 0)
        request->opcode = TFTP_RRQ;
    else if (strcmp(operation, DELETE) == 0)
        request->opcode = TFTP_DRQ;
    else
    {
        fprintf(stderr, ANSI_RED "Error %d: Illegal TFTP operation\n" ANSI_RST, ERROR_ILLEGAL_OPERATION);
        return false;   
    }

    return true;
}

/**
 * @brief Validates the provided filename.
 * @param filepath The full file path.
 * @param request Pointer to the TFTPRequest structure to store the filename.
 * @return true if the filename is valid, false otherwise.
 */
bool validate_filename(const char *filepath, TFTPRequest *request)
{
    if (strlen(filepath) > MAX_FILE_NAME) 
    {
        fprintf(stderr, ANSI_RED "File path length exceeds maximum size %d\n" ANSI_RST, MAX_FILE_NAME);
        return false;
    }

    request->filename = get_filename_from_path(filepath);

    return (request->filename != NULL);
}

/**
 * @brief Validates file access for upload.
 *
 * Checks if the file exists and has read permissions before proceeding.
 *
 * @param filepath Path to the file.
 * @param request Pointer to the TFTPRequest structure where the file pointer will be stored.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_upload_file(const char *filepath, TFTPRequest *request)
{
    if (!validate_file_access(filepath, &request->filepath))
    {
        free(request->filename);
        return false;
    }
    
    return true;
}

/**
 * @brief Validates file existence for downloads.
 *
 * Ensures the file does not already exist before initiating a download request.
 *
 * @param request Pointer to the TFTPRequest structure containing filename details.
 * @return true if the file does not exist, false otherwise.
 */
bool validate_download_file(TFTPRequest *request)
{
    if (access(request->filename, F_OK) == 0) 
    {
        fprintf(stderr, ANSI_RED "Error %d: File already exists\n" ANSI_RST, ERROR_FILE_EXISTS);
        free(request->filename);
        request->filename = NULL;
        return false;
    }
    
    return true;
}

/**
 * @brief Validates if a file is accessible for reading.
 *
 * This function checks if the file exists and has the necessary read permissions
 * before allowing the upload request to proceed.
 *
 * @param filepath Path to the file being accessed.
 * @param file Pointer to a FILE* where the opened file handle will be stored.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_file_access(const char *filepath, FILE **file)
{
    if (access(filepath, F_OK) != 0) 
    {
        fprintf(stderr, ANSI_RED "Error %d: File not found\n" ANSI_RST, ERROR_FILE_NOT_FOUND);
        return false;
    }
    
    if (access(filepath, R_OK) != 0)
    {
        fprintf(stderr, ANSI_RED "Error %d: Access violation\n" ANSI_RST, ERROR_ACCESS_VIOLATION);
        return false;
    }

    *file = fopen(filepath, "rb");
    
    return (*file != NULL);
}

/**
 * @brief Initializes the TFTP socket for communication.
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @return true if the socket was initialized successfully, false otherwise.
 */
bool initialize_socket(TFTPSocket *client, TFTPRequest *request) 
{
    if (client == NULL || request == NULL) 
    {
        fprintf(stderr, ANSI_RED "Invalid client or request pointer\n" ANSI_RST);
        return false;
    }

    client->addr_len = sizeof(client->addr);
    memset(&client->addr, 0, client->addr_len);
    client->addr.sin_family = AF_INET;
    client->addr.sin_port = htons(TFTP_PORT);

    if ((client->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        fprintf(stderr, ANSI_RED "Socket creation failed\n" ANSI_RST);
        cleanup(client, request);
        return false;
    }

    if (inet_pton(AF_INET, request->server_ip, &client->addr.sin_addr) <= 0) 
    {
        fprintf(stderr, ANSI_RED "Invalid address or address not supported: %s\n" ANSI_RST, request->server_ip);
        cleanup(client, request);
        return false;
    }

    if (!set_socket_timeout(client, request->timeout))
    {
        fprintf(stderr, ANSI_RED "Socket set timeout failed\n" ANSI_RST);
        cleanup(client, request);
        return false;
    }    

    return true; 
}

/**
 * @brief Main execution function for processing TFTP operations.
 *
 * Based on the user's request, this function calls the appropriate TFTP function:
 * - upload_request for file uploads.
 * - download_request for file downloads.
 * - delete_request for file deletions.
 *
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 * @return true if the operation was successful, false otherwise.
 */
bool prog(TFTPSocket *client, TFTPRequest *request) 
{
    if (request->opcode == TFTP_RRQ) 
        return download_request(client, request);

    else if (request->opcode == TFTP_WRQ)  
        return upload_request(client, request);
    
    else if (request->opcode == TFTP_DRQ) 
        return delete_request(client, request);
    
    else 
    {
        fprintf(stderr, ANSI_RED "Error %d: Illegal TFTP operation\n" ANSI_RST, ERROR_ILLEGAL_OPERATION);
        return false;
    }
}

/**
 * @brief Cleans up allocated resources.
 * @param client Pointer to the TFTPSocket structure.
 * @param request Pointer to the TFTPRequest structure.
 */
void cleanup(TFTPSocket *client, TFTPRequest *request) 
{
    if (!client || !request) return;

    if (client->sockfd >= 0) 
    {
        close(client->sockfd);
        client->sockfd = -1;
    }

    if (request->filepath) 
    {
        fclose(request->filepath);
        request->filepath = NULL;
    }

    if (request->filename)
    {
        free((char *)request->filename);
        request->filename = NULL;
    }
}

/**
 * @brief Extracts the filename from a full file path.
 *
 * This function takes a full file path and extracts only the filename. 
 *
 * @param path The full file path.
 * @return A dynamically allocated string containing the filename, or NULL on failure.
 */
char *get_filename_from_path(const char *path) 
{
    if (!path) return NULL;
    
    char *path_copy = strdup(path);  
    if (!path_copy) return NULL;
    
    char *filename = basename(path_copy);
    char *result = filename ? strdup(filename) : NULL;

    free(path_copy); 
    
    return result;
}