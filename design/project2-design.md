# سیستم‌های عامل - تمرین گروهی دوم

## مشخصات گروه

>> نام، نام خانوادگی و ایمیل خود را در ادامه وارد کنید.

پارسا محمدیان <parsa2820@gmail.com>

آرین یزدان‌پرست <arian.yazdan2001@gmail.com>

سارا آذرنوش <azarnooshsa@gmail.com> 

کسری امانی <kasra138079@gmail.com>

## مقدمه

>> اگر نکته‌ای درباره فایل‌های سابمیت شده یا برای TAها دارید، لطفا اینجا بیان کنید.

>> اگر از هر منبع برخط یا غیر برخطی به غیر از مستندات Pintos، متن درس، اسلایدهای درس یا نکات گفته شده در کلاس در تمرین گروهی استفاده کرده‌اید، لطفا اینجا آن(ها) را ذکر کنید.

## ساعت زنگ‌دار

### داده ساختارها

>> پرسش اول: تعریف `struct`های جدید، `struct`های تغییر داده شده، متغیرهای گلوبال یا استاتیک، `typedef`ها یا `enumeration`ها را در اینجا آورده و برای هریک در 25 کلمه یا کمتر توضیح بنویسید.

timer.c
```c
static struct list sleep_list; //sorted linked list of all sleeping threads by time to wake up
static struct lock sleep_lock; //lock on all sleeping threads
```

thread.c

    
    
```c
struct thread {
    ...
    int64_t wakeup_time; //time to wake up

};

int64_t wake_up_time() {} // return current_tick + ticks;

```

### الگوریتم

>> پرسش دوم: به اختصار آن‌چه هنگام صدا زدن تابع `timer_sleep()` رخ می‌دهد و همچنین اثر `timer interrupt handler` را توضیح دهید.
```
`timer_sleep()`
When a thread calls `timer_sleep()`, it should determine the time it should be woken up and save it in `wakeup_time` with `wake_up_time()`.
After that, it should get `sleep_lock` and use `list_insert_ordered()` to insert the current thread into the proper spot in `sleep_list`.
Finally, `thread_block()` should be called to puts the current thread to sleep.  
```
```
`timer interrupt handler`
It should determine if `sleep_lock` is currently in use.
If not, it should pop any threads that need to be unblocked from the `sleep_list` and unblock them. 
```
>> پرسش سوم: مراحلی که برای کوتاه کردن زمان صرف‌شده در `timer interrupt handler` صرف می‌شود را نام ببرید.
```
The `sleep_list` is sorted according to `wakeup_time` and always updated. Thus, each time the list control goes from the beginning to the first place where `wakeup_time` is no longer greater than the current count of timer ticks, the rest is not checked.
```
### همگام‌سازی

>> پرسش چهارم: هنگامی که چند ریسه به طور همزمان `timer_sleep()` را صدا می‌زنند، چگونه از `race condition` جلوگیری می‌شود؟
```
We introduced `sleep_lock` because if a thread is traversing the `sleep_list` and a timer interrupt happens, it may pop numerous elements off the list, leaving the thread with invalid pointers.
`Timer_interrupt()`, on the other hand, operates in an external interrupt context. Thus it will check whether or not the lock is held but won't need to acquire it. 
```
>> پرسش پنجم: هنگام صدا زدن `timer_sleep()` اگر یک وقفه ایجاد شود چگونه از `race condition` جلوگیری می‌شود؟
```
To avoid race conditions, we disable interrupts at the beginning of `timer_sleep()`.
```
### منطق

>> پرسش ششم: چرا این طراحی را استفاده کردید؟ برتری طراحی فعلی خود را بر طراحی‌های دیگری که مدنظر داشته‌اید بیان کنید.
```
we use a sorted linked list for `sleep_list` since it reduced the time spent in the handler with interrupts disabled from O(logn) to O(1).
```

## زمان‌بند اولویت‌دار

### داده ساختارها

>> پرسش اول: تعریف `struct`های جدید، `struct`های تغییر داده شده، متغیرهای گلوبال یا استاتیک، `typedef`ها یا `enumeration`ها را در اینجا آورده و برای هریک در ۲۵ کلمه یا کمتر توضیح بنویسید.

`base_priority`: 

برابر اولویت اولیه ترد است و در صورت اهدای اولویت، این متغیر برای بازگردانی اولیت ریسه مورد نیاز است.

`aquired_locks`: 

برابر قفل‌هایی است که ترد در دست دارد. در صورتی که اهدای تو در تو داشته باشیم، برای تغییر اولیت ترد به اطلاعات قفل نیاز داریم.

`priority`:

برابر اولیت قفل است که در واقع برابر بالاترین اولویت ریسه‌هایی است که منتظر قفل هستند یا قفل را گرفته‌اند.
```
struct thread {
    ...
    int base_priority;
    struct list aquired_locks;
    ...
}

struct lock {
    int priority;
}

```

>> پرسش دوم: داده‌ساختارهایی که برای اجرای `priority donation` استفاده شده‌است را توضیح دهید. (می‌توانید تصویر نیز قرار دهید)

از داده ساختار `lock`و `semaphore` و `thread` با توجه به تغییرات داده شده استفاده شده است. از آنجایی که ممکن است اولویت ریسه‌ها تغییر کنند، نیاز است اولویت اولیه ریسه `base_priority` ذخیره شود. از آنجایی که ممکن است اهدای تو در تو داشته باشیم، زمانی که یک ریسه از یک قفل بیرون‌ می‌آید نیاز است اولویت آن به اولویت قفلی که در آن قرار دارد تغییر کند به همین دلیل باید لیستی از قفل‌های گرفته شده داشته باشیم.
برای مقایسه اولویت ریسه‌ی دارای قفل و ریسه‌ی منتظر قفل متغیر `priority` اضافه شده است. در صورتی که اولویت ریسه‌ی دارنده‌ی قفل اولویت پایین‌تری داشته باشد اهدای اولویت صورت می‌گیرد. الگوریتم‌های اهدای اولویت به صورت دقیق‌تر در ادامه توضیح داده شده است.
### الگوریتم

>> پرسش سوم: چگونه مطمئن می‌شوید که ریسه با بیشترین اولویت که منتظر یک قفل، سمافور یا `condition variable` است زودتر از همه بیدار می‌شود؟

در بخشی از تابع `sema_up` داریم:

```
thread_unblock(list_entry(list_pop_front(&sema->waiters),
                              struct thread, elem));
```
در اینجا کافیست تابعی به نام `get_max_thread` داشته باشیم که ریسه‌ای با بالاترین اولویت را از لیست ریسه‌های منتظر پیدا کرده خروجی دهد.
این تابع به جای تابع `list_pop_front`‌ در قطعه کد بالا فراخوانی می‌گردد.


>> پرسش چهارم: مراحلی که هنگام صدازدن `lock_acquire()` منجر به `priority donation` می‌شوند را نام ببرید. دونیشن‌های تو در تو چگونه مدیریت می‌شوند؟

هنگامی که `lock_acquire‍` فراخوانی می‌شود، مقدار اولویت قفل با اولولیت ریسه‌ای که تابع را صدا می‌زند مقایسه می‌شود. در صورتی که ریسه اولویت بالاتری داشته باشد مقدار اولویت قفل و ریسه‌ای که قفل را دارد برابر اولویت ریسه‌ی صدا زننده می‌شود. در صورتی که اهدای تو در تو داشته باشیم، چون اولویت جدید ریسه با قفلی که می‌خواهد بگیرد مقایسه می‌شود، مشکلی پیش نمی‌آید. در این قسمت قفل گرفته شده به لیست `aquired_list`
ریسه اضافه می‌گردد.

>> پرسش پنجم: مراحلی که هنگام صدا زدن `lock_release()` روی یک قفل که یک ریسه با اولویت بالا منتظر آن است، رخ می‌دهد را نام ببرید.
 
همانطور که گفته شد در تابع `sema_up`  ریسه‌ای که اولویت بالاتری دارد انتخاب شده و انبلاک می‌شود. از آنجایی که `lock_release` این تابع را صدا می‌زند در نتیجه ریسه‌ی با اولویت بالا انبلاک می‌شود. دقت کنید بعد از آزاد سازی قفل توسط ریسه، در صورتی که لیست `aquierd_lock`
خالی باشد، اولویت ریسه برابر ‍`base_priority` خواهد شد. در غیر این صورت اولویت ریسه برابر اولویت اخرین عضو این لیست (اخرین قفلی که توسط ریسه گرفته شده است)   می‌شود.

### همگام‌سازی

>> پرسش ششم: یک شرایط احتمالی برای رخداد `race condition` در `thread_set_priority` را بیان کنید و توضیح دهید که چگونه پیاده‌سازی شما از رخداد آن جلوگیری می‌کند. آیا می‌توانید با استفاده از یک قفل از رخداد آن جلوگیری کنید؟

در صورتی که اولویت یک ریسه از طرف دو ریسه‌ی متفاوت، با اولویت‌های متفاوت بخواهد تغییر کند ممکن است `race_condition` رخ دهد. در صورتی که تغییر اولوت ریسه درون `sema_down` یا `sema_up` باشد مشکلی پیش نمی‌آید چون در این توابع اینتراپت‌ها غیر فعال شده و دیگر امکان `context_switch` وجود ندارد. در غیر این صورت می‌توانیم داخل تابع 
`thread_set_priority`
قفل قرار دهیم.
### منطق

>> پرسش هفتم: چرا این طراحی را استفاده کردید؟ برتری طراحی فعلی خود را بر طراحی‌های دیگری که مدنظر داشته‌اید بیان کنید.

راه اول:
برای قسمت اهدای تو در تو، در هر ریسه، شماره‌ی ریسه‌‌ای که به آن اولویت اهدا شده است نگه داری شود و زمانی که قفل گرفته می‌شود آن ریسه به اولویت پایه 
خود بازگردد. این روش از روش فعلی پیچیده‌تر است.

راه دوم:
می‌توان به جای اولویت ترد‌ها، اولویت قفل‌ها را مقایسه کرد که این راه نیز از روش فعلی پیچیده‌تر است.

## سوالات افزون بر طراحی

>> پرسش هشتم: در کلاس سه صفت مهم ریسه‌ها که سیستم عامل هنگامی که ریسه درحال اجرا نیست را ذخیره می‌کند، بررسی کردیم:‍‍ `program counter` ، ‍‍‍`stack pointer` و `registers`. بررسی کنید که این سه کجا و چگونه در `Pintos` ذخیره می‌شوند؟ مطالعه ‍`switch.S` و تابع ‍`schedule` در فایل `thread.c` می‌تواند مفید باشد.

Here is the `switch_threads` function in the `switch.S` file which is called by `schedule` function in the `thread.c` file:

```
```asm
.globl switch_threads
.func switch_threads
switch_threads:
	# Save caller's register state.
	#
	# Note that the SVR4 ABI allows us to destroy %eax, %ecx, %edx,
	# but requires us to preserve %ebx, %ebp, %esi, %edi.  See
	# [SysV-ABI-386] pages 3-11 and 3-12 for details.
	#
	# This stack frame must match the one set up by thread_create()
	# in size.
	pushl %ebx
	pushl %ebp
	pushl %esi
	pushl %edi

	# Get offsetof (struct thread, stack).
.globl thread_stack_ofs
	mov thread_stack_ofs, %edx

	# Save current stack pointer to old thread's stack, if any.
	movl SWITCH_CUR(%esp), %eax
	movl %esp, (%eax,%edx,1)

	# Restore stack pointer from new thread's stack.
	movl SWITCH_NEXT(%esp), %ecx
	movl (%ecx,%edx,1), %esp

	# Restore caller's register state.
	popl %edi
	popl %esi
	popl %ebp
	popl %ebx
        ret
.endfunc
```

As we can comprehend from the comments and code, first it pushes four registers (`%ebx`, `%ebp`, `%esi`, `%edi`) on the stack, then it pushes old thread's stack pointer on the stack of the old thread. Then it restores the new thread's stack pointer from the stack of the new thread. Finally, it restores four previously mentioned registers from the stack.

>> پرسش نهم: وقتی یک ریسه‌ی هسته در ‍`Pintos` تابع `thread_exit` را صدا می‌زند، کجا و به چه ترتیبی صفحه شامل پشته و `TCB` یا `struct thread` آزاد می‌شود؟ چرا این حافظه را نمی‌توانیم به کمک صدازدن تابع ‍`palloc_free_page` داخل تابع ‍`thread_exit` آزاد کنیم؟

Here is the `thread_exit` function.

```c
/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void thread_exit(void)
{
  ASSERT(!intr_context());

#ifdef USERPROG
  process_exit();
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  intr_disable();
  list_remove(&thread_current()->allelem);
  thread_current()->status = THREAD_DYING;
  schedule();
  NOT_REACHED();
}
```

As we can see this function removes the current thread from the list of all threads and sets its status to `THREAD_DYING`. Then it calls `schedule` function to schedule another thread. In the `schedule` function, `thread_schedule_tail` function is called.

```c
/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroys it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf() calls should be
   added at the end of the function.

   After this function and its caller return, the thread switch
   is complete. */
void thread_schedule_tail(struct thread *prev)
{
  struct thread *cur = running_thread();

  ASSERT(intr_get_level() == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread)
  {
    ASSERT(prev != cur);
    palloc_free_page(prev);
  }
}
```

At the end of this function previous thread's (a thread in which `thread_exit` is called) status is checked, if it is `THREAD_DYING` and it is not the initial thread, then it is freed with `palloc_free_page`. The reason for not calling `palloc_free_page` in `thread_exit` is that we need those information in order to schedule next thread.

>> پرسش دهم: زمانی که تابع ‍`thread_tick` توسط `timer interrupt handler` صدا زده می‌شود، در کدام پشته اجرا می‌شود؟

```c
/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void thread_tick(void)
{
  struct thread *t = thread_current();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return();
}
```

As the comments suggest, this function is called by the timer interrupt handler and it runs in an external interrupt context. So, it will run in kernel mode and it will use the kernel stack.

>> پرسش یازدهم: یک پیاده‌سازی کاملا کاربردی و درست این پروژه را در نظر بگیرید که فقط یک مشکل درون تابع ‍`sema_up()` دارد. با توجه به نیازمندی‌های پروژه سمافورها(و سایر متغیرهای به‌هنگام‌سازی) باید ریسه‌های با اولویت بالاتر را بر ریسه‌های با اولویت پایین‌تر ترجیح دهند. با این حال پیاده‌سازی ریسه‌های با اولویت بالاتر را براساس اولویت مبنا `Base Priority` به جای اولویت موثر ‍`Effective Priority` انتخاب می‌کند. اساسا اهدای اولویت زمانی که سمافور تصمیم می‌گیرد که کدام ریسه رفع مسدودیت شود، تاثیر داده نمی‌شود. تستی طراحی کنید که وجود این باگ را اثبات کند. تست‌های `Pintos` شامل کد معمولی در سطح هسته (مانند متغیرها، فراخوانی توابع، جملات شرطی و ...) هستند و می‌توانند متن چاپ کنند و می‌توانیم متن چاپ شده را با خروجی مورد انتظار مقایسه کنیم و اگر متفاوت بودند، وجود مشکل در پیاده‌سازی اثبات می‌شود. شما باید توضیحی درباره این که تست چگونه کار می‌کند، خروجی مورد انتظار و خروجی واقعی آن فراهم کنید.

The following pseudocode exploits this bug. In this test, first `thread_a` starts executing and it occupies `another_sema`. Then `thread_b` starts executing and occupies `sema` and waits on `another_sema`. Then `thread_c` starts executing and waits on `sema`. Then `thread_d` starts executing and waits for `sema`. When `thread_a` finishes, `thread_b` and `thread_c` are both waiting for `another_sema`. Although `thread_c`'s base priority is higher than `thread_b`, `thread_b` should be scheduled first because `thread_d` which has a higher base priority compared to `thread_c` is waiting on `sema` which is occupied by `thread_b`. 

```python
sema = sema(1)
anoter_sema = sema(1)

def func(x):
    sema_down(sema)
    print(x)
    sema_up(sema)

def another_func(x):
    sema_down(another_sema)
    print(x)
    sema_up(another_sema)

def yet_another_func(x):
    sema_down(sema)
    sema_down(another_sema)
    print(x)
    sema_up(another_sema)
    sema_up(sema)

thread_a = thread(target=another_func, args=("thread_a",), priority=1)
thread_b = thread(target=yet_another_func, args=("thread_b",), priority=2)
thread_c = thread(target=another_func, args=("thread_c",), priority=3)
thread_d = thread(target=func, args=("thread_d",), priority=4)
thread_a.start()
thread_b.start()
thread_c.start()
thread_d.start()
```

Expected output:

```
thread_a
thread_b
thread_c
thread_d
```

The last two lines of the above output can be swapped and it does not make a difference. The important bug is that `thread_b` executes before `thread_c`.

Real buggy output:

```
thread_a
thread_c
thread_b
thread_d
```

## سوالات نظرسنجی

پاسخ به این سوالات دلخواه است، اما به ما برای بهبود این درس در ادامه کمک خواهد کرد. نظرات خود را آزادانه به ما بگوئید—این سوالات فقط برای سنجش افکار شماست. ممکن است شما بخواهید ارزیابی خود از درس را به صورت ناشناس و در انتهای ترم بیان کنید.

>> به نظر شما، این تمرین گروهی، یا هر کدام از سه وظیفه آن، از نظر دشواری در چه سطحی بود؟ خیلی سخت یا خیلی آسان؟

>> چه مدت زمانی را صرف انجام این تمرین کردید؟ نسبتا زیاد یا خیلی کم؟

>> آیا بعد از کار بر روی یک بخش خاص از این تمرین (هر بخشی)، این احساس در شما به وجود آمد که اکنون یک دید بهتر نسبت به برخی جنبه‌های سیستم عامل دارید؟

>> آیا نکته یا راهنمایی خاصی وجود دارد که بهتر است ما آنها را به توضیحات این تمرین اضافه کنیم تا به دانشجویان ترم های آتی در حل مسائل کمک کند؟

>> متقابلا، آیا راهنمایی نادرستی که منجر به گمراهی شما شود وجود داشته است؟

>> آیا پیشنهادی در مورد دستیاران آموزشی درس، برای همکاری موثرتر با دانشجویان دارید؟

این پیشنهادات میتوانند هم برای تمرین‌های گروهی بعدی همین ترم و هم برای ترم‌های آینده باشد.

>> آیا حرف دیگری دارید؟