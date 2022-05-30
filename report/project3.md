# Group Homework 3 - File System

## Table of Contents
- [Group Homework 3 - File System](#group-homework-3---file-system)
  - [Table of Contents](#table-of-contents)
  - [Contributers](#contributers)
  - [Changes Compared to The Design Document](#changes-compared-to-the-design-document)
    - [Buffer Cache](#buffer-cache)
    - [Extensible Files](#extensible-files)
    - [Subdirectories](#subdirectories)
  - [Group Members Contributions](#group-members-contributions)

## Contributers
- Parsa Mohammadian <parsa2820@gmail.com>

- Arian Yazdan Parast <arian.yazdan2001@gmail.com>

- Sara Azarnoush <azarnooshsa@gmail.com> 

- Kasra Amani <kasra138079@gmail.com>

## Changes Compared to [The Design Document](../design/project3-design.md)
### Buffer Cache

### Extensible Files
- A single lock is used instead of synchronization mechanism explained in the design document.
- Struct `indirect_block_t` is added to `inode.c`.
- Structs and enums which were previously defined in `inode.c` are moved to `inode.h` in order to make them visible to other files.

### Subdirectories

## Group Members Contributions
- Parsa Mohammadian: Extensible files
- Arian Yazdanparast: Buffer cache
- Sara Azarnoosh and Kasra Amani: Subdirectories
