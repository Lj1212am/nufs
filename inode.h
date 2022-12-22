// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"
#include "bitmap.h"
#include <time.h>

typedef struct inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)
  time_t access_time;
  time_t modified_time;
} inode_t;

// pretty print inode
void print_inode(inode_t *node);

// get inode at given inum
inode_t *get_inode(int inum);

// allocates next free inode, return inum of allocated inode
int alloc_inode();

// free inode at inum
void free_inode(int inum);

// grow inode by size
int grow_inode(inode_t *node, int size);

// shrink inode by size
int shrink_inode(inode_t *node, int size);

#endif
