/**
 * @file server.c
 * @brief Entry point for the TFTP server.
 *
 * This file initializes the server socket and starts processing incoming TFTP requests.
 */

# include "prog.h"

/**
 * @brief Main entry point for the TFTP server.
 *
 * Initializes the TFTP server socket and enters the request handling loop.
 *
 * @return EXIT_SUCCESS if the server runs correctly, EXIT_FAILURE on failure.
 */
int main() 
{
    TFTPSocket server;
    if (!initialize_socket(&server))
        return EXIT_FAILURE;

    prog(&server);

    return EXIT_SUCCESS;
}