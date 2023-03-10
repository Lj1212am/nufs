// based on cs3650 starter code

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

#define  BLOCK_COUNT 256
// we split the "disk" into 256 blocks
#define BLOCK_SIZE  4096
// = 4K
#define NUFS_SIZE  BLOCK_SIZE * BLOCK_COUNT
// = 1MB

#define BLOCK_BITMAP_SIZE  BLOCK_COUNT / 8
// Note: assumes block count is divisible by 8

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes);

// Load and initialize the given disk image.
void blocks_init(const char *image_path);

// Close the disk image.
void blocks_free();

// Get the block with the given index, returning a pointer to its start.
void *blocks_get_block(int bnum);

// Return a pointer to the beginning of the block bitmap.
void *get_blocks_bitmap();

// Return a pointer to the beginning of the inode table bitmap.
void *get_inode_bitmap();

// Allocate a new block and return its index.
int alloc_block();

// Deallocate the block with the given index.
void free_block(int bnum);

#endif
