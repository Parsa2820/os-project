#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "lib/kernel/list.h"

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

static file_descriptor_t *
find_file_descriptor(int fd)
{
  struct list *file_descriptors = &thread_current()->file_descriptors;

  if (list_empty(file_descriptors))
  {
    return NULL;
  }

  struct list_elem *current_element;
  struct file_descriptor *current_fd;

  for (current_element = list_begin(file_descriptors);
       current_element != list_end(file_descriptors);
       current_element = list_next(current_element))
  {
    current_fd = list_entry(current_element, file_descriptor_t, elem);

    if (current_fd->fileno == fd)
    {
      return current_fd;
    }
  }

  return NULL;
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
      fd == 0 /* stdin */)
  {
    f->eax = -1;
    return;
  }

  if (fd == 1 /* stdout */)
  {
    putbuf(buffer, size);
  }

  file_descriptor_t *file_descriptor = find_file_descriptor(fd);

  if (file_descriptor == NULL)
  {
    f->eax = -1;
    return;
  }

  f->eax = file_write(file_descriptor->file, buffer, size);
}

static void
syscall_halt(struct intr_frame *f, uint32_t *args)
{
  shutdown_power_off();
}

static void
syscall_wait(struct intr_frame *f, uint32_t *args)
{
  f->eax = process_wait(args[1]);
}

static void
syscall_exec(struct intr_frame *f, uint32_t *args)
{
  f->eax = process_execute((char *)args[1]);
}

static void
syscall_open(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_close(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_remove(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_read(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_tell(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_seek(struct intr_frame *f, uint32_t *args)
{
}

static void
syscall_filesize(struct intr_frame *f, uint32_t *args)
{
}

syscall_descriptor_t syscall_table[] = {
    {SYS_EXIT, &syscall_exit},
    {SYS_PRACTICE, &syscall_practice},
    {SYS_WRITE, &syscall_write},
    {SYS_HALT, &syscall_halt},
    {SYS_WAIT, &syscall_wait},
    {SYS_EXEC, &syscall_exec},
    {SYS_OPEN, &syscall_open},
    {SYS_CLOSE, &syscall_close},
    {SYS_REMOVE, &syscall_remove},
    {SYS_READ, &syscall_read},
    {SYS_TELL, &syscall_tell},
    {SYS_SEEK, &syscall_seek},
    {SYS_FILESIZE, &syscall_filesize},
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
