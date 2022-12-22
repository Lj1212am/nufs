#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "inode.h"

#define SIZE 64

// pretty print inode
void print_inode(inode_t *node) {

	printf("inode: refs: %d, mode: %d, size: %d, block: %d\n", node->refs, 
			node->mode, node->size, node->block);
}

// get inode at given inum
inode_t *get_inode(int inum) {

	// make sure inum is within range
	assert(inum < SIZE);
	printf("getting inode %d\n", inum);
	inode_t* inode_ptr = (inode_t*) blocks_get_block(1);
	return inode_ptr + inum;
}

// allocates next free inode, return inum of allocated inode
int alloc_inode() {

	// get bitmap
	void *bm = get_inode_bitmap();

	printf("trying to allocate\n");

	// find next free space in the inode bitmap
	for(int i = 0; i < SIZE; i++) {
		// if inode is free in the bitmap
		if (bitmap_get(bm, i) == 0) {
			// get free inode
			inode_t* node = get_inode(i);

			// allocate memory and fields
			memset(node, 0, sizeof(inode_t));
			node->refs = 0;
			node->mode = 010644;
			node->size = 0;
			node->block = alloc_block();
			node->access_time = time(NULL);
			node->modified_time = time(NULL);
			bitmap_put(bm, i, 1);

			// return inum i
			printf("allocating inode at %d\n", i);
			return i;
		}
		// else continue
	}
	// no free inode is found
	return -1;
}

// free inode at inum
void free_inode(int inum) {

	// get bitmap and mark inum as free in bitmap
	void *bm = get_inode_bitmap();
	bitmap_put(bm, inum, 0);

	// get inode at inum
	inode_t* node = get_inode(inum);

	assert(node->refs == 0);
	printf("freeing inode at %d\n", inum);

	// shrink inode and free block, then set the memory at
	// node to 0s
	shrink_inode(node, node->size);


	free_block(node->block);
	memset(node, 0, sizeof(inode_t));
}

// grow inode by size
int grow_inode(inode_t *node, int size) {

	int current_size = node->size;
	printf("growing inode from %d to %d\n", current_size, current_size + size);
	node->size = current_size + size;

	return 0;
}

// shrink inode by size
int shrink_inode(inode_t *node, int size) {

	int current_size = node->size;
	printf("shrinking inode from %d to %d\n", current_size, current_size - size);
	node->size = current_size - size;
	return 0;

}

