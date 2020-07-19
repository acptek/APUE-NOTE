#ifndef SOCKET_PROTO_H
#define SOCKET_PROTO_H

#include <stdint-gcc.h>

#define NAMEMAX (512-8-8)
#define RCVPORT "1989"

// 指定gcc不对齐
// 变长结构体
struct msg_st{
    uint32_t math;
    uint32_t chinese;
    uint8_t name[1]; // 变长，只作为占位符
}__attribute__((packed));



#endif //SOCKET_PROTO_H
