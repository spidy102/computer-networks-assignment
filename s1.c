#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "packet.h"


PACKET *initPacket()
{
    PACKET *pkt = malloc(sizeof(PACKET));
    pkt->seq = -1;
    pkt->size = 0;
    memset(pkt->data, '\0', PACKET_SIZE);
    pkt->isAck = false;
    pkt->isLast = false;
    return pkt;
}

bool drop() {
    int rdm = rand()%10; 
    //printf("rdm:%d\n", rdm);
    if ((float)rdm/10<=PDR) {
        return true;
    }
    return false;
}

int main()
{
    int socket_desc;
    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    struct sockaddr_in server_addr, client_addr;
    char msgServer[2000], msgClient[2000];
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    int temp = bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (temp < 0)
    {
        printf("Error in binding\n");
        return -1;
    }
    printf("Binding successful\n");

    int temp1 = listen(socket_desc, 2);
    if (temp1 < 0)
    {
        printf("Error in listen\n");
    }

    PACKET *pkts[2];

    printf("Listening now\n");
    int state = 1;
    int sizeClient = sizeof(client_addr);
    int clientSocket1 = accept(socket_desc, (struct sockaddr *)&client_addr, &sizeClient);
    int clientSocket2 = accept(socket_desc, (struct sockaddr *)&client_addr, &sizeClient);
    if (clientSocket1 < 0 || clientSocket2 < 0)
    {
        printf("Error in creating client socket\n");
        return -1;
    }
    //printf("hello %d %d\n", clientSocket1, clientSocket2);
    PACKET *pkt11 = initPacket();
    PACKET *pkt22 = initPacket();
    pkt11->isLast = false;
    pkt22->isLast = false;
    int lastOf1 = 0;
    int lastOf2 = 0;
    int expSeqNumber1 = 1;
    int expSeqNumber2 = 1;
    while (lastOf1==0 || lastOf2==0)
    {
        switch (state)
        {
        case 1:
        {
            PACKET *pkt1 = initPacket();
            PACKET *pkt2 = initPacket();
            if (recv(clientSocket2, pkt2, sizeof(PACKET), 0) > 0)
            {
                // discard
                state = 1;
            }
            if (recv(clientSocket1, pkt1, sizeof(PACKET), 0) > 0)
            {
                // send ACK
                if (drop()) {
                    printf("DROP PKT: SEQ=%d\n",pkt1->offset);
                    break;
                }
                if (pkt1->seq!=expSeqNumber1) {
                    break;
                } else {
                    expSeqNumber1++;
                }
                if (pkt1->isLast) {
                    lastOf1 = 1;
                }
                printf("RCVD PACKET: SEQ NO.= %d SIZE= %ld \n", pkt1->offset,strlen(pkt1->data));
                FILE *fp = fopen("output.txt", "w");
                fputs(pkt1->data, fp);
                fputc(',', fp);
                fclose(fp);
                PACKET *ackPacket = initPacket();
                ackPacket->isAck = true;
                ackPacket->seq = pkt1->seq;
                ackPacket->offset=pkt1->offset;
                int bytesSent = send(clientSocket1, ackPacket, sizeof(PACKET), 0);

                if (bytesSent != sizeof(PACKET))
                {
                    printf("Cannot send message to client\n");
                    return -1;
                }
                //printf("sent\n");
                printf("SENT ACK: SEQ= %d\n",ackPacket->offset);

                state = 2;
            }
            pkt11 = pkt1;
            pkt22 = pkt2;
            break;
        }
        case 2:
        {
            PACKET *pkt1 = initPacket();
            PACKET *pkt2 = initPacket();
            if (recv(clientSocket1, pkt1, sizeof(PACKET), 0) > 0)
            {
                // discard
                state = 2;
            }
            if (recv(clientSocket2, pkt2, sizeof(PACKET), 0) > 0)
            {
                if (drop()) {
                    printf("DROP PKT: SEQ=%d\n",pkt2->offset);
                    break;
                }
                // send ACK
                if (pkt2->seq!=expSeqNumber2) {
                    break;
                } else {
                    expSeqNumber2++;
                }
                if (pkt2->isLast) {
                    lastOf2 = 1;
                }
                printf("RCVD PACKET: SEQ NO.= %d SIZE= %ld \n", pkt2->offset,strlen(pkt2->data));
                FILE *fp = fopen("output.txt", "w");
                fputs(pkt2->data, fp);
                fputc(',', fp);
                fclose(fp);
                PACKET *ackPacket = initPacket();
                ackPacket->isAck = true;
                ackPacket->seq = pkt2->seq;
                ackPacket->offset=pkt1->offset;
                int bytesSent = send(clientSocket2, ackPacket, sizeof(PACKET), 0);

                if (bytesSent != sizeof(PACKET))
                {
                    printf("Cannot send message to client\n");
                    return -1;
                }
                //printf("sent\n");
                printf("SENT ACK: SEQ= %d\n",ackPacket->offset);
                state = 1;
            }
            pkt11 = pkt1;
            pkt22 = pkt2;
            break;
        }
        }
        // for (int i = 0; i < 2; i++)
        // {
        //     int sizeClient = sizeof(client_addr);
        //     int clientSocket = accept(socket_desc, (struct sockaddr *)&client_addr, &sizeClient);

        //     if (clientSocket < 0)
        //     {
        //         printf("Error in client socket\n");
        //         return -1;
        //     }
        //     printf("Handling client %s\n", inet_ntoa(client_addr.sin_addr));

        //     PACKET *pkt = initPacket();
        //     pkts[i] = pkt;
        //     int temp2 = recv(clientSocket, pkt, sizeof(PACKET), 0);

        //     if (temp2 < 0)
        //     {
        //         printf("Failed to recv from client\n");
        //         return -1;
        //     }

        //     printf("Got message '%s' from client\n", pkt->data);
        //     FILE *fp = fopen("output.txt", "a");
        //     fputs(pkt->data, fp);
        //     fputc(',', fp);
        //     fclose(fp);

        //     PACKET *ackPacket = initPacket();
        //     ackPacket->isAck = true;
        //     ackPacket->seq = pkt->seq;
        //     int bytesSent = send(clientSocket, ackPacket, sizeof(PACKET), 0);

        //     if (bytesSent != sizeof(PACKET))
        //     {
        //         printf("Cannot send message to client\n");
        //         return -1;
        //     }
        //     close(clientSocket);
        // }
        // if (pkts[0]->isLast && pkts[1]->isLast)
        // {
        //     break;
        // }
    }
    close(socket_desc);
}