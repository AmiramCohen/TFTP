# TFTP Client-Server Implementation

-----------------------------------------------------------------------------
## Overview
This project is a **TFTP (Trivial File Transfer Protocol) Client and Server** implemented in **C**.  
It enables **file transfers** over a **UDP-based connection**, following **RFC 1350**.  

## Supported Features
**Read Request (RRQ)** – Download a file from the server  
**Write Request (WRQ)** – Upload a file to the server  
**Delete Request (DRQ)** – Remove a file on the server  
**Error Handling** – Implements standard **TFTP error codes**  
**Timeout & Retransmission Mechanism** – Ensures reliable file transfer  
**Modular & Efficient Code** – Separate common files for shared functions  

-----------------------------------------------------------------------------
## Project Structure
```
TFTP/  
│── client/                 # Client-side source code  
│   ├── client.c            # Client main entry point  
│   ├── prog.c              # Handles client logic and request validation  
│   ├── tftp_client.c       # Implements TFTP client functionality  
│   ├── tftp_client.h       # Header file for the client  
│   ├── prog.h              # Header file for program logic  
│   ├── Makefile            # Makefile to compile only the client  
│  	
│── server/                 # Server-side source code  
│   ├── server.c            # Server main entry point  
│   ├── prog.c              # Handles server logic and request validation  
│   ├── tftp_server.c       # Implements TFTP server functionality  
│   ├── tftp_server.h       # Header file for the server  
│   ├── prog.h              # Header file for program logic  
│   ├── Makefile            # Makefile to compile only the server  
│  	
│── common/                 # Shared utilities between client & server  
│   ├── common.c            # Implements shared functions  
│   ├── common.h            # Header file for shared functions  
│  	
│── Makefile                # Unified Makefile to compile both client & server  
│── README.md               # Documentation and instructions 
``` 

-----------------------------------------------------------------------------
## **Compilation & Installation**
### **Build Both Client & Server (Unified Makefile)**
Run the following command in the project `TFTP/` directory: `make`
### **Build only the Client**
Run the following command inside the `client/` directory: `make`
This generates an executable named `client`.
### **Build only the Server**
Run the following command inside the `server/` directory: `make`
This generates an executable named `server`.
### **Clean Compilation Artifacts**
To remove compiled binaries and object files, run: `make clean`

-----------------------------------------------------------------------------
## **Usage**
### **Starting the TFTP Server**
The server listens on port **69 (default TFTP port)** and handles requests: `sudo ./server`

**Note:** Since TFTP uses **port 69**, root privileges (`sudo`) are required.

### **Uploading a File**
To upload a file from the client to the server: `./client upload <file_path> <server_ip>`  
**Example:** `./client upload example.txt 127.0.0.1`

### **Downloading a File**
To download a file from the server to the client: `./client download <file_name> <server_ip>`  
**Example:** `./client download example.txt 127.0.0.1`

### **Deleting a File**
To request file deletion on the server: `./client delete <file_name> <server_ip>`  
**Example:** `./client delete example.txt 127.0.0.1`

-----------------------------------------------------------------------------
## **Configuration**
Modify **TFTP timeout, retries, and block size** inside `common.h`:
```c
# define TFTP_TIMEOUT       5       // Timeout duration in seconds
# define TFTP_MAX_RETRIES   3       // Maximum retransmission attempts
# define TFTP_MAX_DATA_SIZE 512     // Max data bytes per packet
```
Recompile the project (`make`) after changes.

-----------------------------------------------------------------------------
## **Error Handling**
```
|   Error Code  |              Description                |  
|---------------------------------------------------------|  
|       0       |       Undefined error                   |  
|       1       |       File not found                    |  
|       2       |       Access violation                  |  
|       3       |       Disk full or allocation exceeded  |  
|       4       |       Illegal TFTP operation            |  
|       5       |       Unknown transfer ID               |  
|       6       |       File already exists               |  
|       7       |       No such user                      |  
```

**Example:**
Error 1: File not found

-----------------------------------------------------------------------------
## **Troubleshooting**
### **Permission Denied (Port 69)**
Since TFTP runs on a privileged port, run the server with `sudo`: `sudo ./server`
