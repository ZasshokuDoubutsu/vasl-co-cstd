/*
    ██╗   ██╗ █████╗ ███████╗██╗     
    ██║   ██║██╔══██╗██╔════╝██║     
    ██║   ██║███████║███████║██║     
    ╚██╗ ██╔╝██╔══██║╚════██║██║     
     ╚████╔╝ ██║  ██║███████║███████╗
      ╚═══╝  ╚═╝  ╚═╝╚══════╝╚══════╝
*/
#ifndef VASL_CO_H_
#define VASL_CO_H_
#pragma once

#include "./vasl_co_config.h"
#include "./vasl_inc_pt.h"

#if VASL_CO_TIME_USE_BASE_MS
    VASL_CO_TIME_BASE_MS_DEC
#endif ///< VASL_CO_TIME_USE_BASE_MS

#define VASL_CO_TASK_PRI_HIGHEST    (0U)
#define VASL_CO_SEQ_FREE            (0U)
#define vasl_co_is_used(seq)        (VASL_CO_SEQ_FREE != (seq))

typedef enum vasl_co_err {
    VASL_CO_ERR__OK                 = 0,
    VASL_CO_ERR__PSCH_NULL          = -1,
    VASL_CO_ERR__PFUNC_NULL         = -2,
    VASL_CO_ERR__PHANDLE_NULL       = -3,
    VASL_CO_ERR__SCH_CAP_EMPTY      = -4,
    VASL_CO_ERR__SCH_PRI_REPLACED   = -5,
    VASL_CO_ERR__SCH_FUNC_NOT_FOUND = -6,
    VASL_CO_ERR__SCH_FUNC_PRI_OVER  = -7,
    VASL_CO_ERR__SCH_ENTRY_NULL     = -8,
} vasl_co_err_t;

typedef enum vasl_co_task_ret {
    VASL_CO_RET__WAITING            = pri_pt_waiting_0,
    VASL_CO_RET__YIELDED            = pri_pt_yielded_1,
    VASL_CO_RET__EXITED             = pri_pt_exited_2,
    VASL_CO_RET__ENDED              = pri_pt_ended_3,
    VASL_CO_RET__MAX_VALUE
} vasl_co_task_ret_t;

typedef int (*vasl_co_task_func_t)(void*);
typedef void (*vasl_co_dbg_func_t)(const char[]);

typedef struct vasl_co_task_arg {
    void*                   p_user_data;
    vasl_co_task_t          co_task;
#if VASL_CO_TIME_USE_BASE_MS
    uint32_t                tout;
    uint32_t                last;
#endif ///< VASL_CO_TIME_USE_BASE_MS
} vasl_co_task_arg_t;

typedef struct vasl_co_task_entry {
    vasl_co_task_func_t     func;
    vasl_co_task_arg_t      co_arg;
    uint32_t                seq;
} vasl_co_task_entry_t;

typedef struct vasl_co_scheduler {
    vasl_co_task_entry_t*   p_entrys;
    uint32_t                capacity;
} vasl_co_scheduler_t;

typedef struct vasl_co_task_handle {
    vasl_co_scheduler_t*    sch;
    uint32_t                id;
    uint32_t                seq;
} vasl_co_task_handle_t;

typedef struct vasl_co_method {
    int  (*const add)       (vasl_co_scheduler_t*   sch, 
                             vasl_co_task_func_t    func, 
                             void*                  p_user_data, 
                             vasl_co_task_handle_t* p_handle);
    int  (*const remove)    (vasl_co_task_handle_t* p_handle);
    int  (*const add_group) (vasl_co_scheduler_t*   sch, 
                             uint32_t               priority, 
                             vasl_co_scheduler_t*   p_old);
    void (*const run)       (bool                   use_dbg);
} vasl_co_method_t;

#define pri_vasl_co_pt      (&pri_vasl_co_arg->co_task)
#ifndef SEC2MS
    #define SEC2MS(sec)     ((sec) * 1000U)
#endif ///< SEC2MS
#ifndef MIN2MS
    #define MIN2MS(min)     ((min) * 1000U * 60UL)
#endif ///< MIN2MS

///< 协程初始化
#define vasl_co_init(co_task)                   \
    pri_pt_init(&co_task)

///< 协程框架
#define vasl_co_setup_arg(arg_)                 \
    vasl_co_task_arg_t* const pri_vasl_co_arg   \
        = (vasl_co_task_arg_t*)(arg_);

#define vasl_co_unpack_arg(arg_, pud_, type)    \
    vasl_co_setup_arg(arg_)                     \
    type const pud_                             \
        = (type)(&pri_vasl_co_arg->p_user_data);\
    (void)pud_;

#define vasl_co_begin(run_mode)                 \
    do { pri_pt_begin(pri_vasl_co_pt);          \
    (void)PT_YIELD_FLAG; run_mode {

#define vasl_co_end                             \
    } pri_pt_end(pri_vasl_co_pt) } while(0)

///< 协程让出CPU
#define vasl_co_yield                           \
    pri_pt_yield(pri_vasl_co_pt)

///< 协程让出CPU 并判断为真时继续执行
#define vasl_co_yield_until(cond)               \
    pri_pt_yield_until(pri_vasl_co_pt, cond)

///< 协程让出CPU 并判断为真或超时时继续执行 超时执行 on_timeout
#if VASL_CO_TIME_USE_BASE_MS
#define vasl_co_yield_until_timeout(cond, ms, on_timeout) \
    do {pri_vasl_co_arg->tout = VASL_CO_TIME_BASE_MS(); \
        for (;;) { \
            if (cond) { break; } \
            if ((VASL_CO_TIME_BASE_MS() - pri_vasl_co_arg->tout) >= (uint32_t)(ms)) { \
                on_timeout; break; \
            } \
            vasl_co_yield; \
        } \
    } while(0)
#endif ///< VASL_CO_TIME_USE_BASE_MS

///< 协程手动彻底结束
#define vasl_co_exit                            \
    pri_pt_exit(pri_vasl_co_pt)

///< 协程退出 下次重新启动
#define vasl_co_restart                         \
    pri_pt_restart(pri_vasl_co_pt)

///< 协程等待直到条件成立
#define vasl_co_wait(cond) 				        \
    pri_pt_wait_until(pri_vasl_co_pt, (cond))

#define vasl_co_wait_while(cond)                \
    pri_pt_wait_until(pri_vasl_co_pt, !(cond))

#if VASL_CO_TIME_USE_BASE_MS
#define vasl_co_wait_timeout_ms(cond, ms, on_timeout)   \
    do { pri_vasl_co_arg->tout = VASL_CO_TIME_BASE_MS();    \
        vasl_co_wait((cond) || ((VASL_CO_TIME_BASE_MS() - pri_vasl_co_arg->tout) >= (uint32_t)(ms))); \
        if (!(cond)) { on_timeout; }    \
    } while(0)
#endif ///< VASL_CO_TIME_USE_BASE_MS

#if VASL_CO_TIME_USE_BASE_MS
///< 协程任务延时
#define vasl_co_delay_ms(ms)                    \
    do { pri_vasl_co_arg->tout = VASL_CO_TIME_BASE_MS();    \
        vasl_co_wait((VASL_CO_TIME_BASE_MS() - pri_vasl_co_arg->tout) >= (uint32_t)(ms)); \
    } while (0)

///< 协程周期任务 以上次唤醒时刻为基准 到点继续
#define vasl_co_period_ms(period)               \
    do { vasl_co_wait((VASL_CO_TIME_BASE_MS() - pri_vasl_co_arg->last) >= (uint32_t)(period)); \
        pri_vasl_co_arg->last = VASL_CO_TIME_BASE_MS(); \
    } while (0)

///< 协程信号量等待（带超时，超时后继续执行，不消耗信号量）
#define vasl_co_sem_wait_timeout_ms(sem, ms, on_timeout)    \
    do { pri_vasl_co_arg->tout = VASL_CO_TIME_BASE_MS();    \
        vasl_co_wait((&(sem))->count > 0U ||    \
                     (VASL_CO_TIME_BASE_MS() - pri_vasl_co_arg->tout) >= (uint32_t)(ms)); \
        if ((&(sem))->count > 0U) { --(&(sem))->count; }    \
        else { on_timeout; }    \
    } while (0)
#endif ///< VASL_CO_TIME_USE_BASE_MS

///< 协程信号量初始化
#define vasl_co_sem_init(sem, count)            \
    pri_pt_sem_init(&(sem), (count))

///< 协程信号量等待
#define vasl_co_sem_wait(sem)                   \
    pri_pt_sem_wait(pri_vasl_co_pt, &(sem))

///< 协程信号量发送
#define vasl_co_sem_post(sem)                   \
    pri_pt_sem_signal(pri_vasl_co_pt, &(sem))

#define VASL_CO_CREATE_SCHE(name, capa)         \
    static vasl_co_task_entry_t name##_entry[(capa)]; \
    vasl_co_scheduler_t name = {                \
        .p_entrys = name##_entry,               \
        .capacity = (capa)                      \
    };

#define VASL_CO_CREATE_DBG(dbg_flush, buf_size) \
    VASL_CO_STATIC_ASSERT((buf_size) >= (VASL_CO_DBG_BUF_MIN_SIZE + 5U)); \
    void vasl_co_dbg_func(char buf[], uint32_t len) {\
        { dbg_flush }                           \
    }                                           \
    char* vasl_co_get_dbg_buf_ptr(void) {       \
        static char buf[buf_size] = {0};        \
        return buf;                             \
    }                                           \
    uint32_t vasl_co_get_dbg_buf_size(void) {   \
        return (uint32_t)(buf_size);            \
    }

///< Weak
void        vasl_co_dbg_func        (char[], uint32_t);
char*       vasl_co_get_dbg_buf_ptr (void);
uint32_t    vasl_co_get_dbg_buf_size(void);

#if VASL_CO_TASK_USE_MAIN
    extern vasl_co_scheduler_t MainSche;
#endif ///< VASL_CO_TASK_USE_MAIN

extern const vasl_co_method_t vasl_coops;

#endif ///< VASL_CO_H_
