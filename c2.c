#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"


int seq = 0;

PACKET *initPacket()
{
    PACKET *pkt = malloc(sizeof(PACKET));
    pkt->seq = -1;
    pkt->size = 0;
    memset(pkt->data, '\0', PACKET_SIZE);
    pkt->isAck = false;
    pkt->isLast = false;
    pkt->offset=0;
    return pkt;
}
bool drop() {
    int rdm = rand()%10; 
    printf("rdm:%d\n", rdm);
    if ((float)rdm/10<=PDR) {
        return true;
    }
    return false;
}


int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Client socket created\n");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Address assigned\n");

    int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (c < 0)
    {
        printf("Error while establishing connection\n");
        return -1;
    }
    printf("Connection established\n");

    printf("Send message for server:\n");
    FILE *fp = fopen("id.txt", "r");
    int fileOffset = 0;
    int state = 1;
    PACKET *gloPkt;
    int flag = 0;
    bool flag1 = true;
    while (flag1)
    {
        switch (state)
        {
        case 1:
        {
            char c;
            char buffer[PACKET_SIZE];
            int i = 0;
            //printf("hello\n");
            int curOffset = fileOffset;
            PACKET *pkt = initPacket();
            pkt->seq = ++seq;
            pkt->offset=ftell(fp);
            gloPkt = pkt;
            while (1)
            {
                c = fgetc(fp);
                if (c == ',')
                {
                    break;
                }
                else if (c == '.')
                {
                    flag = 1;
                    pkt->isLast = true;
                    state  =2;
                    break;
                }
                else
                {
                    buffer[i] = c;
                    i++;
                    curOffset++;
                }
            }
            buffer[i] = '\0';
            strcpy(pkt->data, buffer);
            int bytesSent = send(sock, pkt, sizeof(PACKET), 0);
            if (bytesSent != sizeof(PACKET))
            {
                printf("Error while sending message\n");
                return -1;
            }

            printf("SENT PKT: SEQ= %d SIZE= %ld DATA IS %s\n",pkt->offset,strlen(pkt->data),pkt->data);
            state = 2;
            break;
        }
        case 2:
        {
            PACKET *ackPacket = initPacket();
            ackPacket->isAck = true;
            ackPacket->seq = gloPkt->seq;
            fd_set rfds;
            struct timeval tv;
            int temp9;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            tv.tv_sec = TIMER;
            tv.tv_usec = 0;
            temp9 = select(sock + 1, &rfds, NULL, NULL, &tv);
            if (temp9 == 0)
            {
                //printf("Timeout\n");
                int bytesSent = send(sock, gloPkt, sizeof(PACKET), 0);

                if (bytesSent != sizeof(PACKET))
                {
                    printf("Error while sending message\n");
                    return -1;
                }
                printf("RETRANSMIT PKT SEQ= %d SIZE= %ld DATA IS %s\n",gloPkt->offset,strlen(gloPkt->data),gloPkt->data);
                state = 2;
                break;
            }
            
            int bytesRecvd = recv(sock, ackPacket, sizeof(PACKET), 0);
            if (bytesRecvd < 0)
            {
                printf("Error receiving from server\n");
                return -1;
            }
            // if (drop()) {
            //         printf("ACK dropped\n");
            //         break;
            //     }
            state = 1;

            if (flag==1) {
                flag1 = false;
            }
            break;
        }
        default:
            break;
        }
    }
    // while (!feof(fp))
    // {
    //     char c;
    //     char buffer[PACKET_SIZE];
    //     int i = 0;
    //     printf("hello\n");
    //     int curOffset = fileOffset;
    //     PACKET *pkt = initPacket();
    //     pkt->seq = ++seq;
    //     while (1)
    //     {
    //         c = fgetc(fp);
    //         if (c == ',')
    //         {
    //             break;
    //         }
    //         else if (c == '.')
    //         {
    //             pkt->isLast = true;
    //             break;
    //         }
    //         else
    //         {
    //             buffer[i] = c;
    //             i++;
    //             curOffset++;
    //         }
    //     }

    //     strcpy(pkt->data, buffer);
    //     int bytesSent = send(sock, pkt, sizeof(PACKET), 0);
    //     if (bytesSent != sizeof(PACKET))
    //     {
    //         printf("Error while sending message\n");
    //         return -1;
    //     }

    //     printf("Message sent\n");

    //     PACKET *ackPacket = initPacket();

    //     ackPacket->isAck = true;
    //     ackPacket->seq = pkt->seq;
    //     fd_set rfds;
    //     struct timeval tv;
    //     int temp9;
    //     FD_ZERO(&rfds);
    //     FD_SET(sock, &rfds);
    //     tv.tv_sec = TIMER;
    //     tv.tv_usec = 0;
    //     temp9 = select(sock + 1, &rfds, NULL, NULL, &tv);
    //     if (temp9 == 0)
    //     {
    //         printf("Timeout\n");
    //         int bytesSent = send(sock, pkt, sizeof(PACKET), 0);
    //         if (bytesSent != sizeof(PACKET))
    //         {
    //             printf("Error while sending message\n");
    //             return -1;
    //         }
    //     }
    //     int bytesRecvd = recv(sock, ackPacket, sizeof(PACKET), 0);

    //     if (bytesRecvd < 0)
    //     {
    //         printf("Error receiving from server\n");
    //         return -1;
    //     }
    // }
    fclose(fp);

    close(sock);
}