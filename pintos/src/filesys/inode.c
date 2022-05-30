#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

struct cache_block
{
  struct list_elem elem;
  bool dirty;
  block_sector_t sector;
  struct lock block_lock;
  char data[BLOCK_SECTOR_SIZE];
};

struct list cache_blocks;
struct lock cache_list_block;

struct cache_block cache_blocks_mem[64];

static void init_block(struct cache_block *block)
{
  lock_init(&(block->block_lock));
  block->dirty = 0;
}

void init_cache(void)
{
  cache_hit = 0;
  cache_miss = 0;
  write_cnt = 0;
  read_cnt = 0;
  list_init(&cache_blocks);
  lock_init(&cache_list_block);
  for (int i = 0; i < 64; i++)
  {
    init_block(&cache_blocks_mem[i]);
    list_push_back(&cache_blocks, &cache_blocks_mem[i].elem);
  }
}

static inline bool
is_head(struct list_elem *elem)
{
  return elem != NULL && elem->prev == NULL && elem->next != NULL;
}

static void write_to_memory(struct cache_block *block, off_t size)
{
  lock_acquire(&block->block_lock);
  block_write(fs_device, block->sector, block->data);
  block->dirty = 0;
  write_cnt++;
  lock_release(&block->block_lock);
}

static void *replace_block(block_sector_t sector, off_t size)
{
  lock_acquire(&cache_list_block);
  struct list_elem *element = list_front(&cache_blocks);
  struct cache_block *block_to_remove = list_entry(element, struct cache_block, elem);
  cache_miss++;
  lock_release(&cache_list_block);

  if (block_to_remove->dirty == 1)
  {
    write_to_memory(block_to_remove, size);
  }

  lock_acquire(&cache_list_block);
  block_read(fs_device, sector, block_to_remove->data);
  block_to_remove->sector = sector;
  read_cnt++;
  list_remove(&block_to_remove->elem);
  list_push_back(&cache_blocks, &block_to_remove->elem);
  lock_release(&cache_list_block);
  return block_to_remove;
}

static struct cache_block *find_cache_block(block_sector_t sector, off_t size)
{
  lock_acquire(&cache_list_block);
  struct list_elem *element = list_back(&cache_blocks);
  lock_release(&cache_list_block);
  while (1)
  {
    lock_acquire(&cache_list_block);
    struct cache_block *block = list_entry(element, struct cache_block, elem);
    if (block->sector == sector)
    {
      list_remove(&block->elem);
      list_push_back(&cache_blocks, &block->elem);
      lock_release(&cache_list_block);
      cache_hit++;
      return block;
    }
    element = list_prev(element);
    if (is_head(element))
    {
      break;
    }
    lock_release(&cache_list_block);
  }

  lock_release(&cache_list_block);

  return replace_block(sector, size);
}

static void write_cache(block_sector_t sector, void *buffer_, off_t size, off_t offset)
{
  struct cache_block *block = find_cache_block(sector, size);
  if (size > BLOCK_SECTOR_SIZE)
  {
    while (size > BLOCK_SECTOR_SIZE)
    {
      lock_acquire(&block->block_lock);
      memcpy(&(block->data[offset]), buffer_, BLOCK_SECTOR_SIZE);
      block->dirty = 1;
      sector++;
      offset = 0;
      buffer_ += BLOCK_SECTOR_SIZE;
      block = find_cache_block(sector, size);
      size -= BLOCK_SECTOR_SIZE;
      lock_release(&block->block_lock);
    }
    if (size != 0)
    {
      lock_acquire(&block->block_lock);
      memcpy(&block->data[offset], buffer_, size);
      block->dirty = 1;
      lock_release(&block->block_lock);
    }
  }
  else
  {
    if (size != 0)
    {
      lock_acquire(&(block->block_lock));
      memcpy(&(block->data[offset]), buffer_, size);
      block->dirty = 1;
      lock_release(&(block->block_lock));
    }
  }
}

static void read_cache(block_sector_t sector, void *buffer_, off_t size, off_t offset)
{
  struct cache_block *block = find_cache_block(sector, size);
  if (size > BLOCK_SECTOR_SIZE)
  {
    while (size > BLOCK_SECTOR_SIZE)
    {
      lock_acquire(&block->block_lock);
      memcpy(buffer_, &block->data[offset], BLOCK_SECTOR_SIZE);
      sector++;
      offset = 0;
      buffer_ += BLOCK_SECTOR_SIZE;
      block = find_cache_block(sector, size);
      size -= BLOCK_SECTOR_SIZE;
      lock_release(&block->block_lock);
    }
    lock_acquire(&block->block_lock);
    memcpy(buffer_, &block->data[offset], size);
    lock_release(&block->block_lock);
  }
  else
  {
    lock_acquire(&block->block_lock);
    memcpy(buffer_, &block->data[offset], size);
    lock_release(&block->block_lock);
  }
}

void reset_cache(void)
{
  lock_acquire(&cache_list_block);
  struct list_elem *element = list_back(&cache_blocks);
  lock_release(&cache_list_block);
  while (1)
  {
    lock_acquire(&cache_list_block);
    struct cache_block *block = list_entry(element, struct cache_block, elem);
    if (block->dirty)
      write_to_memory(block, 0);
    lock_acquire(&block->block_lock);
    block->sector = -1;
    lock_release(&block->block_lock);
    element = list_prev(element);
    lock_release(&cache_list_block);
    if (is_head(element))
    {
      break;
    }
  }
  cache_miss = 0;
  cache_hit = 0;
}


void reset_counter(void)
{
  lock_acquire(&cache_list_block);
  struct list_elem *element = list_back(&cache_blocks);
  lock_release(&cache_list_block);
  while (1)
  {
    lock_acquire(&cache_list_block);
    struct cache_block *block = list_entry(element, struct cache_block, elem);
    if (block->dirty)
      write_to_memory(block, 0);
    element = list_prev(element);
    lock_release(&cache_list_block);
    if (is_head(element))
    {
      break;
    }
  }
  read_cnt = 0;
  write_cnt = 0;
}


/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

typedef struct
{
  block_sector_t sectors[INDIRECT_BLOCKS];
} indirect_block_t;

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors(off_t size)
{
  return DIV_ROUND_UP(size, BLOCK_SECTOR_SIZE);
}

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector(const struct inode *inode, off_t pos)
{
  ASSERT(inode != NULL);

  if (pos > inode->data.length)
    return -1;

  size_t sector_index = pos / BLOCK_SECTOR_SIZE;

  // Direct blocks
  if (sector_index < DIRECT_BLOCKS)
    return inode->data.data[sector_index];

  // Indirect blocks
  sector_index -= DIRECT_BLOCKS;
  if (sector_index < INDIRECT_BLOCKS)
  {
    indirect_block_t *indirect_block = calloc(1, sizeof(indirect_block_t));
    block_read(fs_device, inode->data.indirect, indirect_block);
    block_sector_t sector = indirect_block->sectors[sector_index];
    free(indirect_block);
    return sector;
  }

  // Double indirect blocks
  sector_index -= INDIRECT_BLOCKS;
  if (sector_index < DOUBLE_INDIRECT_BLOCKS)
  {
    indirect_block_t *double_indirect_block = calloc(1, sizeof(indirect_block_t));
    block_read(fs_device, inode->data.double_indirect, double_indirect_block);
    block_sector_t sector = double_indirect_block->sectors[sector_index / INDIRECT_BLOCKS];
    free(double_indirect_block);
    indirect_block_t *indirect_block = calloc(1, sizeof(indirect_block_t));
    block_read(fs_device, sector, indirect_block);
    sector = indirect_block->sectors[sector_index % INDIRECT_BLOCKS];
    free(indirect_block);
    return sector;
  }

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

/* Allocate sector and clear it.
   Return true if successful, false on failure. */
static bool sector_free_map_clear_allocate(block_sector_t *sector)
{
  static char zeros[BLOCK_SECTOR_SIZE];

  if (!free_map_allocate(1, sector))
    return false;

  block_write(fs_device, *sector, zeros);
  return true;
}

/* Allocate sector for direct block and clear it.
   Return true if successful, false on failure. */
static bool direct_free_map_clear_allocate(struct inode_disk *disk_inode, block_sector_t *sectors)
{
  for (size_t i = 0; *sectors > 0 && i < DIRECT_BLOCKS; i++, (*sectors)--)
    if (!sector_free_map_clear_allocate(&disk_inode->data[i]))
      return false;
  return true;
}

/* Allocate sector for indirect block and clear it.
   Return true if successful, false on failure. */
static bool indirect_free_map_clear_allocate_aux(block_sector_t *indirect, block_sector_t *sectors)
{
  if (!free_map_allocate(1, indirect))
    return false;

  indirect_block_t *indirect_block = calloc(sizeof(indirect_block_t), 1);

  for (size_t i = 0; *sectors > 0 && i < INDIRECT_BLOCKS; i++, (*sectors)--)
    if (!free_map_allocate(1, &indirect_block->sectors[i]))
      return false;

  block_write(fs_device, *indirect, indirect_block);
  free(indirect_block);
  return true;
}

/* Allocate sector for indirect block of inode_disk and clear it.
   Return true if successful, false on failure. */
static bool indirect_free_map_clear_allocate(struct inode_disk *disk_inode, block_sector_t *sectors)
{
  return indirect_free_map_clear_allocate_aux(&disk_inode->indirect, sectors);
}

/* Allocate sector for double indirect block and clear it.
   Return true if successful, false on failure. */
static bool double_indirect_free_map_clear_allocate(struct inode_disk *disk_inode, block_sector_t *sectors)
{
  if (!free_map_allocate(1, &disk_inode->double_indirect))
    return false;

  indirect_block_t *double_indirect_block = calloc(sizeof(indirect_block_t), 1);

  for (size_t i = 0; i < DOUBLE_INDIRECT_BLOCKS / INDIRECT_BLOCKS; i++)
  {
    if (!indirect_free_map_clear_allocate_aux(&double_indirect_block->sectors[i], sectors))
      return false;
    if (*sectors == 0)
      break;
  }

  block_write(fs_device, disk_inode->double_indirect, double_indirect_block);
  free(double_indirect_block);
  return true;
}

typedef bool (*free_map_clear_allocate_func)(struct inode_disk *disk_inode, block_sector_t *sectors);

/* Allocate sectors for the inode
   from the free map and store them in the inode.
   Returns true if successful, false if not enough
   space in the free map. */
static bool inode_free_map_clear_allocate(struct inode_disk *disk_inode, size_t sectors)
{
  ASSERT(disk_inode != NULL);

  static free_map_clear_allocate_func free_map_clear_allocate_funcs[] = {
      direct_free_map_clear_allocate,
      indirect_free_map_clear_allocate,
      double_indirect_free_map_clear_allocate};

  for (size_t i = 0; i < sizeof(free_map_clear_allocate_funcs) / sizeof(free_map_clear_allocate_func); i++)
  {
    if (!free_map_clear_allocate_funcs[i](disk_inode, &sectors))
      return false;
    if (sectors == 0)
      return true;
  }

  return false;
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool inode_create(block_sector_t sector, off_t length, inode_type_t type)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT(length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT(sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc(1, sizeof *disk_inode);
  if (disk_inode != NULL)
  {
    size_t sectors = bytes_to_sectors(length);
    disk_inode->length = length;
    disk_inode->magic = INODE_MAGIC;
    disk_inode->type = type;
    if (inode_free_map_clear_allocate(disk_inode, sectors))
    {
      block_write(fs_device, sector, disk_inode);
      success = true;
    }
    free(disk_inode);
  }
  return success;
}
/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open(block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin(&open_inodes); e != list_end(&open_inodes);
       e = list_next(e))
  {
    inode = list_entry(e, struct inode, elem);
    if (inode->sector == sector)
    {
      inode_reopen(inode);
      return inode;
    }
  }

  /* Allocate memory. */
  inode = malloc(sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front(&open_inodes, &inode->elem);
  lock_init(&inode->operation_lock);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  block_read(fs_device, inode->sector, &inode->data);
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

/* Deallocate indirect blocks */
static void indirect_free_map_deallocate(block_sector_t indirect)
{
  if (indirect == 0)
    return;

  indirect_block_t *indirect_block = calloc(sizeof(indirect_block_t), 1);
  block_read(fs_device, indirect, indirect_block);
  for (size_t i = 0; i < INDIRECT_BLOCKS; i++)
    if (indirect_block->sectors[i] != 0)
      free_map_release(indirect_block->sectors[i], 1);
  free(indirect_block);
  free_map_release(indirect, 1);
}

/* Deallocate INODE data. */
static void inode_free_map_deallocate(struct inode *inode)
{
  // Direct blocks
  for (size_t i = 0; i < DIRECT_BLOCKS; i++)
    if (inode->data.data[i] != 0)
      free_map_release(inode->data.data[i], 1);

  // Indirect blocks
  indirect_free_map_deallocate(inode->data.indirect);

  // Double indirect blocks
  if (inode->data.double_indirect != 0)
  {
    indirect_block_t *double_indirect_block = calloc(sizeof(indirect_block_t), 1);
    block_read(fs_device, inode->data.double_indirect, double_indirect_block);
    for (size_t i = 0; i < DOUBLE_INDIRECT_BLOCKS / INDIRECT_BLOCKS; i++)
      indirect_free_map_deallocate(double_indirect_block->sectors[i]);
    free(double_indirect_block);
    free_map_release(inode->data.double_indirect, 1);
  }
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
      inode_free_map_deallocate(inode);
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
  lock_acquire(&inode->operation_lock);
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

  lock_release(&inode->operation_lock);
  return bytes_read;
}

/* Extend the file size to SIZE bytes. */
static bool inode_extend(struct inode *inode, off_t size)
{
  inode_free_map_deallocate(inode);
  
  if (!inode_free_map_clear_allocate(&inode->data, size))
    return false;

  inode->data.length = size;
  block_write(fs_device, inode->sector, &inode->data);
  return true;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t inode_write_at(struct inode *inode, const void *buffer_, off_t size, off_t offset)
{
  lock_acquire(&inode->operation_lock);
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
  {
    lock_release(&inode->operation_lock);
    return 0;
  }

  if (offset + size > inode_length(inode))
  {
    if (!inode_extend(inode, offset + size))
    {
      lock_release(&inode->operation_lock);
      return 0;
    }
  }

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

  lock_release(&inode->operation_lock);
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

bool inode_is_dir(struct inode *inode)
{
  return inode->data.type==INODE_TYPE_DIRECTORY;
}

int inode_sector(struct inode *inode)
{
  return (int)inode->sector;
}
