/**
 * @file client.c
 * @brief Entry point for the TFTP client application.
 *
 * This file contains the main() function that initializes the client,
 * validates command-line arguments, starts TFTP operations, and performs cleanup.
 */

# include "prog.h"

/**
 * @brief Main function for the TFTP client.
 *
 * Validates arguments, initializes the client socket, and invokes the TFTP process.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return int EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) 
{
    TFTPRequest request; 
    if (!validate_arguments(argc, argv, &request)) 
        return EXIT_FAILURE; 
        
    TFTPSocket client;
    if (!initialize_socket(&client, &request)) 
        return EXIT_FAILURE;  
        
    if (!prog(&client, &request))
        return EXIT_FAILURE;
    
    cleanup(&client, &request);

    return EXIT_SUCCESS;
}

