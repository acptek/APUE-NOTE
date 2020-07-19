#ifndef SOCKET_PROTO_H
#define SOCKET_PROTO_H

#include <stdint-gcc.h>

#define NAMESIZE 11
#define RCVPORT "1989"

// 指定gcc不对齐
struct msg_st{
    uint8_t name[NAMESIZE];
    uint32_t math;
    uint32_t chinese;
}__attribute__((packed));



#endif //SOCKET_PROTO_H
