#ifndef ADVIO_RELAYER_H
#define ADVIO_RELAYER_H

#include <cstdint>

#define REL_JOBMAX 10000

enum{
    STATE_RUNNIG = 1,
    STATE_CANCELED,
    STATE_OVER
};

// state
struct rel_stat_st{
    int state;
    int fd1;
    int fd2;
    int64_t count1, count2;
    struct timeval start, end;
};

int rel_addjob(int fd1, int fd2);
/*
 * return   >=0     success ,return fd
 *          == -EINVAL
 *          == -ENOSPC
 *          == -ENOMEM
 */

int rel_canceljob(int id);
/*
 * return   ==0     cancel success
 *          == -EINVAL
 *          == -EBUSY
 */

int rel_waitjob(int id, struct rel_stat_st *);
/*
 * return   == 0    success
 *          == -EINVAL
 *
 */


int rel_statjob(int id, struct rel_stat_st *);
/*
 * return   == 0    success
 *          == -EINVAL
 */


#endif //ADVIO_RELAYER_H
