/**
 * 
 */

#include "../inc/main.h"
#include "../inc/ringbuffer.h"

/**
 * 
 * @brief Worker thread that shall toggle a GPIO pin at a specified frequency while logging
 *        the time difference between two consequtive toggles.
 * 
 */
void* func_signal_gen(void* args) {
    thread_args_t* param = (thread_args_t*)args;
    
    /* Stick this thread to specific cpu core */
    stick_thread_to_core(param->core_id);

    /* Set thread priority - only if configured */
    if (param->sched_prio >= 1) {
        set_thread_priority(param->sched_prio);
    }

    /* Store measured time difference as nanoseconds */
    uint64_t time_diff_ns = 0;

    /**
     * Setup before the main loop.
     */

    /* Current GPIO output level. We start LOW and toggle it on every iteration. */
    enum gpiod_line_value pin_value = GPIOD_LINE_VALUE_INACTIVE;

    /* We sleep for half a period; one HIGH phase + one LOW phase = one full period. */
    struct timespec sleep_time = {
        .tv_sec  = (time_t)(param->half_period_ns / SEC_IN_NS),
        .tv_nsec = (long)(param->half_period_ns % SEC_IN_NS)
    };

    /* Timestamps to measure the time elapsed between two consecutive toggles. */
    struct timespec t_last, t_now;
    clock_gettime(CLOCK_MONOTONIC, &t_last);


    /* Main loop for signal generation and time measurement. */
    while (!param->killswitch) {

        /* Wait for one half-period before the next edge. */
        nanosleep(&sleep_time, NULL);

        /* Toggle the GPIO pin: LOW -> HIGH or HIGH -> LOW. */
        pin_value = (pin_value == GPIOD_LINE_VALUE_INACTIVE)
                        ? GPIOD_LINE_VALUE_ACTIVE
                        : GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_value(param->gpio->request,
                                     param->gpio->offset,
                                     pin_value);

        /* Measure the actual time that elapsed since the previous toggle.
         * This is taken right after the toggle so the measurement only
         * covers the toggle interval, not the ringbuffer write below. */
        clock_gettime(CLOCK_MONOTONIC, &t_now);
        time_diff_ns = timespec_delta_nanoseconds(&t_now, &t_last);
        t_last = t_now;

        /* Write measured time difference to ringbuffer */
        WRITE_TO_RINGBUFFER(param->rbuffer, time_diff_ns);
    }

    pthread_exit(NULL);
}


/**
 * @brief Main. 
 */
int main(int argc, char** argv) {

    thread_args_t targs;
    parse_user_args(argc, argv, &targs);

    printf("Hello\n");
    
    /* initialize GPIO Port with default from config.h */
    if (targs.gpio == NULL) {
        targs.gpio = init_gpio(GPIO_PIN, GPIO_CHIP);
    }

    /* Initialize ringbuffer for storing time measurement results */
    size_t buffer_size = RING_BUFFER_SIZE * sizeof(uint64_t);
    char buffer[buffer_size];
    ring_buffer_t ring_buffer;
    ring_buffer_init(&ring_buffer, buffer, buffer_size);

    /* configure thread arguments */
    targs.rbuffer = &ring_buffer;
    targs.killswitch = 0;

    /* Create and start worker threads */
    pthread_t worker_signal_gen, worker_data_handler;
    int ret = pthread_create(&worker_signal_gen, NULL, &func_signal_gen, &targs);
    if (ret != 0) {
        fprintf(stderr, "Error spawning Worker-Thread\n");
        return EXIT_FAILURE;
    }

    ret = pthread_create(&worker_data_handler, NULL, &func_data_handler, &targs);
    if (ret != 0) {
        fprintf(stderr, "Error spawning Plot-Thread\n");
        return EXIT_FAILURE;
    }

    /* Wait for user input to stop the program */
    printf("Press Enter to stop...\n");
    getchar();
    targs.killswitch = 1;

    pthread_join(worker_signal_gen, NULL);
    pthread_join(worker_data_handler, NULL);

    /* Clean up */
    gpiod_line_request_release(targs.gpio->request);
    gpiod_chip_close(targs.gpio->chip);
    free(targs.gpio);

    return EXIT_SUCCESS;
}
