# تمرین گروهی ۱.۱ - مستند طراحی

گروه
-----
 > نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

پارسا محمدیان <parsa2820@gmail.com>

آرین یزدان‌پرست <arian.yazdan2001@gmail.com>

سارا آذرنوش <azarnooshsa@gmail.com> 

کسری امانی <kasra138079@gmail.com>

مقدمات
----------
> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت  بنویسید.

> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

پاس‌دادن آرگومان
============
داده‌ساختار‌ها
----------------
> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ایجاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

```c
struct process_param
{
    int argc;
    char **argv;
};
```
برای هر یک از پردازه‌ها نیاز به پارس کردن و جداسازی ورودی‌ها وجود دارد. به همین دلیل، برای راحتی بیشتر در استفاده از توابع، تعریف استراکت بالا برای پیاده سازی این موضوع کاربردی است.

الگوریتم‌ها
------------
> به‌طور خلاصه توضیح دهید چگونه آرگومان‌ها را پردازش کرده‌اید؟ چگونه اعضای `argv[]` را به ترتیب درست در پشته قرار داده‌اید؟ و چگونه از سرریز پشته جلوگیری کرده‌اید؟

تابع ‍`strtoke_r` یک `string` به عنوان ورودی گرفته و به وسیله‌ی `whitespace` آن را جدا می‌کند. 
با استفاده از این تابع می‌توان ورودی را پارس کرده و در استراکت تعریف شده ذخیره کرد. با توجه به تعداد ارگومان‌های پارس شده که در `argc` ذخیره شده است، به مقدار کافی ‍`esp` پایین آورده می‌شود، سپس با استفاده دستور  `memcpy` در آدرس مناسب ارگومان‌های پردازه در استک نوشته می‌شود. برای سرریز از پشته می‌توان ماکسیممی برای تعداد ارگومان‌های ورودی در نظر گرفت و در صورتی که تعداد ارگومان‌ها از آن بیشتر شد دیگر داخل پشته نوشته نشوند.


منطق طراحی
-----------------
> چرا Pintos به‌جای تابع‌ `strtok()` تابع‌ `strtok_r()` را پیاده‌سازی کرده‌است؟

بدلیل اینکه `strtok()`
از آدرسی `static` برای پاس دادن توکن‌های جدا سازی استفاده استفاده می‌کند، ترد‌های متفاوت نمی‌توانند همزمان این تابع را فراخوانی کنند اما در فراخوانی  `strtok_r()` این مشکل وجود ندارد.

> در Pintos عمل جدا کردن نام فایل از آرگومان‌ها، در داخل کرنل انجام می‌شود. در سیستم عامل‌های برپایه‌ی Unix، این عمل توسط shell انجام می‌شود. حداقل دو مورد از برتری‌های رویکرد Unix را توضیح دهید.

۱. پیچیدگی کرنل را کم می‌کند. دیگر بررسی درست بودن مسیر داده شده، پارس کردن ارگومان‌ها و ... به عهده‌ی کرنل نیست که این باعث سریع‌تر شدن کرنل و کم شدن پیچیدگی آن می‌شود.

۲. دیگر محدودیتی برای دستورات داده شده وجود ندارد. در واقع می‌توان از مفسر‌های متفاوتی برای شل‌های متفاوت استفاده کرد و دیگر نیازی به تغییر خود کرنل نیست.



فراخوانی‌های سیستمی
================
داده‌ساختار‌ها
----------------
> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ای.جاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

```c
// pintos/src/lib/kernel/list.h
struct list_elem
  {
    struct list_elem *prev;     /* Previous list element. */
    struct list_elem *next;     /* Next list element. */
    void *value; // new
  };
```
```c
// pintos/src/threads/thread.h
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    // parent of the current thread
    struct thread *parent; // new
    // child of the current thread stored as a doubly linked list
    struct list children; // new
    // last used fileno
    int last_fileno; // new
    // file descriptor of the current thread stored as a doubly linked list
    struct list files; // new
    // wait semaphore of the current thread.
    // if another thread is waiting on this semaphore, it will be blocked
    // this semaphore will be signaled when the current thread is finished
    struct semaphore wait; // new
    // lock for thread status
    struct lock status_lock; // new
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };
```
```c
// pintos/src/threads/thread.c
enum file_mode
  {
    FILE_READ,
    FILE_WRITE,
    FILE_APPEND
  };
struct file_descriptor // new
  {
    // low level file struct
    struct file *file; 
    // file descriptor number
    int fileno;
    // mode of the file
    enum file_mode mode;
  };

typedef struct file_descriptor file_descriptor_t;
```
```c
// pintos/src/filesys/filesys.h
// global lock for using the file system
struct lock filesys_lock;
```

> توضیح دهید که توصیف‌کننده‌های فایل چگونه به فایل‌های باز مربوط می‌شوند. آیا این توصیف‌کننده‌ها در کل سیستم‌عامل به‌طور یکتا مشخص می‌شوند یا فقط برای هر پردازه یکتا هستند؟

We have defined `file_descriptor_t` which contains a lower level `struct file`. A process only inform about the `file_descriptor_t` of a file it has opened. Considering we have no shared memory between processes, `file_descriptor_t.fileno` is unique for each process but not necessarily unique in the OS.

الگوریتم‌ها
------------
> توضیح دهید خواندن و نوشتن داده‌های کاربر از داخل هسته، در کد شما چگونه انجام شده است.

First we check the pointers. If they are NULL or invalid, we return -1. Then we check if the requested operation complies with the file mode. If it does not, we return -1. The we try to acuiqre file system the lock and do the operation. 

> فرض کنید یک فراخوانی سیستمی باعث شود یک صفحه‌ی کامل (۴۰۹۶ بایت) از فضای کاربر در فضای هسته کپی شود. بیشترین و کمترین تعداد بررسی‌‌های جدول صفحات (page table) چقدر است؟ (تعداد دفعاتی که `pagedir_get_page()` صدا زده می‌شود.) در‌ یک فراخوانی سیستمی که فقط ۲ بایت کپی می‌شود چطور؟ آیا این عددها می‌توانند بهبود یابند؟ چقدر؟

For both cases, if data are in a single page, we call `pagedir_get_page()` single time. In the worst case scenario, we should call it number of bytes times. In this case every two bytes are in different pages.

> پیاده‌سازی فراخوانی سیستمی `wait` را توضیح دهید و بگویید چگونه با پایان یافتن پردازه در ارتباط است.

As we can see in the `struct thread` we stored the `wait` semaphore. This semaphore initial value is 0. When a thread execution is finished, it will up the semaphore. On the other hand, when a thread wants to wait for another thread to finish, first it checks thread status and if it is not already finished, it downs the semaphore. Hence it will be signaled when the current thread is finished.

> هر دستیابی هسته به حافظه‌ی برنامه‌ی کاربر، که آدرس آن را کاربر مشخص کرده است، ممکن است به دلیل مقدار نامعتبر اشاره‌گر منجر به شکست شود. در این صورت باید پردازه‌ی کاربر خاتمه داده شود. فراخوانی های سیستمی پر از چنین دستیابی‌هایی هستند. برای مثال فراخوانی سیستمی `write‍` نیاز دارد ابتدا شماره‌ی فراخوانی سیستمی را از پشته‌ی کاربر بخواند، سپس باید سه آرگومان ورودی و بعد از آن مقدار دلخواهی از حافظه کاربر را (که آرگومان ها به آن اشاره می کنند) بخواند. هر یک از این دسترسی ها به حافظه ممکن است با شکست مواجه شود. بدین ترتیب با یک مسئله‌ی طراحی و رسیدگی به خطا (error handling) مواجهیم. بهترین روشی که به ذهن شما می‌رسد تا از گم‌شدن مفهوم اصلی کد در بین شروط رسیدگی به خطا جلوگیری کند چیست؟ همچنین چگونه بعد از تشخیص خطا، از آزاد شدن تمامی منابع موقتی‌ای که تخصیص داده‌اید (قفل‌ها، بافر‌ها و...) مطمئن می‌شوید؟ در تعداد کمی پاراگراف، استراتژی خود را برای مدیریت این مسائل با ذکر مثال بیان کنید.

For error handling we can use a global to store error message each time an error occurs. Beside that we can use a pointer to function to release resources. This is very similar to higher level programming languages exception handling mechanism. The release function may take some parameters to release (e.g. file descriptor, semaphore, mutex, etc.). So the prototype of the release function is `void (*release)(int argc, char** argv)`. Since after an error occurred, we do not have access to parameters, we should store parameters in a global variable as well. This is a reasonable solution, however, we may consider different approache in implentation because it is complicated. 

همگام‌سازی
---------------
> فراخوانی سیستمی `exec` نباید قبل از پایان بارگذاری فایل اجرایی برگردد، چون در صورتی که بارگذاری فایل اجرایی با خطا مواجه شود باید `-۱` برگرداند. کد شما چگونه از این موضوع اطمینان حاصل می‌کند؟ چگونه وضعیت موفقیت یا شکست در اجرا به ریسه‌ای که `exec` را فراخوانی کرده اطلاع داده می‌شود؟

This question also answerd in [project1.0.md](../report/project1.0.md) question 18. I copy the answer from there.

```c
int
process_wait (tid_t child_tid UNUSED)
{
  sema_down (&temporary);
  return 0;
}
```
As we can see the corresponding `sema_down` is in the `process_wait` function. If we search for the initial value of the `temporary` semaphore, we can see that it is equal to 0 (`sema_init` function is called with `value=0` in the `process_execute` function). This imply that only one user program can be executed at a time. So `process_wait` wait for the `temporary` semaphore to be released or in other words, it waits for the user program to finish.

> پردازه‌ی والد P و پردازه‌ی فرزند C را درنظر بگیرید. هنگامی که P فراخوانی `wait(C)` را اجرا می‌کند و C  هنوز خارج نشده است، توضیح دهید که چگونه همگام‌سازی مناسب را برای جلوگیری از ایجاد شرایط مسابقه (race condition) پیاده‌سازی کرده‌اید. وقتی که C از قبل خارج شده باشد چطور؟ در هر حالت چگونه از آزاد شدن تمامی منابع اطمینان حاصل می‌کنید؟ اگر P بدون منتظر ماندن، قبل از C خارج شود چطور؟ اگر بدون منتظر ماندن بعد از C خارج شود چطور؟ آیا حالت‌های خاصی وجود دارد؟

For preventing such a race conditions, We consider a lock for process status. Whenever a process wants to wait for another, first it should acquire status lock and then read the status. If the status is not finished, then it wait for the thread using the wait semaphore and release the status lock afterwards. If the status is finished, then it release the status lock and continue. Obviously, thread should acquire the status lock for changing the status. 

منطق طراحی
-----------------
> به چه دلیل روش دسترسی به حافظه سطح کاربر از داخل هسته را این‌گونه پیاده‌سازی کرده‌اید؟

Not implemented yet.

> طراحی شما برای توصیف‌کننده‌های فایل چه نقاط قوت و ضعفی دارد؟

Since we used new file descriptor each time, even if we have already opened the file, we are not using memory efficiently. But this approach is much more simpler and much more easier to use than reusing file descriptors. 

> در حالت پیش‌فرض نگاشت `tid` به `pid` یک نگاشت همانی است. اگر این را تغییر داده‌اید، روی‌کرد شما چه نقاط قوتی دارد؟

Not changed.

سوالات افزون بر طراحی
===========
> تستی را که هنگام اجرای فراخوانی سیستمی از یک اشاره‌گر پشته‌ی(esp) نامعتبر استفاده کرده است بیابید. پاسخ شما باید دقیق بوده و نام تست و چگونگی کارکرد آن را شامل شود.

`sc-bad-sp.c` as the name suggests, is the test which uses an invalid value for `esp`. In line 18 in `asm` statement, `esp` is set to `$.-(64*1024*1024)`([program counter](https://stackoverflow.com/questions/43012485/what-does-dot-mean-in-x86-s-assembly-files) minus 2^26) which is an invalid address. Then in same line, `int $0x30` is called. This system call must return error.

> تستی را که هنگام اجرای فراخوانی سیستمی از یک اشاره‌گر پشته‌ی معتبر استفاده کرده ولی اشاره‌گر پشته آنقدر به مرز صفحه نزدیک است که برخی از آرگومان‌های فراخوانی سیستمی در جای نامعتبر مموری قرار گرفته اند مشخص کنید. پاسخ شما باید دقیق بوده و نام تست و چگونگی کارکرد آن را شامل شود.یک قسمت از خواسته‌های تمرین را که توسط مجموعه تست موجود تست نشده‌است، نام ببرید. سپس مشخص کنید تستی که این خواسته را پوشش بدهد چگونه باید باشد.

`sc-bad-arg.c` is the test which uses a valid address for `esp` but because of ajacency of the address to kernel space, the system call will fail. In line 14 in `asm` statement, `esp` is set to `$0xbffffffc`. Considering the `PHYS_BASE` is `0xc0000000`, the address is valid but its difference from the `PHYS_BASE` is only 4. So if system call try to access the address which is 4 bytes away, it will fail.

سوالات نظرخواهی
==============
پاسخ به این سوالات اختیاری است، ولی پاسخ به آن‌ها می‌تواند به ما در بهبود درس در ترم‌های آینده کمک کند. هر چه در ذهن خود دارید بگویید. این سوالات برای دریافت افکار شما هستند. هم‌چنین می‌توانید پاسخ خود را به صورت ناشناس در انتهای ترم ارائه دهید.

> به نظر شما، این تمرین یا هر یک از سه بخش آن، آسان یا سخت بودند؟ آیا وقت خیلی کم یا وقت خیلی زیادی گرفتند؟

> آیا شما بخشی را در تمرین یافتید که دید عمیق‌تری نسبت به طراحی سیستم عامل به شما بدهد؟

> آیا مسئله یا راهنمایی خاصی وجود دارد که بخواهید برای حل مسائل تمرین به دانشجویان ترم‌های آینده بگویید؟

> آیا توصیه‌ای برای دستیاران آموزشی دارید که چگونه دانشجویان را در ترم‌های آینده یا در ادامه‌ی ترم بهتر یاری کنند؟

> اگر نظر یا بازخورد دیگری دارید در این قسمت بنویسید.
