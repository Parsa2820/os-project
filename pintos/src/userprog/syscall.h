#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdint.h>
#include "threads/interrupt.h"

void syscall_init(void);

void exit_error();

void reset_cache();
int get_cache_hit();
int get_cache_miss();
typedef void syscall_func_t(struct intr_frame *, uint32_t *);

typedef struct syscall_descriptor
{
    int number;
    syscall_func_t *syscall_func;
    int is_file_op;
} syscall_descriptor_t;

#endif /* userprog/syscall.h */
