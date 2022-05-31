#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"
#include "threads/synch.h"

/* Number of direct blocks. */
#define DIRECT_BLOCKS 8
/* Number of indirect blocks. */
#define INDIRECT_BLOCKS 128
/* Number of double indirect blocks. */
#define DOUBLE_INDIRECT_BLOCKS 128 * 128

typedef enum
{
  INODE_TYPE_FILE,
  INODE_TYPE_DIRECTORY
} inode_type_t;

/* On-disk inode.
Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
{
  off_t length;                       /* File size in bytes. */
  unsigned magic;                     /* Magic number. */
  inode_type_t type;                  /* Type of the inode */
  block_sector_t data[DIRECT_BLOCKS]; /* Direct blocks */
  block_sector_t indirect;            /* Indirect block */
  block_sector_t double_indirect;     /* Double indirect block */
  uint32_t unused[115];               /* Not used. */
};

/* In-memory inode. */
struct inode
{
  struct list_elem elem;      /* Element in inode list. */
  block_sector_t sector;      /* Sector number of disk location. */
  int open_cnt;               /* Number of openers. */
  bool removed;               /* True if deleted, false otherwise. */
  int deny_write_cnt;         /* 0: writes ok, >0: deny writes. */
  struct inode_disk data;     /* Inode content. */
  // int readers;             /* Number of readers. */
  struct lock operation_lock; /* Lock for operations. */
  // struct condition cond;   /* Conditional variable for waiting writers. */
};

struct bitmap;
int cache_hit;
int cache_miss;
void reset_counter(void);
void flush_cache(struct block *);
int write_cnt;
int read_cnt;
void reset_cache(void);
void inode_init(void);
bool inode_create(block_sector_t, off_t, inode_type_t);
struct inode *inode_open(block_sector_t);
struct inode *inode_reopen(struct inode *);
block_sector_t inode_get_inumber(const struct inode *);
void inode_close(struct inode *);
void inode_remove(struct inode *);
off_t inode_read_at(struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at(struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write(struct inode *);
void inode_allow_write(struct inode *);
off_t inode_length(const struct inode *);
bool inode_is_dir(struct inode *);
int inode_sector(struct inode *);
void init_cache(void);

#endif /* filesys/inode.h */
