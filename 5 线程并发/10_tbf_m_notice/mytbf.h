//
// Created by shan on 2020/2/29.
//

#ifndef MYTBF_MYTBF_H
#define MYTBF_MYTBF_H

#define MYTBF_MAX 1024

typedef void mytbf_t;

mytbf_t * mytbf_init(int cps, int burst);

int mytbf_fetchtoken(mytbf_t *tbf, int size);

int mytbf_returntoken(mytbf_t *tbf, int size);

int mytbf_destroy(mytbf_t *tbf);

#endif //MYTBF_MYTBF_H
