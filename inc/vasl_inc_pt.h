/*
    ██╗   ██╗ █████╗ ███████╗██╗     
    ██║   ██║██╔══██╗██╔════╝██║     
    ██║   ██║███████║███████║██║     
    ╚██╗ ██╔╝██╔══██║╚════██║██║     
     ╚████╔╝ ██║  ██║███████║███████╗
      ╚═══╝  ╚═╝  ╚═╝╚══════╝╚══════╝
*/
#ifndef VASL_INC_PT_H_
#define VASL_INC_PT_H_
#pragma once

#include "./vasl_co_config.h"

#if defined(VASL_CO_USE_SWITCH) && VASL_CO_USE_SWITCH
    #define LC_INCLUDE "lc-switch.h"
#else
    #define LC_INCLUDE "lc-addrlabels.h"
#endif
#include "protothreads-1.4/pt.h"
#include "protothreads-1.4/pt-sem.h"

#define pri_pt_init(pt)                 PT_INIT(pt)
#define pri_pt_yield(pt)                PT_YIELD(pt)
#define pri_pt_yield_until(pt, cond)    PT_YIELD_UNTIL(pt, cond)

#define pri_pt_wait_until(pt, cond)     PT_WAIT_UNTIL(pt, cond)

#define pri_pt_exit(pt)                 PT_EXIT(pt)
#define pri_pt_restart(pt)              PT_RESTART(pt)

#define pri_pt_sem_init(s, c)           PT_SEM_INIT(s, c)
#define pri_pt_sem_wait(pt, s)          PT_SEM_WAIT(pt, s)
#define pri_pt_sem_signal(pt, s)        PT_SEM_SIGNAL(pt, s)

#define pri_pt_begin(pt)                PT_BEGIN(pt)
#define pri_pt_end(pt)                  PT_END(pt)

#define pri_pt_waiting_0                PT_WAITING  // 0
#define pri_pt_yielded_1                PT_YIELDED  // 1
#define pri_pt_exited_2                 PT_EXITED   // 2
#define pri_pt_ended_3                  PT_ENDED    // 3

typedef struct pt                       vasl_co_task_t;
typedef struct pt_sem                   vasl_co_sem_t;

#endif ///< VASL_INC_PT_H_
