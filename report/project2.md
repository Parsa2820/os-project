# Group Homework 2 - Scheduling

## Table of Contents
- [Group Homework 2 - Scheduling](#group-homework-2---scheduling)
  - [Table of Contents](#table-of-contents)
  - [Contributers](#contributers)
  - [Changes Compared to The Design Document](#changes-compared-to-the-design-document)
    - [Sleep timer](#sleep-timer)
    - [Priority scheduling](#priority-scheduling)
  - [Group Members Contributions](#group-members-contributions)

## Contributers
- Parsa Mohammadian <parsa2820@gmail.com>

- Arian Yazdan Parast <arian.yazdan2001@gmail.com>

- Sara Azarnoush <azarnooshsa@gmail.com> 

- Kasra Amani <kasra138079@gmail.com>

## Changes Compared to [The Design Document](../design/project2-design.md)
### Sleep timer

In the beginning and when the target design was written, a lock was implemented to handle a specific race condition but in practice this condition never occurred, so the lock was removed to make the implementation code as simple as possible.

### Priority scheduling

The "struct * lock waiting" parameter has been added to the thread struct. This parameter is added to specify the lock that the thread is waiting for, and it is used to donate nested priority. The "list_elem elem" parameter has been added to the lock struct so that the linked list feature in the "acquired_locks" list can be used. The "struct thread * thread" parameter has been added to the "semaphore_elem" struct so that a thread with higher priority in the conditional variables waiting can be selected.

## Group Members Contributions
- Parsa Mohammadian: Scheduling labratory
- Arian Yazdanparast: Priority scheduling
- Sara Azarnoosh and Kasra Amani: Sleep timer + report 
