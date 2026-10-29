/*
    ██╗   ██╗ █████╗ ███████╗██╗     
    ██║   ██║██╔══██╗██╔════╝██║     
    ██║   ██║███████║███████║██║     
    ╚██╗ ██╔╝██╔══██║╚════██║██║     
     ╚████╔╝ ██║  ██║███████║███████╗
      ╚═══╝  ╚═╝  ╚═╝╚══════╝╚══════╝
*/
#ifndef VASL_CO_CONFIG_H_
#define VASL_CO_CONFIG_H_
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* 公共开关：可在编译命令行或包含本头文件前覆盖。 */
#ifndef VASL_CO_USE_SWITCH
    #define VASL_CO_USE_SWITCH              (0U)    /* 0: lc-addrlabels.h (GCC label 扩展) / 1: lc-switch.h */
#endif

#ifndef VASL_CO_GROUP_PRIORITY_MAX
    #define VASL_CO_GROUP_PRIORITY_MAX      (3U)    /* 0 为最高优先级 */
#endif

#ifndef VASL_CO_TIME_USE_BASE_MS
    #define VASL_CO_TIME_USE_BASE_MS        (1U)
#endif

#if VASL_CO_TIME_USE_BASE_MS
    #ifndef VASL_CO_TIME_BASE_MS
        #define VASL_CO_TIME_BASE_MS        vasl_get_tick_ms
    #endif
    #ifndef VASL_CO_TIME_BASE_MS_DEC
        #define VASL_CO_TIME_BASE_MS_DEC    extern uint32_t VASL_CO_TIME_BASE_MS(void);
    #endif
#endif

#ifndef VASL_CO_TASK_USE_MAIN
    #define VASL_CO_TASK_USE_MAIN           (1U)
#endif
#if VASL_CO_TASK_USE_MAIN
    #ifndef VASL_CO_TASK_MAIN_MAX_TASKS
        #define VASL_CO_TASK_MAIN_MAX_TASKS (10U)
    #endif
#endif

#ifndef VASL_CO_USE_DBG
    #define VASL_CO_USE_DBG                 (1U)
#endif
#if VASL_CO_USE_DBG
    #ifndef VASL_CO_DBG_BUF_MIN_SIZE
        #define VASL_CO_DBG_BUF_MIN_SIZE    (35U)
    #endif
    #ifndef VASL_CO_DBG_FUNC
        #define VASL_CO_DBG_FUNC            printlntick
    #endif
    #ifndef VASL_CO_DBG_FUNC_DEC
        #define VASL_CO_DBG_FUNC_DEC        extern int VASL_CO_DBG_FUNC(const char*, ...);
    #endif
    #ifndef VASL_CO_DBG_PRINT_NEWLINE
        #define VASL_CO_DBG_PRINT_NEWLINE() VASL_CO_DBG_FUNC("\n")
    #endif
    #ifndef VASL_CO_DBG_STRING
        #define VASL_CO_DBG_STRING          "SchePri:%u,TaskId:%u,Ret:%d\n"
    #endif
#endif

/* 基础类型与空指针。 */
#define VASL_CO_BOOL                        bool
#define VASL_CO_NULL                        NULL
#define VASL_CO_FALSE                       (false)
#define VASL_CO_TRUE                        (true)

typedef uint32_t PRI_VASL_CO_U32;

/* C99 可用的编译期断言：typedef 数组长度为负会触发编译错误。 */
#define VASL_CO_STATIC_ASSERT_CONCAT_(a, b) a##b
#define VASL_CO_STATIC_ASSERT_CONCAT(a, b)  VASL_CO_STATIC_ASSERT_CONCAT_(a, b)
#define VASL_CO_STATIC_ASSERT(cond)         \
    typedef char VASL_CO_STATIC_ASSERT_CONCAT(vasl_co_static_assert_, __LINE__)[(cond) ? 1 : -1]

/* 弱符号：让用户可覆盖默认调试钩子。 */
#if defined(__GNUC__) || defined(__clang__)
    #define VASL_CO_WEAK                    __attribute__((weak))
#else
    #define VASL_CO_WEAK
#endif

#endif ///< VASL_CO_CONFIG_H_
