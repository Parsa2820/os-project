# Group Homework 1.1 - User Program

## Table of Contents
- [Contributers](#contributers)
- [Changes Compared to The Design Document](#changes-compared-to-the-design-document)
- [Group Members Contributions](#group-members-contributions)
- [Quality Assurance Questions](#quality-assurance-questions)

## Contributers
- Parsa Mohammadian <parsa2820@gmail.com>

- Arian Yazdan Parast <arian.yazdan2001@gmail.com>

- Sara Azarnoush <azarnooshsa@gmail.com> 

- Kasra Amani <kasra138079@gmail.com>

## Changes Compared to [The Design Document](../design/project1.1-design.md)
### Minor Changes
- We used the already defined doubly linked list to store the data instead of custom one we have proposed. The reason behind this decision is that the mentioned liked list has been implemented in generic manner and was easy to use.
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
The reason for this decision is the convinent of adding new system call without changing the code. This somehow ressembles to open/close principle in OOP.

## Group Members Contributions

## Quality Assurance Questions