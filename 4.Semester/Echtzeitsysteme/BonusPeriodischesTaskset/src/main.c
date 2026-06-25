/**
 * @file main.c
 * @brief Aufgabe 2 - Periodisches Task-System mit ptask (Buttazzo/Lipari).
 *
 * Es wird ein Taskset aus drei periodischen Tasks mit unterschiedlichen
 * Perioden und Prioritaeten erzeugt. Jeder Task misst pro Aktivierung den
 * Release-Jitter (Abweichung der tatsaechlichen von der nominalen Periode)
 * und schreibt die Werte am Ende in eine CSV-Datei.
 *
 * Zusaetzlich toggelt der schnellste Task einen GPIO-Pin, sodass das
 * periodische Verhalten am Oszilloskop sichtbar ist (libgpiod v2).
 *
 * ptask uebernimmt das periodische Aufwecken (clock_nanosleep, absolut)
 * sowie das Setzen der Scheduling-Policy/Prioritaet.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <gpiod.h>

#include "ptask.h"
#include "ptime.h"

/* ----------------------------- Konfiguration ----------------------------- */

#define NUM_TASKS    3
#define MAX_SAMPLES  200000           /* max. gespeicherte Messwerte pro Task */
#define SEC_IN_NS    1000000000ULL

/* GPIO (wie in Aufgabe 1 ermittelt: Pi 5 -> gpiochip0, Pin 26) */
#define GPIO_CHIP    "/dev/gpiochip0"
#define GPIO_PIN     26

/* Taskset: Periode (ms), Prioritaet (0..99), toggelt GPIO? */
static const int  task_period_ms[NUM_TASKS] = { 10, 20, 50 };
static const int  task_priority [NUM_TASKS] = { 30, 20, 10 };
static const int  task_toggle   [NUM_TASKS] = {  1,  0,  0 };
static const int  TASK_PROCESSOR            = 1;   /* alle Tasks auf Kern 1 */

/* --------------------------- gemeinsame Daten ---------------------------- */

typedef struct {
    uint64_t  period_ns;   /* nominale Periode in ns                        */
    int       toggle_gpio; /* 1 = dieser Task toggelt den Pin               */
    uint64_t *samples;     /* gemessene Inter-Aktivierungszeiten (ns)       */
    int       count;       /* Anzahl gespeicherter Messwerte                */
    int       dmiss;       /* Anzahl Deadline-Misses (von ptask gemeldet)   */
} task_ctx_t;

static volatile sig_atomic_t running = 1;
static task_ctx_t ctx[NUM_TASKS];

/* libgpiod v2 Handle */
static struct gpiod_chip         *gpio_chip = NULL;
static struct gpiod_line_request *gpio_req  = NULL;
static unsigned int               gpio_offset = GPIO_PIN;

/* ------------------------------- Helfer ---------------------------------- */

static inline uint64_t delta_ns(const struct timespec *a, const struct timespec *b) {
    return (uint64_t)(a->tv_sec - b->tv_sec) * SEC_IN_NS
         + (uint64_t)(a->tv_nsec - b->tv_nsec);
}

static void on_sigint(int sig) { (void)sig; running = 0; }

/**
 * @brief GPIO-Pin als Ausgang anfordern (libgpiod v2).
 * @return 0 bei Erfolg, -1 sonst (Programm laeuft dann ohne GPIO weiter).
 */
static int init_gpio(void) {
    gpio_chip = gpiod_chip_open(GPIO_CHIP);
    if (!gpio_chip) { perror("gpiod_chip_open"); return -1; }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &gpio_offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "PeriodicTaskset");

    gpio_req = gpiod_chip_request_lines(gpio_chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    if (!gpio_req) { perror("gpiod_chip_request_lines"); return -1; }
    return 0;
}

static void cleanup_gpio(void) {
    if (gpio_req)  gpiod_line_request_release(gpio_req);
    if (gpio_chip) gpiod_chip_close(gpio_chip);
}

/* ----------------------------- Task-Rumpf -------------------------------- */

/**
 * @brief Gemeinsamer Rumpf aller periodischen Tasks.
 *
 * Pro Periode:
 *   - optional GPIO toggeln (nur der dafuer markierte Task),
 *   - Zeitstempel nehmen und Inter-Aktivierungszeit zur vorigen messen,
 *   - Deadline-Miss von ptask abfragen,
 *   - mit ptask_wait_for_period() bis zum naechsten Periodenbeginn schlafen.
 */
void periodic_task(void) {
    task_ctx_t *c = (task_ctx_t *)ptask_get_argument();

    enum gpiod_line_value pin = GPIOD_LINE_VALUE_INACTIVE;
    struct timespec last, now;
    int first = 1;

    clock_gettime(CLOCK_MONOTONIC, &last);

    while (running) {
        /* GPIO-Toggle fuer Oszilloskop-Messung */
        if (c->toggle_gpio && gpio_req) {
            pin = (pin == GPIOD_LINE_VALUE_INACTIVE)
                      ? GPIOD_LINE_VALUE_ACTIVE
                      : GPIOD_LINE_VALUE_INACTIVE;
            gpiod_line_request_set_value(gpio_req, gpio_offset, pin);
        }

        /* tatsaechliche Zeit seit der vorigen Aktivierung messen */
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (!first && c->count < MAX_SAMPLES) {
            c->samples[c->count++] = delta_ns(&now, &last);
        }
        first = 0;
        last  = now;

        /* von ptask gemeldete Deadline-Misses zaehlen */
        if (ptask_deadline_miss()) {
            c->dmiss++;
        }

        ptask_wait_for_period();
    }
}

/* --------------------------------- main ---------------------------------- */

int main(int argc, char **argv) {
    int run_seconds = (argc > 1) ? atoi(argv[1]) : 15;

    signal(SIGINT, on_sigint);

    if (init_gpio() != 0) {
        fprintf(stderr, "Warnung: GPIO konnte nicht initialisiert werden - "
                        "laufe ohne Oszilloskop-Ausgabe weiter.\n");
    }

    /* ptask initialisieren: SCHED_FIFO, partitioniert, ohne Sema-Protokoll */
    ptask_init(SCHED_FIFO, PARTITIONED, NO_PROTOCOL);

    /* Taskset erzeugen */
    for (int i = 0; i < NUM_TASKS; i++) {
        ctx[i].period_ns   = (uint64_t)task_period_ms[i] * 1000000ULL;
        ctx[i].toggle_gpio = task_toggle[i];
        ctx[i].count       = 0;
        ctx[i].dmiss       = 0;
        ctx[i].samples     = malloc(sizeof(uint64_t) * MAX_SAMPLES);
        if (!ctx[i].samples) { perror("malloc"); return EXIT_FAILURE; }

        tpars p;
        ptask_param_init(p);
        ptask_param_period(p,   task_period_ms[i], MILLI);
        ptask_param_deadline(p, task_period_ms[i], MILLI);
        ptask_param_priority(p, task_priority[i]);
        ptask_param_processor(p, TASK_PROCESSOR);
        ptask_param_activation(p, NOW);
        ptask_param_argument(p, &ctx[i]);

        int id = ptask_create_param(periodic_task, &p);
        if (id < 0) {
            fprintf(stderr, "Fehler: Task %d konnte nicht erstellt werden "
                            "(als root/sudo starten!)\n", i);
            return EXIT_FAILURE;
        }
        printf("Task %d erstellt: T=%d ms, prio=%d, core=%d%s\n",
               i, task_period_ms[i], task_priority[i], TASK_PROCESSOR,
               task_toggle[i] ? ", GPIO-Toggle" : "");
    }

    printf("\nLaufe %d Sekunden ... (Strg+C beendet frueher)\n", run_seconds);
    for (int s = 0; s < run_seconds && running; s++) sleep(1);
    running = 0;

    /* den Tasks Zeit geben, die laufende Periode zu beenden */
    sleep(1);

    /* Ergebnisse je Task als CSV schreiben und Statistik ausgeben */
    printf("\n=== Ergebnisse ===\n");
    for (int i = 0; i < NUM_TASKS; i++) {
        char fn[64];
        snprintf(fn, sizeof(fn), "task%d_%dms.csv", i, task_period_ms[i]);
        FILE *f = fopen(fn, "w");
        if (f) {
            for (int k = 0; k < ctx[i].count; k++) {
                fprintf(f, "%llu\n", (unsigned long long)ctx[i].samples[k]);
            }
            fclose(f);
        }
        printf("Task %d (T=%2d ms, prio=%2d): %6d Samples, Deadline-Misses=%d -> %s\n",
               i, task_period_ms[i], task_priority[i],
               ctx[i].count, ctx[i].dmiss, fn);
        free(ctx[i].samples);
    }

    cleanup_gpio();
    return EXIT_SUCCESS;
}
