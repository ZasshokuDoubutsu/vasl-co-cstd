#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "vasl_co.h"

/*
 * 一个协作式并发示例：
 *
 *   高优先级  alarm   —— 等待按键，1 s 无事件则打印 idle
 *   中优先级  sensor  —— 每 500 ms 采样一次，并 post 信号量
 *   低优先级  console —— 等信号量，200 ms 超时；有样本就打印
 *   低优先级  blink   —— 每 300 ms 翻转一次 LED
 *
 * 所有任务都运行在同一个线程里，由 vasl_coops.run() 轮转。
 * 这不是抢占式并发：每个任务必须在 wait/period/yield 处主动让出。
 */

static uint32_t g_tick_ms;
static unsigned g_alarm_events;
static unsigned g_alarm_timeouts;
static unsigned g_samples;
static unsigned g_console_timeouts;
static unsigned g_blink_count;

static int g_button_event;
static int g_button_sent;
static int g_led_on;
static int g_temp_x10 = 242;
static int g_console_timeout;

static vasl_co_sem_t g_sample_sem;

uint32_t vasl_get_tick_ms(void)
{
    return g_tick_ms;
}

static void led_toggle(void)
{
    g_led_on = !g_led_on;
    printf("[%5u ms] blink  : LED %s\n", (unsigned)g_tick_ms, g_led_on ? "ON" : "OFF");
}

static int alarm_task(void *arg)
{
    vasl_co_setup_arg(arg);

    vasl_co_begin();
    for (;;) {
        vasl_co_wait_timeout_ms(g_button_event != 0, 1000U, {
            ++g_alarm_timeouts;
            printf("[%5u ms] alarm  : 1 s idle, keep watching\n", (unsigned)g_tick_ms);
        });

        if (g_button_event) {
            g_button_event = 0;
            ++g_alarm_events;
            printf("[%5u ms] alarm  : button -> buzzer 80 ms\n", (unsigned)g_tick_ms);
        }
    }
    vasl_co_end;
}

static int sensor_task(void *arg)
{
    vasl_co_setup_arg(arg);

    vasl_co_begin();
    for (;;) {
        vasl_co_period_ms(500U);

        g_temp_x10 += 3;
        if (g_temp_x10 > 280) {
            g_temp_x10 = 240;
        }
        ++g_samples;

        printf("[%5u ms] sensor : sample #%u, %d.%d C\n",
               (unsigned)g_tick_ms, g_samples,
               g_temp_x10 / 10, g_temp_x10 % 10);

        vasl_co_sem_post(g_sample_sem);

        if (g_samples >= 5U) {
            printf("[%5u ms] sensor : 5 samples done, exit\n", (unsigned)g_tick_ms);
            vasl_co_exit;
        }
    }
    vasl_co_end;
}

static int console_task(void *arg)
{
    vasl_co_setup_arg(arg);

    vasl_co_begin();
    for (;;) {
        g_console_timeout = 0;
        vasl_co_sem_wait_timeout_ms(g_sample_sem, 200U, {
            g_console_timeout = 1;
        });

        if (g_console_timeout) {
            ++g_console_timeouts;
            printf("[%5u ms] console: wait sample timeout\n", (unsigned)g_tick_ms);
        } else {
            printf("[%5u ms] console: consume sample #%u\n",
                   (unsigned)g_tick_ms, g_samples);
        }

        if (g_samples >= 5U) {
            printf("[%5u ms] console: sensor finished, exit\n", (unsigned)g_tick_ms);
            vasl_co_exit;
        }
    }
    vasl_co_end;
}

static int blink_task(void *arg)
{
    vasl_co_setup_arg(arg);

    vasl_co_begin();
    for (;;) {
        vasl_co_period_ms(300U);
        led_toggle();
        if (++g_blink_count >= 8U) {
            printf("[%5u ms] blink  : 8 toggles done, exit\n", (unsigned)g_tick_ms);
            vasl_co_exit;
        }
    }
    vasl_co_end;
}

static int add_group_ok(vasl_co_scheduler_t *sch, uint32_t prio)
{
    int rc = vasl_coops.add_group(sch, prio, NULL);

    /* add_group 在成功占用空槽位时返回 SCH_PRI_REPLACED。 */
    return (rc == VASL_CO_ERR__OK || rc == VASL_CO_ERR__SCH_PRI_REPLACED) ? 0 : rc;
}

static uint32_t elapsed_ms(clock_t start)
{
    return (uint32_t)((uint64_t)(clock() - start) * 1000U / CLOCKS_PER_SEC);
}

int main(void)
{
    VASL_CO_CREATE_SCHE(sch_high, 1U);
    VASL_CO_CREATE_SCHE(sch_mid, 1U);
    VASL_CO_CREATE_SCHE(sch_low, 2U);

    vasl_co_task_handle_t h_alarm;
    vasl_co_task_handle_t h_sensor;
    vasl_co_task_handle_t h_console;
    vasl_co_task_handle_t h_blink;

    clock_t start;

    vasl_co_sem_init(g_sample_sem, 0U);

    if (add_group_ok(&sch_high, 0U) != 0 ||
        add_group_ok(&sch_mid, 1U) != 0 ||
        add_group_ok(&sch_low, 2U) != 0) {
        fprintf(stderr, "add_group failed\n");
        return 1;
    }

    if (vasl_coops.add(&sch_high, alarm_task, NULL, &h_alarm) != VASL_CO_ERR__OK ||
        vasl_coops.add(&sch_mid, sensor_task, NULL, &h_sensor) != VASL_CO_ERR__OK ||
        vasl_coops.add(&sch_low, console_task, NULL, &h_console) != VASL_CO_ERR__OK ||
        vasl_coops.add(&sch_low, blink_task, NULL, &h_blink) != VASL_CO_ERR__OK) {
        fprintf(stderr, "add task failed\n");
        return 1;
    }

    puts("priority: alarm(0) > sensor(1) > console/blink(2)");
    puts("cooperative: each task yields at wait/period");

    start = clock();
    for (;;) {
        uint32_t now = elapsed_ms(start);
        g_tick_ms = now;

        if (!g_button_sent && now >= 900U) {
            g_button_sent = 1;
            g_button_event = 1;
            printf("[%5u ms] main   : button pressed\n", (unsigned)now);
        }

        vasl_coops.run(false);

        if (now >= 3000U) {
            break;
        }
    }

    printf("\nsummary: alarm_events=%u alarm_timeouts=%u samples=%u console_timeouts=%u blink=%u\n",
           g_alarm_events, g_alarm_timeouts, g_samples, g_console_timeouts, g_blink_count);

    (void)vasl_coops.remove(&h_alarm);
    (void)vasl_coops.remove(&h_sensor);
    (void)vasl_coops.remove(&h_console);
    (void)vasl_coops.remove(&h_blink);

    return 0;
}
