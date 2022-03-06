تمرین گروهی ۱/۰ - آشنایی با pintos
======================

شماره گروه:
-----
> نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

پارسا محمدیان <parsa2820@gmail.com>

آرین یزدانپرست <arian.yazdan2001@gmail.com>

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

۷.

۸.

۹.

۱۰.

۱۱.

۱۲.

۱۳.


## دیباگ

۱۴.

۱۵.

۱۶.

۱۷.

۱۸.

۱۹.