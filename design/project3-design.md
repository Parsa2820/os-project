تمرین گروهی ۳ - مستند طراحی
======================

گروه
-----

>>نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.


پارسا محمدیان <parsa2820@gmail.com>

آرین یزدان‌پرست <arian.yazdan2001@gmail.com>

سارا آذرنوش <azarnooshsa@gmail.com> 

کسری امانی <kasra138079@gmail.com>

مقدمات
----------

>> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت  بنویسید.



>> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

بافر کش
============

داده‌ساختار‌ها و توابع
---------------------

>> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ایجاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

```
struct cache_block{
    struct list_elem elem;
    bool dirty;
    block_sector_t sector;
    struct lock block_lock;
    char data[BLOCK_SECTOR_SIZE];
}


struct list cache_blocks;
struct lock process_lock;


struct lock cache_list_lock;
int number_of_req;
struct lock lock_number_of_req;
struct condition cond;


void* read_cache(block_sector_t sector,void *buffer_, off_t size, off_t offset);
void* write_cache(block_sector_t sector,void *buffer_, off_t size, off_t offset);


```
`cache_block`: 

داده ساختاری که هر یک از خانه‌های حافظه‌ی نهان را تشکیل می‌دهند. 

`sector`:

برای پیدا کردن بلاک مورد نظر در حافظه‌ی 
نهان

`block_lock:`

ممکن است در حین نوشتن داخل یک بلاک، یا جایگزینی یک بلاک، پردازه‌ای بخواهد از آن بلاک استفاده کند. قفل `block_lock` برای همگام سازی در این مواقع است.


`data:`

داده‌ی داخل دیسک در این قسمت ذخیره می‌شود.

‍`cache_blocks:`

حافظه‌ی نهان سیستم عامل

`cache_list_lock:`

برای جلوگیری از دسترسی همزمان دو پروسس مختلف از حافظه‌ی نهان

```
struct lock cache_list_lock;
int number_of_req;
struct lock lock_number_of_req;
struct condition cond;
```

این بخش در صورتی استفاده می‌شود که برای `read-ahead` بخواهیم چند بلاک را داخل حافظه‌ی نهان جایگزین کنیم.
برای اینکار باید مطمئن شویم که هیچ تردی از بلاک‌های حافظه در حال استفاده نباشد. برای جایگزینی بلاک‌ها ابتدا قفل `cache_list_lock` گرفته می‌شود تا دیگر هیچ ترد یا پروسه‌ای نتواند از لیست استفاده کند. متغیر `number_of_req` نشان‌ دهنده‌ی تعداد بلاک‌های مورد استفاده است. 
در صورتی که مقدار این متغیر صفر باشد عملیات جایگزینی شروع می‌شود و در غیر این صورت ترد مربوطه منتظر می‌شود تا این متغیر برابر صفر شود. بعد از صفر شدن این متغیر ترد با استفاده از تابع `signal`  بیدار شده و عملیات جایگزینی ادامه پیدا می‌کند.

‍
‍‍‍‍
```
void* read_cache(block_sector_t sector,void *buffer_, off_t size, off_t offset);
void* write_cache(block_sector_t sector,void *buffer_, off_t size, off_t offset);
```
این بخش درون تابع‌های ‍‍
‍ `inode_write_at()`
و ‍‍`inode_read_at()`
فراخوانی می‌شود. در این توابع در صورتی که سکتور خواسته شده درون حافظه‌ی نهان وجود داشته باشد، عملیات نوشتن و یا خواندن درون این حافظه صورت می‌گیرد.


الگوریتم‌ها
------------

>> توضیح دهید که الگوریتم مورد استفاده‌ی شما به چه صورت یک بلاک را برای جایگزین  شدن انتخاب می‌کند؟

با دسترسی به هر بلاک برای خواندن یا نوشتن، آن بلاک به ابتدای لیست  `cache_block`
منتقل می‌شود و خارج کردن بلاک‌ها از انتهای لیست صورت می‌گیرد. به این صورت الگوریتم     `LRU` پیاده سازی می‌شود.

>> روش پیاده‌سازی `read-ahead` را توضیح دهید.

بعد از فراخوانی بلاکی که درون حافظه‌ی نهان موجود نیست، علاوه بر جایگزینی خود بلاک در لیست، بلاک‌های مجاور آن بلاک نیز جایگزین بلاک‌های داخل `block_list`
می‌شوند.
این اتفاق به کمک متغیر‌های `cond` و `lock_block_list` صورت مي‌گیرد. 

همگام سازی
-------------

>> هنگامی که یک پردازه به طور مستمر در حال خواندن یا نوشتن داده در یک بلاک بافرکش می‌باشد به چه صورت از دخالت سایر پردازه‌ها جلوگیری میشود؟

زمانی که یک پردازنده با کش کار دارد قفل `process_lock` را می‌گیردو در نتیجه سایر پردازنده‌ها نمی‌توانند از کش استفاده کنند.

>> در حین خارج شدن یک بلوک از حافظه‌ی نهان، چگونه از پروسه‌های دیگر جلوگیری می‌شود تا به این بلاک دسترسی پیدا نکنند؟

برای هر یک از بلاک‌های حافظه‌ی نهان یک قفل قرار داده شده که پروسه‌های دیگر در صورت لزوم به آن بلاک دسترسی نداشته باشند.

منطق طراحی
-----------------

>> یک سناریو را توضیح دهید که از بافر کش، `read-ahead` و یا از `write-behind` استفاده کند.
`` 

`read_ahead`:

زمان‌هایی که نیاز است آرایه‌های بزرگ ذخیره شده (مثلا ارایه‌هایی که اعداد بزرگ را نگه میدارند)
استفاده شود، `read-ahead` کمک کننده خواهد بود. زمانی که افراد فیلم می‌بینند هم `read-ahead` کمک کننده است.

`write-behind:`

زمان‌هایی که سیستم `crash` می‌کند یا به هنگام قطعی برق ممکن است کمک کننده باشد.


فایل‌های قابل گسترش
=====================

داده‌ساختار‌ها و توابع
---------------------

>> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ایجاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

All the changes are in `inode.c`.

**Additions**
```c
#define DIRECT_BLOCKS 8
```
```c
typedef enum {
    INODE_TYPE_FILE,
    INODE_TYPE_DIRECTORY
} inode_type_t;
```
This enum is used to distinguish between files and directories in `struct inode_disk`.

**Modifications**
```c
/* On-disk inode. 
Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
{
    off_t length;         /* File size in bytes. */
    unsigned magic;       /* Magic number. */
    inode_type_t type;    /* Type of the inode */
    block_sector_t data[DIRECT_BLOCKS]; /* Direct blocks */
    block_sector_t indirect; /* Indirect block */
    block_sector_t double_indirect; /* Double indirect block */
    uint32_t unused[115]; /* Not used. */
};
```
Here we used [FFS](https://dsf.berkeley.edu/cs262/FFS-annotated.pdf) paper as reference to determine the structure of `struct inode_disk`. As mentioned in this paper, we used 8 direct blocks, a indirect block, and a double indirect block.

The size of the variable `unused` is equals to:
$$125+1-8-2-1=115$$
```c
/* In-memory inode. */
struct inode
{
    struct list_elem elem;  /* Element in inode list. */
    block_sector_t sector;  /* Sector number of disk location. */
    int open_cnt;           /* Number of openers. */
    bool removed;           /* True if deleted, false otherwise. */
    int deny_write_cnt;     /* 0: writes ok, >0: deny writes. */
    struct inode_disk data; /* Inode content. */
    int readers;            /* Number of readers. */
    struct lock write_lock; /* Lock for write operations. */
    struct condition cond;  /* Conditional variable for waiting writers. */
};
```
The members `readers`, `write_lock`, and `cond` are added to `struct inode`. The reason for this is explained in the next section.

>> بیشترین سایز فایل پشتیبانی شده توسط ساختار inode شما چقدر است؟
$$
(8 + 128 + 128 \times 128) \times 512 = 8458240~B = 8.4~MB
$$
So possible maximum size of the file is 8.4 MB.

همگام سازی
----------

>> توضیح دهید که اگر دو پردازه بخواهند یک فایل را به طور همزمان گسترش دهند، کد شما چگونه از حالت مسابقه جلوگیری می‌کند.

As mentioned before, `write_lock` is added to `struct inode`. Every process wishing to write to a file must acquire this lock before writing to the file. So the mentioned race condition is irrelevant.

>> فرض کنید دو پردازه‌ی A و B فایل F را باز کرده‌اند و هر دو به end-of-file اشاره کرده‌اند. اگر  همزمان A از F بخواند و B روی آن بنویسد، ممکن است که A تمام، بخشی یا هیچ چیز از اطلاعات نوشته شده توسط B را بخواند. همچنین A نمی‌تواند چیزی جز اطلاعات نوشته شده توسط B را بخواند. مثلا اگر B تماما ۱ بنویسد، A نیز باید تماما ۱ بخواند. توضیح دهید کد شما چگونه از این حالت مسابقه جلوگیری می‌کند.

Every process wishing to read a file, just check if the `write_lock` is held by any other process. If it is held, then the process must wait until the `write_lock` is released. Otherwise, the process can read the file. This mechanism can handle the case where a process is writing to a file and another process wants to read from the same file. What if a process is reading from a file and another process wants to write to the same file? For this situation, we have introduced `readers` member in `struct inode`. This member is used to count the number of processes reading from the file. A process wishing to write to a file must wait until the `readers` is zero. For this purpose we used `cond` member in `struct inode`.

>> توضیح دهید همگام سازی شما چگونه "عدالت" را برقرار می‌کند. فایل سیستمی "عادل" است که خواننده‌های اطلاعات به صورت ناسازگار نویسنده‌های اطلاعات را مسدود نکنند و برعکس. بدین ترتیب اگر تعدادی بسیار زیاد پردازه‌هایی که از یک فایل می‌خوانند نمی‌توانند تا ابد مانع نوشده شدن اطلاعات توسط یک پردازه‌ی دیگر شوند و برعکس.

Processes wishing to read from a file must wait if one or more processes wants to write to the same file and waiting for some other processes to finish their operation. This way fairness is guaranteed.

منطق طراحی
----------

>> آیا ساختار `inode` شما از طبقه‌بندی چند سطحه پشتیبانی می‌کند؟ اگر بله، دلیل خود را برای انتخاب این ترکیب خاص از بلوک‌های مستقیم، غیر مستقیم و غیر مستقیم دوطرفه توضیح دهید.‌ اگر خیر، دلیل خود برای انتخاب ساختاری غیر از طبقه‌بندی چند سطحه و مزایا و معایب ساختار مورد استفاده خود نسبت به طبقه‌بندی چند سطحه را توضیح دهید.

As suggested in the handout, we have used UNIX FFS as the base of our file system. So, yes, we have used multi-level indexing. We have decided to store files less than $8 \times 512~B$ in the inode block. This enhances the performance of our file system and also makes it more efficient for small files. Larger files up untill $8 \times 512 + 128 \times 512~B$ are stored in the indirect block. They are slower to access. Even larger files are stored in the double indirect block. They are slowest to access. It worth mentioning that these values are ones used in UNIX FFS.

زیرمسیرها
============

داده‌ساختار‌ها و توابع
---------------------

>> در این قسمت تعریف هر یک از `struct` ها، اعضای `struct` ها، متغیرهای سراسری یا ایستا، `typedef` ها یا `enum` هایی که ایجاد کرده‌اید یا تغییر داده‌اید را بنویسید و دلیل هر کدام را در حداکثر ۲۵ کلمه توضیح دهید.

thead.h
```c
struct thread {
    ...
    struct dir *cur_dir; // thread current directory
};
```

directory.c
```c
struct dir {
  ...
  struct lock dir_lock; //lock on directory
};
```

الگوریتم‌ها
-----------

>> کد خود را برای طی کردن یک مسیر گرفته‌شده از کاربر را توضیح دهید. آیا عبور از مسیرهای absolute و relative تفاوتی دارد؟

با کانکت کردن مسیر relative و متغیر `cur_dir` مسیر absolute را بدست می آوریم.  


برای عبور از مسیر absolute از `ROOT_DIR_SECTOR` (Root directory file inode sector) استفاده میکنیم و با تابع `dir_lookup` DIR یک فایل را با اسم آن جست و جو میکنیم.

همگام سازی
-------------

>> چگونه از رخ دادن race-condition در مورد دایرکتوری ها پیشگیری می‌کنید؟ برای مثال اگر دو درخواست موازی برای حذف یک فایل وجود داشته باشد و  تنها یکی از آنها باید موفق شود یا مثلاً دو ریسه موازی بخواهند فایلی یک اسم در یک مسیر ایجاد کنند و مانند آن. آیا پیاده سازی شما اجازه می‌دهد مسیری که CWD یک ریسه شده یا پردازه‌ای از آن استفاده می‌کند حذف شود؟ اگر بله، عملیات فایل سیستم بعدی روی آن دایرکتوری چه نتیجه‌ای می‌دهند؟ اگر نه، چطور جلوی آن را می‌گیرید؟


یک قفل مجزا با هر ساختار dir مرتبط میکنیم (`dir_lock`).
 هر زمان که عملیات thread_safe با dir انجام شود از این قفل استفاده میشود. از آنجایی که هر dir با سطح متفاوتی در درخت دایرکتوری مرتبط است،  در عملیات‌هایی که همان dir را تغییر می‌دهند، از حذف متقابل اطمینان حاصل میکنیم.

منطق طراحی
-----------------

>> توضیح دهید چرا تصمیم گرفتید CWD یک پردازه را به شکلی که طراحی کرده‌اید پیاده‌سازی کنید

در این پیاده سازی تنها نیاز به اضافه کردن دایرکتوری برای هر رشته (`cur_dir`) و لاک برای هر دایرکتوری (`dir_lock`) داریم.  این پیاده سازی ساده و بهینه است و پیچیدگی اضافی ندارد و میتوان به آسانی در درخت دایرکتوری حرکت کرد.

سوال های افزون بر طراحی
============
Upon the startup of the kernel, a thread is created which has two purposes: handling the dirty data which is indicated in the cache by a single bit (1 represents corruption) and also, every time a piece of data is to be removed from the cache, it must be stored on the disk and indicated as ‘dirty’, so that the same thread can restore it upon the start of the kernel.
To implement the read-ahead functionality in the kernel, two tasks must be done: 1. the operating system holds a list of essential files and preloads them into the cache upon the startup of the system; this results in a quikcer boot.
2. the kernel takes advantage of the writing on the disk being sequential: when an application requests access to block A, a few other blocks are also fetched into the cache since that app will most likely want to access those blocks as well.

### سوالات نظرسنجی

پاسخ به این سوالات دلخواه است، اما به ما برای بهبود این درس در ادامه کمک خواهد کرد.

نظرات خود را آزادانه به ما بگوئید—این سوالات فقط برای سنجش افکار شماست.

ممکن است شما بخواهید ارزیابی خود از درس را به صورت ناشناس و در انتهای ترم بیان کنید.

>> به نظر شما، این تمرین گروهی، یا هر کدام از سه وظیفه آن، از نظر دشواری در چه سطحی بود؟ خیلی سخت یا خیلی آسان؟

>> چه مدت زمانی را صرف انجام این تمرین کردید؟ نسبتا زیاد یا خیلی کم؟

>> آیا بعد از کار بر روی یک بخش خاص از این تمرین (هر بخشی)، این احساس در شما به وجود آمد که اکنون یک دید بهتر نسبت به برخی جنبه‌های سیستم عامل دارید؟

>> آیا نکته یا راهنمایی خاصی وجود دارد که بهتر است ما آنها را به توضیحات این تمرین اضافه کنیم تا به دانشجویان ترم های آتی در حل مسائل کمک کند؟

>> متقابلا، آیا راهنمایی نادرستی که منجر به گمراهی شما شود وجود داشته است؟

>> آیا پیشنهادی در مورد دستیاران آموزشی درس، برای همکاری موثرتر با دانشجویان دارید؟

این پیشنهادات میتوانند هم برای تمرین‌های گروهی بعدی همین ترم و هم برای ترم‌های آینده باشد.

>> آیا حرف دیگری دارید؟
