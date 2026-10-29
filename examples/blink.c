#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "vasl_co.h"

static uint32_t g_tick_ms;
static unsigned g_blink_count;

uint32_t vasl_get_tick_ms(void)
{
    return g_tick_ms;
}

static void led_toggle(void)
{
    static int on;

    on = !on;
    printf("[%5u ms] LED %s\n", (unsigned)g_tick_ms, on ? "ON" : "OFF");
}

static int blink(void *arg)
{
    vasl_co_setup_arg(arg);

    vasl_co_begin();
    for (;;) {
        vasl_co_period_ms(500U);
        led_toggle();
        if (++g_blink_count >= 4U) {
            vasl_co_exit;
        }
    }
    vasl_co_end;
}

int main(void)
{
    VASL_CO_CREATE_SCHE(sch, 1U);
    vasl_co_task_handle_t handle;
    int rc;
    clock_t start;

    rc = vasl_coops.add_group(&sch, 0U, NULL);
    if (rc != VASL_CO_ERR__OK && rc != VASL_CO_ERR__SCH_PRI_REPLACED) {
        fprintf(stderr, "add_group failed: %d\n", rc);
        return 1;
    }

    rc = vasl_coops.add(&sch, blink, NULL, &handle);
    if (rc != VASL_CO_ERR__OK) {
        fprintf(stderr, "add failed: %d\n", rc);
        return 1;
    }

    start = clock();
    while ((unsigned)((clock() - start) * 1000 / CLOCKS_PER_SEC) < 2200U) {
        g_tick_ms = (uint32_t)((uint64_t)(clock() - start) * 1000U / CLOCKS_PER_SEC);
        vasl_coops.run(false);
    }

    (void)vasl_coops.remove(&handle);
    puts("done");
    return 0;
}
