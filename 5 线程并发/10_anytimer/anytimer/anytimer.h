#ifndef MYTBF_ANYTIMER_H
#define MYTBF_ANYTIMER_H

#define JOB_MAX 1024

typedef void at_jobfunc_t(void*);

int at_addjob(int sec, at_jobfunc_t *jobp, void *arg);
/*
 * return >= 0       success, 返回任务ID
 *        == -EINVAL fail,参数非法
 *        == -ENOSPC fail,数组满
 *        == -ENOMEM fail,内存空间不足
 */

//int at_canceljob(int id);
///*
// * return == 0       success, 成功指定任务成功取消
// *        == -EINVAL fail, 参数非法
// *        == -EBUSY  fail, 指定任务已完成
// *        == ECANCELED fail, 指定任务重复取消
// */

int at_waitjob(int id);
/*
 * return == 0       sucess, 指定任务已成功释放
 *        == -EINVAL fail,参数非法
 */
//
//int at_pausejob(int id);
//
//int at_resumejob(int id);

#endif //MYTBF_ANYTIMER_H
