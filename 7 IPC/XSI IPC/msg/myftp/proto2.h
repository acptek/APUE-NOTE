//
// Created by acptek on 2020/4/7.
//

#ifndef IPC_PROTO2_H
#define IPC_PROTO2_H

#define KEYPATH     "/etc/services"
#define KEYPROJ     'a'

#define PATHMAX     1024
#define DATAMAX     1024

// 传输过程中产生的包

enum {
    MSG_PATH = 1,
    MSG_DATA
};

typedef struct msg_path_st{
    long mtype;         /* must be MSG_PATH */
    char path[PATHMAX]; /* ASCIIZ  带尾0的串*/
}msg_path_t;

typedef struct msg_s2c_st{
    long mtype;         /* must be MSG_DATA or MSG_EOT*/
    /*
     * datalen > 0      :data
     *         ==0      :eot
     */
    int datalen;
    char data[DATAMAX];
}msg_data_t;

#endif //IPC_PROTO2_H
