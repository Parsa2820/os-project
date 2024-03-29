تمرین گروهی ۱/۰ - آشنایی با pintos
======================

شماره گروه:
-----
> نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

پارسا محمدیان <parsa2820@gmail.com>

آرین یزدان‌پرست <arian.yazdan2001@gmail.com>

سارا آذرنوش <azarnooshsa@gmail.com> 

کسری امانی <kasra138079@gmail.com>

مقدمات
----------
> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت بنویسید.


> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع  درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

آشنایی با pintos
============
>  در مستند تمرین گروهی ۱۹ سوال مطرح شده است. پاسخ آن ها را در زیر بنویسید.


## یافتن دستور معیوب

۱.
```c
0xc0000008
```

۲.
```c
0x8048757
```

۳. 
```
Command: objdump -d do-nothing
Function: _start
Instruction: mov    0x24(%esp),%eax
```

۴.
``` 
Command: grep -rnw 'pintos/src/lib/' -e '_start'
Output: 
pintos/src/lib/user/user.lds:3:ENTRY(_start)
pintos/src/lib/user/entry.c:4:void _start (int argc, char *argv[]);
pintos/src/lib/user/entry.c:7:_start (int argc, char *argv[])
```

First result (`user.lds`) is linker script file which does not contain `_start` function definition. So the definition is in `entry.c` file. 

```c
void
_start (int argc, char *argv[])
{
  exit (main (argc, argv));
}
```

**`objdump` assembly notation is slightly different from x86 assembly. For example in `mov` instruction, first operand is source and the second one is destination.**

```
08048754 <_start>:
 Instruction: 8048754:	83 ec 1c             	sub    $0x1c,%esp
 Explanation: %esp is the stack pointer. It is subtracted by 28(0x1c) which means we have expanded the stack by 28 bytes.
 Instruction: 8048757:	8b 44 24 24          	mov    0x24(%esp),%eax
 Explanation: Copy argv (which is pushed in stack by _start caller) from stack to %eax register.
 Instruction: 804875b:	89 44 24 04          	mov    %eax,0x4(%esp)
 Explanation: Copy %eax (which now contains argv) to stack in order to prepare main function parameter.
 Instruction: 804875f:	8b 44 24 20          	mov    0x20(%esp),%eax
 Explanation: Copy argc (which is pushed in stack by _start caller) from stack to %eax register.
 Instruction: 8048763:	89 04 24             	mov    %eax,(%esp)
 Explanation: Copy %eax (which now contains argc) to stack in order to prepare main function parameter.
 Instruction: 8048766:	e8 35 f9 ff ff       	call   80480a0 <main>
 Explanation: This is the call to the main function. First it pushes the return address (4 byte) to the stack and then it jumps to the main function address.
 Instruction: 804876b:	89 04 24             	mov    %eax,(%esp)
 Explanation: Return value of the main function is stored in %eax. So we are pushing it into the stack as exit function argument.
 Instruction: 804876e:	e8 49 1b 00 00       	call   804a2bc <exit>
 Explanation: This is the call to the exit function. First it pushes the return address (4 byte) to the stack and then it jumps to the main function address.
```

۵.

In question 3 instruction, we are trying to access parameters which passed to us by `_start` function caller (actually pushed to stack). Probably caller does not pass parameters properly, or the caller does not alter the stack pointer appropriately. So when we try to access them, we get an error.

## به سوی crash

۶.
```
  Id   Target Id         Frame
* 1    Thread <main>     process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
```
```
Name: main
Address: 0xc000e000
```
```
(gdb) dumplist &all_list thread allelem
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000edec <incomplete sequence \357>, priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```
۷.
```
#0  process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
#1  0xc0020268 in run_task (argv=0xc00357cc <argv+12>) at ../../threads/init.c:288
#2  0xc0020921 in run_actions (argv=0xc00357cc <argv+12>) at ../../threads/init.c:340
#3  main () at ../../threads/init.c:133
```
```c
int
main (void)
{
  char **argv;

  /* Clear BSS. */
  bss_init ();

  /* Break command line into arguments and parse options. */
  argv = read_command_line ();
  argv = parse_options (argv);

  /* Initialize ourselves as a thread so we can use locks,
     then enable console locking. */
  thread_init ();
  console_init ();

  /* Greet user. */
  printf ("Pintos booting with %'"PRIu32" kB RAM...\n",
          init_ram_pages * PGSIZE / 1024);

  /* Initialize memory system. */
  palloc_init (user_page_limit);
  malloc_init ();
  paging_init ();

  /* Segmentation. */
#ifdef USERPROG
  tss_init ();
  gdt_init ();
#endif

  /* Initialize interrupt handlers. */
  intr_init ();
  timer_init ();
  kbd_init ();
  input_init ();
#ifdef USERPROG
  exception_init ();
  syscall_init ();
#endif

  /* Start thread scheduler and enable interrupts. */
  thread_start ();
  serial_init_queue ();
  timer_calibrate ();

#ifdef FILESYS
  /* Initialize file system. */
  ide_init ();
  locate_block_devices ();
  filesys_init (format_filesys);
#endif

  printf ("Boot complete.\n");

  /* Run actions specified on kernel command line. */
  run_actions (argv); /* This line is the function call */

  /* Finish up. */
  shutdown ();
  thread_exit ();
}
```
```c
static void
run_actions (char **argv)
{
  /* An action. */
  struct action
    {
      char *name;                       /* Action name. */
      int argc;                         /* # of args, including action name. */
      void (*function) (char **argv);   /* Function to execute action. */
    };

  /* Table of supported actions. */
  static const struct action actions[] =
    {
      {"run", 2, run_task},
#ifdef FILESYS
      {"ls", 1, fsutil_ls},
      {"cat", 2, fsutil_cat},
      {"rm", 2, fsutil_rm},
      {"extract", 1, fsutil_extract},
      {"append", 2, fsutil_append},
#endif
      {NULL, 0, NULL},
    };

  while (*argv != NULL)
    {
      const struct action *a;
      int i;

      /* Find action name. */
      for (a = actions; ; a++)
        if (a->name == NULL)
          PANIC ("unknown action `%s' (use -h for help)", *argv);
        else if (!strcmp (*argv, a->name))
          break;

      /* Check for required arguments. */
      for (i = 1; i < a->argc; i++)
        if (argv[i] == NULL)
          PANIC ("action `%s' requires %d argument(s)", *argv, a->argc - 1);

      /* Invoke action and advance. */
      a->function (argv); /* This line is the function call */
      argv += a->argc;
    }

}
```
```c
static void
run_task (char **argv)
{
  const char *task = argv[1];

  printf ("Executing '%s':\n", task);
#ifdef USERPROG
  process_wait (process_execute (task)); /* This line is the function call */
#else
  run_test (task);
#endif
  printf ("Execution of '%s' complete.\n", task);
}
```

۸.

Threads `idle` and `do-nothing` are exist in the OS too.
```
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_BLOCKED, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0037314 <temporary+4>, next = 0xc003731c <temporary+12>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc010a020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #2: 0xc010a000 {tid = 3, status = THREAD_RUNNING, name = "do-nothing\000\000\000\000\000", stack = 0xc010afd4 "", priority = 31, allelem = {prev = 0xc0104020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۹.
```c
tid_t
process_execute (const char *file_name)
{
  char *fn_copy;
  tid_t tid;

  sema_init (&temporary, 0);
  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR)
    palloc_free_page (fn_copy);
  return tid;
}
```

۱۰.
```
{edi = 0, esi = 0, ebp = 0, esp_dummy = 0, ebx = 0, edx = 0, ecx = 0, eax = 0, gs = 35, fs = 35, es = 35, ds = 35, vec_no = 0, error_code = 0, frame_pointer = 0x0, eip = 0x8048754, cs = 27, eflags = 514, esp = 0xc0000000, ss = 35}
```

۱۱.

In order to run the user's code (context switch), an interrupt must be given to the operating system, and therefore this interrupt is simulated in this function so that the user's code can be executed. At the beginning of the function, the registers are given specific values, and after that, the user's code is loaded in the created frame. In the end, the simulation of an interrupt is completed via calling the `intr_exit` function. The function ends with the execution of the user's code, and this means we return to the "user space". After the user's code execution, control is returned to where the interrupt was simulated.

۱۲.
```
info registers
eax            0x0      0
ecx            0x0      0
edx            0x0      0
ebx            0x0      0
esp            0xc0000000       0xc0000000
ebp            0x0      0x0
esi            0x0      0
edi            0x0      0
eip            0x8048754        0x8048754
eflags         0x202    [ IF ]
cs             0x1b     27
ss             0x23     35
ds             0x23     35
es             0x23     35
fs             0x23     35
gs             0x23     35
```
As expected, these values are equal to the ones of question 10.

۱۳.
```
#0  _start (argc=<unavailable>, argv=<unavailable>) at ../../lib/user/entry.c:9
```

## دیباگ

۱۴.
It is clear that there is an issue with the stack; Therefore we must seek it in the `start_process` function. 
```c
#define INIT_STACK_SIZE 0x24
// ...
*esp = PHYS_BASE - INIT_STACK_SIZE;
```
An error was given because the user code wanted to access a block of memory that was not allowed. Here, by adding the space required to stack (since stack pointer in downward, 0x24 is subtracted form esp), the user code will be allowed to access that block which contains the parameters of the function.

۱۵.
```perl
do-stack-align: exit(12)
```
This line indicates that the return value of the function must be 12. Which means `esp % 16` must be equal to 12. Since `esp` initial value is `PHYS_BASE - INIT_STACK_SIZE`, we can see that the equation is satisfied.
```c
#define LOADER_PHYS_BASE 0xc0000000     /* 3 GB. */
#define	PHYS_BASE ((void *) LOADER_PHYS_BASE)
#define INIT_STACK_SIZE 0x24
*esp = PHYS_BASE - INIT_STACK_SIZE;
```
```
*esp = 0xc0000000 - 0x24
*esp = 0xBFFFFFDC
*esp = 3221225436
*esp % 16 = 3221225436 % 16 = 12
```

۱۶.
```
0xbfffff98:     0x00000001      0x000000a2
```

۱۷.
```
(gdb) print args[0]
$1 = 1
(gdb) print args[1]
$2 = 162
```
As we can see their values are identical to those which were on the top of the stack in the previous question.

۱۸.
```c
int
process_wait (tid_t child_tid UNUSED)
{
  sema_down (&temporary);
  return 0;
}
```
As we can see the corresponding `sema_down` is in the `process_wait` function. If we search for the initial value of the `temporary` semaphore, we can see that it is equal to 0 (`sema_init` function is called with `value=0` in the `process_execute` function). This imply that only one user program can be executed at a time. So `process_wait` wait for the `temporary` semaphore to be released or in other words, it waits for the user program to finish.

۱۹.
```
  Id   Target Id         Frame
* 1    Thread <main>     sema_down (sema=sema@entry=0xc00372fc <console_lock+4>) at ../../threads/synch.c:62
```
```
Name: main
Address: 0xc000e000
```
```
All threads:
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```
