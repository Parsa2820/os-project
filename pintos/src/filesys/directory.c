#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/thread.h"

/* A directory. */
struct dir
{
  struct inode *inode;  /* Backing store. */
  off_t pos;            /* Current position. */
  struct lock dir_lock; /* lock on directory */
};

/* A single directory entry. */
struct dir_entry
{
  block_sector_t inode_sector; /* Sector number of header. */
  char name[NAME_MAX + 1];     /* Null terminated file name. */
  bool in_use;                 /* In use or free? */
};

/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool dir_create(block_sector_t sector, size_t entry_cnt)
{
  // return inode_create(sector, entry_cnt * sizeof(struct dir_entry), INODE_TYPE_DIRECTORY);
  bool operation_result = inode_create(sector, entry_cnt * sizeof(struct dir_entry), INODE_TYPE_DIRECTORY);
  if (operation_result)
  {
    struct dir *cur_dir = dir_open(inode_open(sector));
    struct dir_entry entry;
    entry.inode_sector = sector;
    entry.in_use = false;

    /* Acquire dir lock */
    dir_acquire_lock(cur_dir);
    if (inode_write_at(dir_get_inode(cur_dir), &entry, sizeof(entry), 0) != sizeof(entry))
      operation_result = false;

    /* Release dir lock */
    dir_release_lock(cur_dir);
    dir_close(cur_dir);
  }
  return operation_result;
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open(struct inode *inode)
{
  struct dir *dir = calloc(1, sizeof *dir);
  if (inode && dir)
  {
    lock_init(&dir->dir_lock);
    dir->inode = inode;
    dir->pos = 0;
    return dir;
  }
  else
  {
    inode_close(inode);
    free(dir);
    return NULL;
  }
}

/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir *
dir_open_root(void)
{
  return dir_open(inode_open(ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir *
dir_reopen(struct dir *dir)
{
  return dir_open(inode_reopen(dir->inode));
}

/* Destroys DIR and frees associated resources. */
void dir_close(struct dir *dir)
{
  if (dir != NULL)
  {
    inode_close(dir->inode);
    free(dir);
  }
}

/* Returns the inode encapsulated by DIR. */
struct inode *
dir_get_inode(struct dir *dir)
{
  return dir->inode;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool
lookup(const struct dir *dir, const char *name,
       struct dir_entry *ep, off_t *ofsp)
{
  struct dir_entry e;
  size_t ofs;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  for (ofs = 0; inode_read_at(dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e)
    if (e.in_use && !strcmp(name, e.name))
    {
      if (ep != NULL)
        *ep = e;
      if (ofsp != NULL)
        *ofsp = ofs;
      return true;
    }
  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool dir_lookup(const struct dir *dir, const char *name,
                struct inode **inode)
{
  struct dir_entry e;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  if (lookup(dir, name, &e, NULL))
    *inode = inode_open(e.inode_sector);
  else if (strcmp(name, ".") == 0)
  {
    *inode = inode_reopen(dir->inode);
  }
  else if (strcmp(name, "..") == 0)
  {
    inode_read_at(dir->inode, &e, sizeof e, 0);
    *inode = inode_open(e.inode_sector);
  }
  else
    *inode = NULL;

  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool dir_add(struct dir *dir, const char *name, block_sector_t inode_sector, inode_type_t type)
{
  struct dir_entry e;
  off_t ofs;
  bool success = false;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  dir_acquire_lock(dir);

  // is name valid or not; must not exceed size limit nor be NULL
  if (*name == '\0' || strlen(name) > NAME_MAX)
    return false;

  // check uniqueness
  if (lookup(dir, name, NULL, NULL))
    goto done;

  if (type == INODE_TYPE_DIRECTORY)
  {
    struct dir *current_dir = dir_open(inode_open(inode_sector));
    if (current_dir == NULL)
      goto done;

    // Acquire the lock
    dir_acquire_lock(current_dir);

    bool written = true;
    struct dir_entry e_prime;
    e_prime.in_use = false;
    e_prime.inode_sector = inode_get_inumber(dir_get_inode(dir));
    if (inode_write_at(current_dir->inode, &e_prime, sizeof e_prime, 0) != sizeof e_prime)
      written = false;

    // Release the lock
    dir_release_lock(current_dir);
    dir_close(current_dir);

    if (!written)
      goto done;
  }

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.

     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */
  for (ofs = 0; inode_read_at(dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e)
    if (!e.in_use)
      break;

  /* Write slot. */
  e.in_use = true;
  strlcpy(e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  success = inode_write_at(dir->inode, &e, sizeof e, ofs) == sizeof e;

// release directory lock and declare success
done:
  dir_release_lock(dir);
  return success;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool dir_remove(struct dir *dir, const char *name)
{
  struct dir_entry e;
  struct inode *inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  /* Find directory entry. */
  if (!lookup(dir, name, &e, &ofs))
    goto done;

  /* Open inode. */
  inode = inode_open(e.inode_sector);
  if (inode == NULL)
    goto done;

  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at(dir->inode, &e, sizeof e, ofs) != sizeof e)
    goto done;

  /* Remove inode. */
  inode_remove(inode);
  success = true;

done:
  inode_close(inode);
  return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool dir_readdir(struct dir *dir, char name[NAME_MAX + 1])
{
  struct dir_entry e;

  while (inode_read_at(dir->inode, &e, sizeof e, dir->pos) == sizeof e)
  {
    dir->pos += sizeof e;
    if (e.in_use)
    {
      strlcpy(name, e.name, NAME_MAX + 1);
      return true;
    }
  }
  return false;
}

static int
next_section(char section[NAME_MAX + 1], const char **srcptr)
{
  const char *src = *srcptr;
  int src_ofs = 0;

  while (src_ofs < NAME_MAX + 1 && *(src + src_ofs) == '/')
    src_ofs++;
  if (!*(src + src_ofs))
    return 0;

  src += src_ofs;

  if (src_ofs >= NAME_MAX)
    return -1;

  char *destination = section;
  while (*src)
  {
    if (*src == '/')
      break;
    if (destination < section + NAME_MAX)
    {
      *destination = *src;
      destination++;
      src++;
    }
    else
      return -1;
  }

  *destination = '\0';
  *srcptr = src;
  return 1;
}

bool separate_path_and_filename(const char *path, char *dir, char *file)
{
  if (path[0] == '\0')
    return false;

  int status;
  char token[NAME_MAX + 1] = "";
  char prev_token[NAME_MAX + 1] = "";

  if (path[0] == '/')
    *(dir++) = path[0];

  while (status = next_section(token, &path))
  {
    if (status == -1)
      return false;

    size_t prev_length = strlen(prev_token);

    if (prev_length > 0)
    {
      memcpy(dir, prev_token, sizeof(char) * prev_length);
      dir[prev_length] = '/';
      dir += prev_length + 1;
    }

    memcpy(prev_token, token, sizeof(char) * strlen(token));
    prev_token[strlen(token)] = '\0';
  }

  *dir = '\0';
  memcpy(file, token, sizeof(char) * (strlen(token) + 1));
  return true;
}

bool is_absolute_path(const char *path)
{
  return path[0] == '/';
}

struct dir *
dir_open_by_path(char *name)
{
  struct thread *cur_thread = thread_current();
  struct dir *cur_dir = !cur_thread->current_dir || is_absolute_path(name) ? dir_open_root() : dir_reopen(cur_thread->current_dir);
  char dir_token[NAME_MAX + 1];

  for (struct dir *next_dir; next_section(dir_token, &name); cur_dir = next_dir)
  {
    struct inode *next_inode;

    if ((!dir_lookup(cur_dir, dir_token, &next_inode)) || (!(next_dir = dir_open(next_inode))))
    {
      dir_close(cur_dir);
      return NULL;
    }

    dir_close(cur_dir);
  }

  if (!dir_get_inode(cur_dir)->removed)
    return cur_dir;

  dir_close(cur_dir);
  return NULL;
}

void dir_release_lock(struct dir *dir)
{
  lock_release(&(dir->dir_lock));
}

void dir_acquire_lock(struct dir *dir)
{
  lock_acquire(&(dir->dir_lock));
}
