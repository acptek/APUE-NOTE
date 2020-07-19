#ifndef IPC_PROTO_H
#define IPC_PROTO_H

// 约定双方对话格式

#define KEYPATH     "/etc/services"
#define KEYPROJ     'g'

#define NAMESIZE    32

struct msg_st{
    long mtype;
    char name[NAMESIZE];
    int math;
    int chinese;
};

#endif //IPC_PROTO_H
