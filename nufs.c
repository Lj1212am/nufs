// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "inode.h"
#include "storage.h"
#include "directory.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
    int inum = tree_lookup(path);

    if (inum == -1) {
        return -1;
    }

    inode_t *node = get_inode(inum);
  
    printf("access(%s, %04o) -> \n", path, mask);
  return 0;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = storage_stat(path, st);

  if (rv < 0) {
    return -ENOENT;
  }
  printf("getting attr\n");
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  
  char item_path[128];
  struct dirent* dir_entry;
  (void) offset;
  (void) fi;
  int rv = 0;

  slist_t *items = storage_list(path);
  
  

  for (slist_t *xs = items; xs != 0; xs = xs->next) {
    struct stat st;
    printf("current item: %s\n", xs->data);
    strcpy(item_path, path);
    strcat(item_path, "/");
    strcat(item_path, xs->data);
    
    rv = storage_stat(item_path, &st);
    assert(rv == 0);
    filler(buf, xs->data, &st, 0);
    memset(item_path, 0, sizeof(char[128])); 
  }

  //filler(buf, "hello.txt", &st, 0);

  printf("readdir(%s) -> %d\n", path, rv);

  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  printf("making object\n");
  int rv = storage_mknod(path, mode);


  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);
  char *from_parent = malloc(strlen(path) + 1);
  get_parent_dir(path, from_parent);
  int parent_inum = tree_lookup(from_parent);
 
  if (rv >= 0) {
    directory_put(node, ".", parent_inum);
    directory_put(node, "..", inum);
  }
  printf("making dir\n");
 
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

//unlinks file from this path
int nufs_unlink(const char *path) {
  int rv = storage_unlink(path);
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

// links the files from the to paths
int nufs_link(const char *from, const char *to) {
  int rv = storage_link(from, to);
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// removes the directory from that path
int nufs_rmdir(const char *path) {
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  int mode = node->mode;

  if (mode != 040755) {
      printf("rmdir(%s) -> %d\n", path, -1);
      return -1;
  }

  return nufs_unlink(path);
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = storage_rename(from, to);
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// changes permissions
int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  if (node->mode == mode) {
      return 0;
  } else {
      shrink_inode(node, 0);
      node->mode = mode;
      rv = 0;
  }
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// truncates file/dir by the passed in size
int nufs_truncate(const char *path, off_t size) {
  int rv = storage_truncate(path, size);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = nufs_access(path, 0);
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = storage_read(path, buf, size, offset);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);
  time_t time = ts->tv_sec;
  node->modified_time = time;
  if (node == NULL) {
    return -1;
  }
  printf("utimens(%s, [%ld, %ld; %ld, %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, 1);
  return 0;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = 0;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

// links fuse operations to nufs
void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);


  //initalize blocks
  storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
