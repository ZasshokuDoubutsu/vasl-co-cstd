/*
    ██╗   ██╗ █████╗ ███████╗██╗     
    ██║   ██║██╔══██╗██╔════╝██║     
    ██║   ██║███████║███████║██║     
    ╚██╗ ██╔╝██╔══██║╚════██║██║     
     ╚████╔╝ ██║  ██║███████║███████╗
      ╚═══╝  ╚═╝  ╚═╝╚══════╝╚══════╝
*/
#include "./inc/vasl_co.h"
#include "./inc/vasl_co_config.h"

#if VASL_CO_USE_DBG
    #include <stdio.h>
#endif

typedef struct vasl_co_scheduler_group {
    vasl_co_scheduler_t**   p_sches;
    uint32_t                capacity;
} vasl_co_scheduler_group_t;

#define VASL_CO_CREATE_SCHE_GROUP(name, capa)   \
    static vasl_co_scheduler_t* name##Sches[(capa)]; \
    static vasl_co_scheduler_group_t name = {   \
        .p_sches = name##Sches,                 \
        .capacity = capa                        \
    };                                          \
    static uint32_t name##Seq = 0U;

#if VASL_CO_USE_DBG
    VASL_CO_STATIC_ASSERT(sizeof(VASL_CO_DBG_STRING) <= (VASL_CO_DBG_BUF_MIN_SIZE + 5U));
#endif

VASL_CO_CREATE_SCHE_GROUP(CoScheGroup, VASL_CO_GROUP_PRIORITY_MAX);

#if VASL_CO_TASK_USE_MAIN
    VASL_CO_CREATE_SCHE(MainSche, VASL_CO_TASK_MAIN_MAX_TASKS);
#endif ///< VASL_CO_TASK_USE_MAIN

// vasl_attr(nonnull(1, 2)) 
static int co_add(vasl_co_scheduler_t* sch, vasl_co_task_func_t func, void* p_user_data, vasl_co_task_handle_t* p_handle) {
    if(VASL_CO_NULL == sch) { return VASL_CO_ERR__PSCH_NULL; }
    if(VASL_CO_NULL == func) { return VASL_CO_ERR__PFUNC_NULL; }
    if(VASL_CO_NULL == sch->p_entrys) { return VASL_CO_ERR__SCH_ENTRY_NULL; }
    vasl_co_task_entry_t* const p_entrys = sch->p_entrys;
    for(uint32_t i = 0U; i < sch->capacity; ++i) {
        if(VASL_CO_SEQ_FREE != p_entrys[i].seq) { continue; }
        if(VASL_CO_SEQ_FREE == ++CoScheGroupSeq) { ++CoScheGroupSeq; }
        p_entrys[i].func = func;
        p_entrys[i].co_arg.p_user_data = p_user_data;
        vasl_co_init(p_entrys[i].co_arg.co_task);
        #if VASL_CO_TIME_USE_BASE_MS
        p_entrys[i].co_arg.tout = 0U;
        p_entrys[i].co_arg.last = VASL_CO_TIME_BASE_MS();
        #endif ///< VASL_CO_TIME_USE_BASE_MS
        p_entrys[i].seq = CoScheGroupSeq;
        if(VASL_CO_NULL != p_handle) {
            p_handle->sch = sch;
            p_handle->id = i;
            p_handle->seq = CoScheGroupSeq;
        }
        return VASL_CO_ERR__OK;
    }
    return VASL_CO_ERR__SCH_CAP_EMPTY;
}

// vasl_attr(nonnull(1)) 
static int co_remove(vasl_co_task_handle_t* const p_handle) {
    if(VASL_CO_NULL == p_handle) { return VASL_CO_ERR__PHANDLE_NULL; }
    vasl_co_scheduler_t* const sch = p_handle->sch;
    if(VASL_CO_NULL == sch) { return VASL_CO_ERR__PSCH_NULL; }
    if(VASL_CO_NULL == sch->p_entrys) { return VASL_CO_ERR__SCH_ENTRY_NULL; }
    if(p_handle->id >= sch->capacity) { return VASL_CO_ERR__SCH_FUNC_NOT_FOUND; }
    vasl_co_task_entry_t* const p_entry = &(sch->p_entrys[p_handle->id]);
    if(VASL_CO_SEQ_FREE == p_entry->seq || p_entry->seq != p_handle->seq) {
        return VASL_CO_ERR__SCH_FUNC_NOT_FOUND;
    }
    p_entry->func = VASL_CO_NULL;
    p_entry->co_arg.p_user_data = VASL_CO_NULL;
    vasl_co_init(p_entry->co_arg.co_task);
    p_entry->seq = VASL_CO_SEQ_FREE;
    return VASL_CO_ERR__OK;
}

// vasl_attr(nonnull(1)) 
static int co_add_group(vasl_co_scheduler_t* sch, uint32_t priority, vasl_co_scheduler_t* p_old) {
    if(VASL_CO_NULL == sch) { return VASL_CO_ERR__PSCH_NULL; }
    if(VASL_CO_NULL == sch->p_entrys) { return VASL_CO_ERR__SCH_ENTRY_NULL; }
    if(VASL_CO_GROUP_PRIORITY_MAX <= priority) { return VASL_CO_ERR__SCH_FUNC_PRI_OVER; }
    vasl_co_scheduler_t** const p_sches = CoScheGroup.p_sches;
    if(VASL_CO_NULL == p_sches[priority]) {
        p_sches[priority] = sch;
        return VASL_CO_ERR__OK;
    }
    if(VASL_CO_NULL != p_old) {
        *p_old = *p_sches[priority];
        return VASL_CO_ERR__SCH_PRI_REPLACED;
    }
    p_sches[priority] = sch;
    return VASL_CO_ERR__SCH_PRI_REPLACED;
}

static void co_run(bool use_dbg) {
    vasl_co_scheduler_t** const p_sches = CoScheGroup.p_sches;
    VASL_CO_BOOL again = VASL_CO_FALSE;
    do {
        for(uint32_t _pri = VASL_CO_TASK_PRI_HIGHEST; _pri < VASL_CO_GROUP_PRIORITY_MAX; ++_pri) {
            vasl_co_scheduler_t* const p_sch = p_sches[_pri];
            if(VASL_CO_NULL == p_sch) { continue; }
            vasl_co_task_entry_t* const p_entrys = p_sch->p_entrys;
            for (uint32_t i = 0U; i < p_sch->capacity; ++i) {
                if (VASL_CO_SEQ_FREE == p_entrys[i].seq) { continue; }
                int ret = p_entrys[i].func(&(p_entrys[i].co_arg));
                #if VASL_CO_USE_DBG
                do { 
                    if(!use_dbg) { break; }
                    char* const buf_ptr = vasl_co_get_dbg_buf_ptr();
                    uint32_t buf_size = vasl_co_get_dbg_buf_size();
                    if (VASL_CO_NULL == buf_ptr || VASL_CO_DBG_BUF_MIN_SIZE > buf_size) { break; }
                    const int fmt_len = snprintf(
                        buf_ptr, buf_size, VASL_CO_DBG_STRING,
                        (unsigned)_pri, (unsigned)i, ret);
                    if(fmt_len < 0 || (uint32_t)fmt_len >= buf_size) { break; }
                    vasl_co_dbg_func(buf_ptr, (uint32_t)fmt_len);
                } while(0);
                #else
                (void)use_dbg;
                #endif ///< VASL_CO_USE_DBG
                if (VASL_CO_RET__EXITED <= ret) {
                    p_entrys[i].seq = VASL_CO_SEQ_FREE;
                } else if (VASL_CO_RET__WAITING != ret) {
                    again = VASL_CO_TRUE;
                }
            }
            if (again) { break; }
        }
    } while (again);
}

VASL_CO_WEAK void       vasl_co_dbg_func        (char buf[], uint32_t len) { (void)buf; (void)len; }
VASL_CO_WEAK char*      vasl_co_get_dbg_buf_ptr (void) { return VASL_CO_NULL; }
VASL_CO_WEAK uint32_t   vasl_co_get_dbg_buf_size(void) { return 0U; }

const vasl_co_method_t vasl_coops = {
    .add        = co_add,
    .remove     = co_remove,
    .add_group  = co_add_group,
    .run        = co_run
};
