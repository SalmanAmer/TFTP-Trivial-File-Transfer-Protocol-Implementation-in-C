#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_file_transfer.h"

#define SERVER_IP "127.0.0.1"
#define FILENAME_MAX_LENGTH 100


int sockfd;
struct sockaddr_in server_addr;


int main() {
    Init_Connect();

    while (1) {
        char filename[FILENAME_MAX_LENGTH];
        printf("Enter filename: ");
        scanf("%s", filename);
        filename[strcspn(filename, "\n")] = '\0';

        printf("Enter Opcode: \n0)Read\n1)Write\n2)Delete\n>>");
        int opcode;
        scanf("%d", &opcode);

        switch (opcode) {
            case RRQ:
                Send_rrq(filename);
                break;
            case WRQ:
                Send_wrq(filename);
                break;
            case DEL:
                Send_del(filename);
                break;
            default:
                printf("Invalid opcode!\n");
        }
    }

    close(sockfd);
    return 0;
}
 // Initialize socket and connect to server
void Init_Connect() {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
}
 // Send Read Request (RRQ)
void Send_rrq(const char* filename) {
    TftpPacket packet;
    memset(&packet, 0, sizeof(packet));

    packet.opcode = htons(RRQ);
    strcpy(packet.buffer, filename);

    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Sendto Failed");
        exit(EXIT_FAILURE);
    }

    

    socklen_t server_len = sizeof(server_addr);
    recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&server_addr, &server_len);

    if(ntohs(packet.opcode) == ERR){
        perror("Read Failed");
        return;
    }
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Open failed");
        return;
    }
    while (1) {
        int n = recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&server_addr, &server_len);
        if (n < 0) {
            perror("Receive failed");
            break;
        }

        if (ntohs(packet.opcode) == DATA) {
            fwrite(packet.buffer, 1, n - 4, file);
            fflush(file);

            packet.opcode = htons(ACK);
            sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&server_addr, server_len);

            if (n < BLOCK_SIZE + 4) break;  // Last block
        }else{
    
            perror("Read Failed");
            break;
        }
    }

    fclose(file);
}
 // Send Write Request (WRQ)
void Send_wrq(const char* filename) {
    TftpPacket packet;
    socklen_t server_len = sizeof(server_addr);
    memset(&packet, 0, sizeof(packet));
    FILE *file = fopen(filename, "rb");
    if (!file) {
    packet.opcode = htons(ERR);
    sendto(sockfd, &packet, 4, 0, (const struct sockaddr *)&server_addr, server_len);
        perror("Open failed");
        return;
    }
    packet.opcode = htons(WRQ);
    strcpy(packet.buffer, filename);

    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Sendto Failed");
        exit(EXIT_FAILURE);
    }


    recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&server_addr, &server_len);

    if (ntohs(packet.opcode) == ACK) {
       

        uint16_t block = 1;
        int bytes_read;

        while ((bytes_read = fread(packet.buffer, 1, BLOCK_SIZE, file)) > 0) {
            packet.opcode = htons(DATA);
            packet.block = htons(block);

            sendto(sockfd, &packet, bytes_read + 4, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

            do {
                recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&server_addr, &server_len);
            } while (ntohs(packet.opcode) != ACK || ntohs(packet.block) != block);

            block++;
        }

        fclose(file);
    }
}
 // Send Delete Request (DEL)
void Send_del(const char* filename) {
    TftpPacket packet;
    socklen_t server_len = sizeof(server_addr);
    memset(&packet, 0, sizeof(packet));

    packet.opcode = htons(DEL);
    strcpy(packet.buffer, filename);

    if (sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Sendto Failed");
        exit(EXIT_FAILURE);
    }

    recvfrom(sockfd, &packet, sizeof(TftpPacket), 0, (struct sockaddr *)&server_addr, &server_len);
    if (ntohs(packet.opcode) == ACK) {
        printf("The file %s has been deleted.\n", filename);
    }
    else{
        if(ntohs(packet.opcode) == ERR){
            perror("Deletion failed");
        }
    }


}