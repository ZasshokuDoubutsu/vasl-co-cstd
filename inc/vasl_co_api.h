/*
    ██╗   ██╗ █████╗ ███████╗██╗     
    ██║   ██║██╔══██╗██╔════╝██║     
    ██║   ██║███████║███████║██║     
    ╚██╗ ██╔╝██╔══██║╚════██║██║     
     ╚████╔╝ ██║  ██║███████║███████╗
      ╚═══╝  ╚═╝  ╚═╝╚══════╝╚══════╝
*/
#ifndef VASL_CO_API_H_
#define VASL_CO_API_H_
#pragma once

#include "./vasl_co.h"

//: 自定义别名
//: 与 vasl_co.h 保持同步

#if VASL_CO_USE_DBG
    VASL_CO_DBG_FUNC_DEC
#endif ///< VASL_CO_USE_DBG

///< 类型
// typedef vasl_co_task_t              co_task_t;
// typedef vasl_co_task_func_t         co_task_func_t;
// typedef vasl_co_task_arg_t          co_task_arg_t;
// typedef vasl_co_scheduler_t         co_scheduler_t;
// typedef vasl_co_task_handle_t       co_task_handle_t;
// typedef vasl_co_sem_t               co_sem_t;
// typedef vasl_co_err_t               co_err_t;
// typedef vasl_co_task_ret_t          co_task_ret_t;
// typedef vasl_co_method_t            co_method_t;

///< 常量
#define co_is_used(seq)             vasl_co_is_used(seq)

///< 协程框架
// #define co_create_sche(name, capa)  VASL_CO_CREATE_SCHE(name, capa)
// #define co_init(co_task)            vasl_co_init(co_task)
// #define co_begin(arg)               vasl_co_begin(arg)
// #define co_end                      vasl_co_end

///< 协程让出
// #define co_yield                    vasl_co_yield
// #define co_yield_until(cond)        vasl_co_yield_until(cond)

///< 协程结束
// #define co_exit                     vasl_co_exit
// #define co_restart                  vasl_co_restart

///< 协程等待
// #define co_wait(cond)               vasl_co_wait(cond)
// #define co_wait_while(cond)         vasl_co_wait_while(cond)

///< 信号量
// #define co_sem_init(sem, count)     vasl_co_sem_init(sem, count)
// #define co_sem_wait(sem)            vasl_co_sem_wait(sem)
// #define co_sem_post(sem)            vasl_co_sem_post(sem)

///< 时基相关 需 VASL_CO_TIME_USE_BASE_MS
#if VASL_CO_TIME_USE_BASE_MS

// #define co_yield_until_timeout(cond, ms, on_timeout) vasl_co_yield_until_timeout(cond, ms, on_timeout)
// #define co_wait_timeout_ms(cond, ms, on_timeout) vasl_co_wait_timeout_ms(cond, ms, on_timeout)

// #define co_delay_ms(ms)             vasl_co_delay_ms(ms)
// #define co_period_ms(period)        vasl_co_period_ms(period)

// #define co_sem_wait_timeout_ms(sem, ms, on_timeout) vasl_co_sem_wait_timeout_ms(sem, ms, on_timeout)
#endif ///< VASL_CO_TIME_USE_BASE_MS

///< 调度器
// #define co_add(sch, func, p_user_data, p_handle) vasl_come.add(sch, func, p_user_data, p_handle)
// #define co_remove(p_handle)         vasl_come.remove(p_handle)
// #define co_add_group(sch, priority, p_old) vasl_come.add_group(sch, priority, p_old)
// #define co_run()                    vasl_come.run()

#if VASL_CO_USE_DBG
#define co_add_may_fail(sch, func, pud, phandle, tag) \
    do { const int ret = vasl_coops.add(sch, func, pud, phandle);   \
        if(VASL_CO_ERR__OK != ret)  \
            { (void)VASL_CO_DBG_FUNC("%s > co add fail,  \
                ecode: %d, task: %s, line: %d, file: %s, in func: %s",  \
                tag, ret, #func, __LINE__, __FILE__, __func__); }   \
            VASL_CO_DBG_PRINT_NEWLINE();    \
    } while(0)
#else
#define co_add_may_fail(sch, func, pud, phandle, tag) \
    do { (void)(tag); (void)vasl_coops.add(sch, func, pud, phandle); } while(0)
#endif ///< VASL_CO_USE_DBG

#endif ///< VASL_CO_API_H_
