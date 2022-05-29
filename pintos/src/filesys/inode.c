#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"





struct cache_block{
    struct list_elem elem;
    bool dirty;
    block_sector_t sector;
    struct lock block_lock;
    char data[BLOCK_SECTOR_SIZE];
};

struct list cache_blocks;
struct lock cache_list_block;

struct cache_block cache_blocks_mem[64];

static
void init_block(struct cache_block * block){
  lock_init(&(block -> block_lock));
  block -> dirty = 0;
}


void init_cache(void){
  list_init(&cache_blocks);
  lock_init(&cache_list_block);
  for (int i = 0; i < 64; i++){
    init_block(&cache_blocks_mem[i]);
    list_push_back(&cache_blocks, &cache_blocks_mem[i].elem);
  }
}

static inline bool
is_head(struct list_elem *elem)
{
  return elem != NULL && elem->prev == NULL && elem->next != NULL;
}

static 
void write_to_memory(struct cache_block * block){
  lock_acquire(&block->block_lock);
  block_write (fs_device, block -> sector, block -> data);
  block -> dirty = 0;
  lock_release(&block->block_lock);
}

static 
void * replace_block(block_sector_t sector){
  lock_acquire(&cache_list_block);
  struct list_elem * element = list_front(&cache_blocks);
  struct cache_block * block_to_remove = list_entry(element, struct cache_block, elem);
  lock_release(&cache_list_block);
  if (block_to_remove -> dirty == 1){
    write_to_memory(block_to_remove);
  }
  block_read (fs_device, sector, block_to_remove->data);
  block_to_remove->sector = sector;
  list_remove(&block_to_remove->elem);
  list_push_back(&cache_blocks ,&block_to_remove->elem);
  return block_to_remove;
}

static 
struct cache_block * find_cache_block(block_sector_t sector){
  lock_acquire(&cache_list_block);
  struct list_elem * element = list_back(&cache_blocks);
  lock_release(&cache_list_block);
  while (1)
  {
    lock_acquire(&cache_list_block);
    struct cache_block * block = list_entry(element, struct cache_block, elem);
    if (block -> sector == sector){
        list_remove(&block->elem);
        list_push_back(&cache_blocks, &block->elem);
        lock_release(&cache_list_block);
        return block;
    }
    element = list_prev(element);
    if (is_head(element)){
      break;
    }
    lock_release(&cache_list_block);
  }
  
  lock_release(&cache_list_block);
  
  return replace_block(sector);
}




static 
void write_cache(block_sector_t sector,void *buffer_,off_t size,off_t offset){
    struct cache_block * block = find_cache_block(sector);
    if (size > BLOCK_SECTOR_SIZE){
      while (size > BLOCK_SECTOR_SIZE)
      {
        lock_acquire(&block->block_lock);
        memcpy(&(block->data[offset]), buffer_, BLOCK_SECTOR_SIZE);
        block->dirty = 1;
        sector++;
        offset = 0;
        buffer_ += BLOCK_SECTOR_SIZE;
        block = find_cache_block(sector);
        size -= BLOCK_SECTOR_SIZE;
        lock_release(&block->block_lock);
      }
      lock_acquire(&block->block_lock);
      memcpy(&block->data[offset], buffer_, size);
      block->dirty = 1;
      lock_release(&block->block_lock);
    }else{
      lock_acquire(&(block->block_lock));
      memcpy(&(block->data[offset]), buffer_, size);
      block->dirty = 1;
      lock_release(&(block->block_lock));
      
    }
}

static
void read_cache(block_sector_t sector,void *buffer_,off_t size,off_t offset){
  struct cache_block * block = find_cache_block(sector);
  if (size > BLOCK_SECTOR_SIZE){
      while (size > BLOCK_SECTOR_SIZE)
      {
        lock_acquire(&block->block_lock);
        memcpy(buffer_, &block->data[offset], BLOCK_SECTOR_SIZE);
        sector++;
        offset = 0;
        buffer_ += BLOCK_SECTOR_SIZE;
        block = find_cache_block(sector);
        size -= BLOCK_SECTOR_SIZE;
        lock_release(&block->block_lock);
      }
      lock_acquire(&block->block_lock);
      memcpy(buffer_, &block->data[offset],  size);
      lock_release(&block->block_lock);
    } else {
      lock_acquire(&block->block_lock);
      memcpy(buffer_, &block->data[offset], size);
      lock_release(&block->block_lock);
    } 
}

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44


/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
   
struct inode_disk
{
  block_sector_t start; /* First data sector. */
  off_t length;         /* File size in bytes. */
  unsigned magic;       /* Magic number. */
  uint32_t unused[125]; /* Not used. */
};

struct inode
{
  struct list_elem elem;  /* Element in inode list. */
  block_sector_t sector;  /* Sector number of disk location. */
  int open_cnt;           /* Number of openers. */
  bool removed;           /* True if deleted, false otherwise. */
  int deny_write_cnt;     /* 0: writes ok, >0: deny writes. */
  struct inode_disk data; /* Inode content. */
};



/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors(off_t size)
{
  return DIV_ROUND_UP(size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */


/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector(const struct inode *inode, off_t pos)
{
  ASSERT(inode != NULL);
  if (pos < inode->data.length)
    return inode->data.start + pos / BLOCK_SECTOR_SIZE;
  else 
    return -1;
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void inode_init(void)
{
  list_init(&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      if (free_map_allocate (sectors, &disk_inode->start))
        {
          block_write (fs_device, sector, disk_inode);
          if (sectors > 0)
            {
              static char zeros[BLOCK_SECTOR_SIZE];
              size_t i;

              for (i = 0; i < sectors; i++)
                block_write (fs_device, disk_inode->start + i, zeros);
            }
          success = true;
        }
      free (disk_inode);
    }
  return success;
}
/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e))
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector)
        {
          inode_reopen (inode);
          return inode;
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  block_read (fs_device, inode->sector, &inode->data);
  return inode;
}
/* Reopens and returns INODE. */
struct inode *
inode_reopen(struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber(const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void inode_close(struct inode *inode)
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
  {
    /* Remove from inode list and release lock. */
    list_remove(&inode->elem);

    /* Deallocate blocks if removed. */
    if (inode->removed)
    {
      free_map_release(inode->sector, 1);
      free_map_release(inode->data.start,
                       bytes_to_sectors(inode->data.length));
    }

    free(inode);
  }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void inode_remove(struct inode *inode)
{
  ASSERT(inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t inode_read_at(struct inode *inode, void *buffer_, off_t size, off_t offset)
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;

  while (size > 0)
  {
    /* Disk sector to read, starting byte offset within sector. */
    block_sector_t sector_idx = byte_to_sector(inode, offset);
    int sector_ofs = offset % BLOCK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length(inode) - offset;
    int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually copy out of this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    read_cache(sector_idx, buffer + bytes_read, chunk_size, sector_ofs);

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_read += chunk_size;
  }

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t inode_write_at(struct inode *inode, const void *buffer_, off_t size,
                     off_t offset)
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
    return 0;

  while (size > 0)
  {
    /* Sector to write, starting byte offset within sector. */
    block_sector_t sector_idx = byte_to_sector(inode, offset);
    int sector_ofs = offset % BLOCK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length(inode) - offset;
    int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually write into this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;


    write_cache(sector_idx, buffer + bytes_written, chunk_size, sector_ofs);
    

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_written += chunk_size;
  }

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void inode_deny_write(struct inode *inode)
{
  inode->deny_write_cnt++;
  ASSERT(inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void inode_allow_write(struct inode *inode)
{
  ASSERT(inode->deny_write_cnt > 0);
  ASSERT(inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t inode_length(const struct inode *inode)
{
  return inode->data.length;
}



