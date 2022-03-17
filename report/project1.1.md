# Group Homework 1.1 - User Program

## Table of Contents
- [Group Homework 1.1 - User Program](#group-homework-11---user-program)
  - [Table of Contents](#table-of-contents)
  - [Contributers](#contributers)
  - [Changes Compared to The Design Document](#changes-compared-to-the-design-document)
    - [Minor Changes](#minor-changes)
    - [New Data Structures](#new-data-structures)
  - [Group Members Contributions](#group-members-contributions)
  - [Quality Assurance Questions](#quality-assurance-questions)

## Contributers
- Parsa Mohammadian <parsa2820@gmail.com>

- Arian Yazdan Parast <arian.yazdan2001@gmail.com>

- Sara Azarnoush <azarnooshsa@gmail.com> 

- Kasra Amani <kasra138079@gmail.com>

## Changes Compared to [The Design Document](../design/project1.1-design.md)
### Minor Changes
- We used the already defined doubly linked list to store the data instead of custom one we had proposed. The reasoning behind this decision is that the mentioned liked list has been implemented in a generic manner and was easy to use.
### New Data Structures
We added a new data structure in order to store related system call information.
```c
/* syscall.h */
typedef void syscall_func_t(struct intr_frame *, uint32_t *);

typedef struct syscall_descriptor
{
    int number;
    syscall_func_t *syscall_func;
    int is_file_op;
} syscall_descriptor_t;
```
The reason for this decision is the convinence of adding a new system call without changing the code. This somehow ressembles to the open/close principle in object oriented programming.

## Group Members Contributions
- Parsa Mohammadian: Implementing system calls on files and also "practice" and "halt" commands and creation of the report file, all in collaboration with Ms. Azarnoosh; also assisting Mr. Yazdanparast in the implementation of "wait" and "exec" commands.
- Arian Yazdanparast: Implementation of "wait" and "exec" commands and passing arguements from the command line to a process.
- Sara Azarnoosh: Collaboration on the tasks of Mr. Mohammadian.
- Kasra Amani: Collaboration on the tasks of Mr. Yazdanparast and completion of this document.

## Quality Assurance Questions
>Are there any known security holes in your design?

No, nothing that we know about.
>Is your coding style uniform with eachother and with the pintos skeleton code?

The whole source code was reformatted in the beginning and the style from there was the same across all contributors.
>Is your code easy to understand?

Proper variable and method names are used across the implementation; so yes, it is easy enough to understand.
>Are there any complicated pieces in your design? if so, are they commented properly for a reader to understand?

There is no overcomplicated piece of code in our design; everything is quite modular and easy to understand; methods have been implemented wherever possible and one can easily understand and follow the execution of the program and there are comments wherever the developer deemed necessary.
>Are there any commented pieces of code in your final design?

No, there are none.
>Have you used any code that you have copied from another source?

No, there are none.
>Have you used the linked list implementation given in the skeleton or have you implemented you own?

We used the structure that was provided by the staff.
>Have you gone over the 100 character limit in a line that is common among developers?

No, this has not been done and the lines are short enough.
>Are there any **binary** files in your git repository?

No, there are none.