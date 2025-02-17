#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_file_transfer.h"


int sockfd;
struct sockaddr_in server_addr, client_addr;



int main() {


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("TFTP server listening on port %d...\n", SERVER_PORT);

    while (1) {
        process_request();
    }

    close(sockfd);
    return 0;
}
// End of main.c


// a function to handle RRQ requests
void handle_rrq(char* filename, socklen_t addr_len) {
    uint16_t block = 1;
    int bytes_read;
    TftpPacket packet;
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        packet.opcode = htons(ERR);
        sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&client_addr, addr_len);
        perror("Open failed");
        return;
    }
    packet.opcode = htons(ACK);
    sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&client_addr, addr_len);

    while ((bytes_read = fread(packet.buffer, 1, BLOCK_SIZE, file)) > 0) {
        packet.opcode = htons(DATA);
        packet.block = htons(block);

        int ack_received = 0;
        while (!ack_received) {  // Keep sending until correct ACK is received
            sendto(sockfd, &packet, bytes_read + 4, 0, (const struct sockaddr *)&client_addr, addr_len);

            // Wait for ACK
            TftpPacket ack_packet;
            socklen_t client_len = addr_len;
            recvfrom(sockfd, &ack_packet, sizeof(TftpPacket), 0, (struct sockaddr *)&client_addr, &client_len);

            // Check if ACK is correct
            if (ntohs(ack_packet.opcode) == ACK && ntohs(ack_packet.block) == block) {
                ack_received = 1;  // Move to next block
            } else {
                printf("Incorrect ACK for block %d, resending...\n", block);
            }
        }
        block++;
    }

    fclose(file);
}

 // a function to handle WRQ requests
void handle_wrq(char* filename, socklen_t addr_len) {
    TftpPacket packet;
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Open failed");
        return;
    }


    packet.opcode = htons(ACK);
    packet.block = htons(0);
    sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&client_addr, addr_len);

    socklen_t client_len = addr_len;
    uint16_t expected_block = 1;
    int n;

    while ((n = recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&client_addr, &client_len)) > 0) {
        if (ntohs(packet.opcode) == DATA && ntohs(packet.block) == expected_block) {

            fwrite(packet.buffer, 1, n - 4, file);
            fflush(file);

            packet.opcode = htons(ACK);
            sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&client_addr, client_len);

            expected_block++;
            if (n < BLOCK_SIZE + 4) break;
        }else{
            if(ntohs(packet.opcode) == ERR){
                perror("Write failed");
                return;
            }
        }
    }

    fclose(file);
}
 // a function to handle DEL requests
void handle_del(char* filename,socklen_t addr_len) {
    TftpPacket packet;
    if (remove(filename) == 0) {
        packet.opcode = htons(ACK);
        sendto(sockfd, &packet,  4, 0, (const struct sockaddr *)&client_addr, addr_len);
        printf("The file %s has been deleted.\n", filename);
    } else {
        packet.opcode = htons(ERR);
        sendto(sockfd, &packet,  4, 0, (const struct sockaddr *)&client_addr, addr_len);
        perror("Deletion failed");
    }
}
 // a function to process incoming requests

void process_request() {
    socklen_t addr_len = sizeof(client_addr);
    TftpPacket packet;
    memset(&packet, 0, sizeof(packet));

    int received = recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&client_addr, &addr_len);
    if (received < 0) {
        perror("Receive failed");
        return;
    }

    uint16_t opcode = ntohs(packet.opcode);
    char* filename = packet.buffer;

    printf("Received request: Opcode %u, File: %s\n", opcode, filename);

    switch (opcode) {
        case RRQ:
            handle_rrq(filename, addr_len);
            break;
        case WRQ:
            handle_wrq(filename, addr_len);
            break;
        case DEL:
            handle_del(filename, addr_len);
            break;
        case ERR:
            perror("Error Received");
            break;    
        default:
            printf("Invalid opcode received.\n");
    }
}