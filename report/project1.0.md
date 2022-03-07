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
```
0xc0000008
```

۲.
```
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
In question 3 instruction, we are trying to access parameters which passed to us by `_start` function caller (actually pushed to stack). Probably caller does not pass parameters properly. So when we try to access them, we get error. 

## به سوی crash

۶.
```
  Id   Target Id         Frame
* 1    Thread <main>     process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
Address: 0xc000e000
```
```
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000edec <incomple
te sequence \357>, priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0
xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", prior
ity = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8
>}, pagedir = 0x0, magic = 3446325067}
```

۷.
```
#0  process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
#1  0xc0020268 in run_task (argv=0xc00357cc <argv+12>) at ../../threads/init.c:288
#2  0xc0020921 in run_actions (argv=0xc00357cc <argv+12>) at ../../threads/init.c:340
#3  main () at ../../threads/init.c:133
```
```
sema_init (&temporary, 0);
process_wait (process_execute (task));
a->function (argv);
run_actions (argv);
```

۸.
```
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_BLOCKED, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0037314 <temporary+4>, next = 0xc003731c <temporary+12>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc010a020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #2: 0xc010a000 {tid = 3, status = THREAD_RUNNING, name = "do-nothing\000\000\000\000\000", stack = 0xc010afd4 "", priority = 31, allelem = {prev = 0xc0104020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۹.
```
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

In order to run the user's code, an interrupt must be given to the operating system, and therefore this interrupt is thrown in this function so that the user's code can be executed. At the beginning of the function, the registers are given correct values, and after that, the user's code is loaded in the created frame. In the end, the simulation of an interrupt is completed via calling the "intr_exit" function. The function ends with the execution of the user's code, and this means we return to the "userspace."
<!-- برای اجرای کد کاربر نیاز است که وقفه‌ای به سیستم‌ عامل داده شود به همین دلیل  در این تابع وقفه‌ی مورد نظر شبیه‌سازی شده است تا اجرای کد کاربر صورت بگیرد. در ابتدای تابع رجیستر‌ها به صورت صحیح مقدار دهی شده‌اند و در ادامه، کد کاربر در فریم ساخته شده لود شده است. در انتها با فراخوانی تابع 
intr_exit
با فریم ساخته شده فراخوانی شده که شبیه‌سازی وقفه را تکمیل می‌کند. در انتهای این تابع اجرای کد کاربر صورت می‌گیرد که به معنای انتقال به 
user space
است.،  -->

<!-- #TODO
در زمان اجرای کد کاربر این وقفه اتفاق افتاده که برای برگشت از ادامه‌ی اجرای آن کد برگردد -->

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
As expected, these values are equal to values in question 10.

۱۳.
```
#0  _start (argc=<unavailable>, argv=<unavailable>) at ../../lib/user/entry.c:9
```

## دیباگ

۱۴.
Surely we have problem with the stack. So we have to search for the problem before in the `start_process` function. 

۱۵.

> The point of this is that there are some "SIMD" (Single Instruction, Multiple Data) instructions (also known in x86-land as "SSE" for "Streaming SIMD Extensions") which can perform parallel operations on multiple words in memory, but require those multiple words to be a block starting at an address which is a multiple of 16 bytes.
[reference](https://stackoverflow.com/questions/4175281/what-does-it-mean-to-align-the-stack)

```perl
do-stack-align: exit(12)
```
This line indicates that the return value of the function must be 12. Which means `esp % 16` must be equal to 12.




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
As we can see they are equal with those values which were on top of the stack in previous question.

۱۸.
```
int process_wait(tid_t child_tid UNUSED)
{
  sema_down(&temporary);
  return 0;
}
```

۱۹.
```
  Id   Target Id         Frame
* 1    Thread <main>     sema_down (sema=sema@entry=0xc00372fc <console_lock+4>) at ../../threads/synch.c:62
Address:
0xc000e000
All threads:
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```
