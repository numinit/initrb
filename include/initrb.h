#ifndef INITRB_H
#define INITRB_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#include <ruby.h>
#include <readline.h>

typedef int (*initrb_boot_fn_t)(int, char **);
typedef VALUE (*initrb_ruby_fn_t)(VALUE);

typedef struct initrb_signal_handler {
    int signal;
    void (*handler)(int);
} initrb_signal_handler_t;

int initrb_start(int argc, char **argv, initrb_boot_fn_t boot);

#endif
