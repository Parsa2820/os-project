#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init(void);

typedef struct syscall_descriptor
{
    int number;
    void (*func)(struct intr_frame*, uint32_t*);
} syscall_descriptor_t;

#endif /* userprog/syscall.h */
