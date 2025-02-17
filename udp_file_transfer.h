/**
 * @file udp_file_transfer.h
 * @brief Header file for UDP-based file transfer (TFTP-like) implementation.
 *
 * This file defines packet formats, constants, and function prototypes
 * for handling file upload, download, and deletion using UDP.
 *
 * @author Salman Amer
 * @date 2025
 */

#ifndef UDP_FILE_TRANSFER_H
#define UDP_FILE_TRANSFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/** @brief Server port for communication */
#define SERVER_PORT 8080
/** @brief Buffer size for file transfer */
#define BUFFER_SIZE 1024
/** @brief Data block size for transmission */
#define BLOCK_SIZE 512

/**
 * @enum Opcode
 * @brief Enumeration for TFTP packet opcodes
 */
typedef enum {
    RRQ = 0, /**< Read Request */
    WRQ = 1, /**< Write Request */
    DEL = 2, /**< Delete Request */
    DATA = 3, /**< Data Packet */
    ACK = 4, /**< Acknowledgment Packet */
    ERR = 5  /**< Error Packet */
} Opcode;

/**
 * @struct TftpPacket
 * @brief Structure representing a TFTP packet
 */
typedef struct {
    uint16_t opcode; /**< Packet opcode (operation code) */
    uint16_t block;  /**< Block number (used in DATA and ACK packets) */
    char buffer[BUFFER_SIZE]; /**< Data buffer for file transfer */
} TftpPacket;

/**
 * @brief Initializes the UDP connection for the client.
 */
void Init_Connect();

/**
 * @brief Sends a Read Request (RRQ) to download a file from the server.
 * @param filename Name of the file to be downloaded.
 */
void Send_rrq(const char* filename);

/**
 * @brief Sends a Write Request (WRQ) to upload a file to the server.
 * @param filename Name of the file to be uploaded.
 */
void Send_wrq(const char* filename);

/**
 * @brief Sends a Delete Request (DEL) to remove a file from the server.
 * @param filename Name of the file to be deleted.
 */
void Send_del(const char* filename);

/**
 * @brief Handles a Read Request (RRQ) from the client and sends file data.
 * @param filename Name of the file to be sent.
 * @param addr_len Length of the client's address structure.
 */
void handle_rrq(char* filename, socklen_t addr_len);

/**
 * @brief Handles a Write Request (WRQ) from the client and receives file data.
 * @param filename Name of the file to be received.
 * @param addr_len Length of the client's address structure.
 */
void handle_wrq(char* filename, socklen_t addr_len);

/**
 * @brief Handles a Delete Request (DEL) from the client and removes a file.
 * @param filename Name of the file to be deleted.
 * @param addr_len Length of the client's address structure.
 */
void handle_del(char* filename, socklen_t addr_len);

/**
 * @brief Processes incoming requests from clients and dispatches them.
 */
void process_request();

#endif /* UDP_FILE_TRANSFER_H */
