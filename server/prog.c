/**
 * @file prog.c
 * @brief Handles TFTP server request validation and execution.
 *
 * This file contains functions for validating incoming requests, initializing the server,
 * handling different TFTP operations (upload, download, delete), and managing user privileges.
 */

# include "prog.h"
# include "tftp_server.h"

/**
 * @brief Initializes the TFTP server socket.
 *
 * Creates a UDP socket, binds it to the default TFTP port, and prepares it for incoming requests.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @return true if initialization succeeds, false on failure.
 */
bool initialize_socket(TFTPSocket *server) 
{
    if ((server->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        fprintf(stderr, ANSI_RED "Error: Socket creation failed\n" ANSI_RST);
        return false;
    }

    server->addr_len = sizeof(server->addr);

    memset(&server->addr, 0, server->addr_len);
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = INADDR_ANY;
    server->addr.sin_port = htons(TFTP_PORT);  

    if (bind(server->sockfd, (const struct sockaddr*)&server->addr, server->addr_len) < 0) 
    {
        fprintf(stderr, ANSI_RED "Error: Bind failed.\n" ANSI_RST);
        close(server->sockfd);
        return false;
    }

    printf(ANSI_PURPLE "Server is listening on port %d...\n" ANSI_RST, TFTP_PORT);

    return true;
}

/**
 * @brief Main execution function for processing incoming TFTP requests.
 *
 * This function drops root privileges, initializes the TFTPRequest structure,
 * and then enters an infinite loop where it waits for, validates, and handles incoming TFTP requests.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 */
void prog(TFTPSocket *server) 
{
    drop_privileges();
    TFTPRequest request = { 0 };
    request.timeout = TFTP_TIMEOUT;
    request.block_size = TFTP_MAX_DATA_SIZE;

    while (1) 
    {
        init_request(&request);

        printf(ANSI_CYAN "\nWaiting for requests...\n" ANSI_RST);
        
        ssize_t len = recvfrom(server->sockfd, request.buffer, TFTP_MAX_PACKET_SIZE, 0,
            (struct sockaddr *)&server->addr, &server->addr_len);

        if (len <= 0)
            continue;

        if (!validate_request(server, &request, len))
            continue;
        
        handle_request(server, &request);
    }
}

/**
 * @brief Initializes a TFTPRequest structure for a new request.
 *
 * This function first cleans up resources from a previous request 
 * and then resets the structure to its default state so that it is ready for a new request.
 *
 * @param request Pointer to the TFTPRequest structure to initialize.
 */
void init_request(TFTPRequest *request)
{
    if (!request) return;
    
    cleanup_request(request);
    memset(request->buffer, 0, sizeof(request->buffer));
    request->opcode = 0;
}

/**
 * @brief Releases dynamically allocated resources in a TFTPRequest structure.
 *
 * This function frees the memory allocated for the filename and closes the open file pointer,
 * if they exist, then sets the pointers to NULL.
 *
 * @param request Pointer to the TFTPRequest structure to clean up.
 */
void cleanup_request(TFTPRequest *request)
{
    if (!request) return;

    if (request->filename)
    {
        free(request->filename);
        request->filename = NULL;
    }

    if (request->filepath)
    {
        fclose(request->filepath);
        request->filepath = NULL;
    }
}

/**
 * @brief Drops root privileges after binding the server to port 69.
 *
 * The server initially runs as root to bind to port 69 but should
 * drop privileges to a non-root user for security reasons.
 *
 * @return true if privileges are successfully dropped, false otherwise.
 */
bool drop_privileges()
{
    const char *username = getenv("SUDO_USER");
    if (!username) 
    {
        struct passwd *pw = getpwuid(getuid());
        if (!pw) 
        {
            fprintf(stderr, ANSI_RED "Error: Unable to retrieve user information\n" ANSI_RST);
            return false;
        }
        username = pw->pw_name;
    }

    struct passwd *pw = getpwnam(username);
    if (!pw) 
    {
        fprintf(stderr, ANSI_RED "Error: Failed to get user information" ANSI_RST);
        return false;
    }

    struct group *gr = getgrgid(pw->pw_gid);
    if (!gr) 
    {
        fprintf(stderr, ANSI_RED "Error: Failed to get group information" ANSI_RST);
        return false;
    }

    if (setgid(gr->gr_gid) != 0 || setuid(pw->pw_uid) != 0) 
    {
        fprintf(stderr, ANSI_RED "Error: Failed to drop privileges" ANSI_RST);
        return false;
    }
    
    return true;
}

/**
 * @brief Validates the mode of the TFTP request.
 *
 * Ensures that the requested mode is "octet" (binary transfer).
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the mode is valid, false otherwise.
 */
bool validate_mode(TFTPSocket *server, TFTPRequest *request) 
{    
    char *filename = (char *)(request->buffer + TFTP_OPCODE_SIZE);  
    size_t filename_len = strlen(filename);  

    char *mode = filename + filename_len + 1;  

    if (strcasecmp(mode, MODE) != 0) 
    {
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, "Unsupported mode: Only 'octet' is allowed");

        print_error(request->buffer, TFTP_MAX_PACKET_SIZE);
        return false;
    }

    return true;
}

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
bool validate_request(TFTPSocket *server, TFTPRequest *request, ssize_t len) 
{
    if (!validate_request_count(server, request, len))
        return false;

    if (!validate_mode(server, request))
        return false;
    
    if (!validate_operation(server, request))
        return false;

    if(!validate_filename(server, request))
        return false;

    if (request->opcode == TFTP_RRQ)
        return validate_download_file(server, request);

    if (request->opcode == TFTP_WRQ)
        return validate_upload_file(server, request);

    if (request->opcode == TFTP_DRQ)
        return validate_delete_file(server, request);
    
    return true;
}

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
bool validate_request_count(TFTPSocket *server, TFTPRequest *request, ssize_t len)
{
    if (!server || !request || len < TFTP_DATA_HEADER_SIZE) 
    {   
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, "Invalid request received");
        
        return false;
    }

    return true;
}

/**
 * @brief Validates the requested TFTP operation.
 *
 * Extracts the opcode from the request packet and ensures it is a valid operation.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the operation is valid, false otherwise.
 */
bool validate_operation(TFTPSocket *server, TFTPRequest *request)
{
    unsigned short opcode = (request->buffer[0] << 8) | request->buffer[1];
    
    if (opcode != TFTP_RRQ && opcode != TFTP_WRQ && opcode != TFTP_DRQ) 
    {
        send_error_packet(server, ERROR_ILLEGAL_OPERATION, request->buffer, NULL);
        return false;
    }
    
    request->opcode = opcode;

    return true;
}

/**
 * @brief Validates the filename extracted from the TFTP request.
 *
 * Ensures that the filename does not exceed the maximum allowed length and is correctly formatted.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the filename is valid, false otherwise.
 */
bool validate_filename(TFTPSocket *server, TFTPRequest *request)
{
    char *filename = (char *)(request->buffer + TFTP_OPCODE_SIZE);
    
    if (strnlen((char *)(request->buffer + TFTP_OPCODE_SIZE), MAX_FILE_NAME) == 0) 
    {
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, "Filename missing");
        return false;
    }
    
    char *temp = strdup(filename);
    if (!temp)
    {
        send_error_packet(server, ERROR_UNDEFINED, request->buffer, "Memory allocation failed");
        return false;
    }

    if (request->filename)  
    {
        free(request->filename);
        request->filename = NULL;
    }

    request->filename = temp;

    return true;
}

/**
 * @brief Validates and prepares a file for upload.
 *
 * Ensures that the file does not already exist to prevent accidental overwrites.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is ready for upload, false otherwise.
 */
bool validate_upload_file(TFTPSocket *server, TFTPRequest *request)
{
    if (access(request->filename, F_OK) == 0) 
    {
        fprintf(stderr, ANSI_RED "Error %d: File already exists\n" ANSI_RST, ERROR_FILE_EXISTS);
        send_error_packet(server, ERROR_FILE_EXISTS, request->buffer, NULL);
        return false;
    }
    
    return true;
}

/**
 * @brief Validates the file for a download operation.
 *
 * Ensures that the file exists and is accessible before sending data.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_download_file(TFTPSocket *server, TFTPRequest *request)
{
    return (validate_file_access(server, request));
}

/**
 * @brief Validates file access for delete operations.
 *
 * Ensures that the file exists before attempting to delete it.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file exists and can be deleted, false otherwise.
 */
bool validate_delete_file(TFTPSocket *server, TFTPRequest *request)
{
    if (access(request->filename, F_OK) != 0) 
    {
        send_error_packet(server, ERROR_FILE_NOT_FOUND, request->buffer, NULL);
        print_error(request->buffer, TFTP_MAX_PACKET_SIZE);
        return false;
    }

    return true;
}

/**
 * @brief Validates file access permissions for read operations.
 *
 * Ensures that the file exists and has the required read permissions.
 *
 * @param server Pointer to the TFTPSocket structure representing the server.
 * @param request Pointer to the TFTPRequest structure containing request data.
 * @return true if the file is accessible, false otherwise.
 */
bool validate_file_access(TFTPSocket *server, TFTPRequest *request)
{
    if (access(request->filename, F_OK) != 0) 
    {
        fprintf(stderr, ANSI_RED "Error %d: File not found\n" ANSI_RST, ERROR_FILE_NOT_FOUND);
        send_error_packet(server, ERROR_FILE_NOT_FOUND, request->buffer, NULL);
        return false;
    }
    
    if (access(request->filename, R_OK) != 0)
    {
        fprintf(stderr, ANSI_RED "Error %d: Access violation\n" ANSI_RST, ERROR_ACCESS_VIOLATION);
        send_error_packet(server, ERROR_ACCESS_VIOLATION, request->buffer, NULL);        
        return false;
    }

    request->filepath = fopen(request->filename, "rb");
    
    return (request->filepath != NULL);
}

