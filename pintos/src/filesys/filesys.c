#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/directory.h"
#include "threads/synch.h"
/* Partition that contains the file system. */
struct block *fs_device;

static void do_format(void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void filesys_init(bool format)
{
  fs_device = block_get_role(BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC("No file system device found, can't initialize file system.");

  inode_init();
  init_cache();
  free_map_init();

  if (format)
    do_format();

  free_map_open();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void filesys_done(void)
{
  free_map_close();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool filesys_create(const char *name, off_t initial_size, inode_type_t type)
{
  block_sector_t inode_sector = 0;

  char dirname[strlen (name) + 1];
  char filename[strlen (name) + 1];
  dirname[0] = '\0';
  filename[0] = '\0';
  bool path_separated = separate_path_and_filename (name, dirname, filename);
  struct dir *dir = dir_open_by_path(dirname);
  bool success = (path_separated && dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, type)
                  && dir_add (dir, filename, inode_sector, type));
  if (!success && inode_sector != 0)
    free_map_release(inode_sector, 1);
  dir_close(dir);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  char dirname[strlen (name) + 1];
  char filename[NAME_MAX + 1];
  dirname[0] = '\0';
  filename[0] = '\0';

  bool path_separated = separate_path_and_filename (name, dirname, filename);
  struct dir *dir = dir_open_by_path (dirname);

  struct inode *inode = NULL;
  if (dir == NULL || !path_separated)
    return NULL;

  if (strlen (filename) == 0)
    inode = dir_get_inode (dir);
  else
    {
      dir_lookup (dir, filename, &inode);
      dir_close (dir);
    }

  if (inode == NULL || inode->removed)
    return NULL;

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool filesys_remove(const char *name)
{
  struct dir *dir = dir_open_root();
  bool success = dir != NULL && dir_remove(dir, name);
  dir_close(dir);

  return success;
}

/* Formats the file system. */
static void
do_format(void)
{
  printf("Formatting file system...");
  free_map_create();
  if (!dir_create(ROOT_DIR_SECTOR, 16))
    PANIC("root directory creation failed");
  free_map_close();
  printf("done.\n");
}
