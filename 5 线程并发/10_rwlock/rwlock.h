//
// Created by acptek on 2020/3/17.
//

#ifndef RWLOCK_RWLOCK_H
#define RWLOCK_RWLOCK_H

typedef void mywrlock;

mywrlock * rwlock_create(void);

int rdlock_lock(mywrlock *lock);

int wrlock_lock(mywrlock * lock);

int rwlock_unlock(mywrlock *lock);

int rdlock_t
rylock(mywrlock *lock);

int wrlock_trylock(mywrlock * lock);

int rwlock_destroy(mywrlock * lock);

#endif //RWLOCK_RWLOCK_H
