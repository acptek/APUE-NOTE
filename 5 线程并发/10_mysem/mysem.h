#ifndef THREAD_MYSEM_H
#define THREAD_MYSEM_H

typedef void mysem_t;

/*
 * 资源总量
 */
mysem_t *mysem_init(int initval);

int mysem_add(mysem_t *, int);

int mysem_sub(mysem_t *, int);

int mysem_destroy(mysem_t *);

#endif //THREAD_MYSEM_H
