#ifndef PACKET_H
#define PACKET_H

#define PACKET_SIZE 100
#define TIMER 2
#define PDR 0.3
#include <stdbool.h>

typedef struct Packet
{
    int size;
    int seq;
    bool isAck;
    char data[PACKET_SIZE];
    bool isLast;
    int offset;
} PACKET;

#endif
