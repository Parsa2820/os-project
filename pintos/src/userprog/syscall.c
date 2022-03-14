#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static bool
is_valid_ptr(const void *ptr)
{
  return ptr != NULL && is_user_vaddr(ptr);
}

static void
syscall_exit(struct intr_frame *f, uint32_t *args)
{
  int status = args[1];
  f->eax = status;
  printf("%s: exit(%d)\n", &thread_current()->name, status);
  thread_exit();
}

static void
syscall_practice(struct intr_frame *f, uint32_t *args)
{
  int i = args[1];
  f->eax = i + 1;
}

static void
syscall_write(struct intr_frame *f, uint32_t *args)
{
  int fd = args[1];
  void *buffer = (void *)args[2];
  unsigned size = args[3];
  if (!is_valid_ptr(buffer) ||
      !is_valid_ptr(buffer + size - 1) ||
      // size <= 0 || // size validation may needed
      // size > PGSIZE ||
      fd == 0 /* stdin */)
  {
    f->eax = -1;
    return;
  }
  if (fd == 1 /* stdout */)
  {
    putbuf(buffer, size);
  }
  // TODO: get the file descriptor from the thread and write to it
}

syscall_descriptor_t syscall_table[] = {
    {SYS_EXIT, &syscall_exit},
    {SYS_PRACTICE, &syscall_practice},
    {SYS_WRITE, &syscall_write},
};

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  uint32_t *args = ((uint32_t *)f->esp);

  /*
   * The following print statement, if uncommented, will print out the syscall
   * number whenever a process enters a system call. You might find it useful
   * when debugging. It will cause tests to fail, however, so you should not
   * include it in your final submission.
   */

  /* printf("System call number: %d\n", args[0]); */

  int syscall_number = args[0];

  for (int i = 0; i < (sizeof(syscall_table) / sizeof(syscall_descriptor_t)); i++)
  {
    if (syscall_number == syscall_table[i].number)
    {
      syscall_table[i].func(f, args);
    }
  }

  NOT_REACHED();
}
