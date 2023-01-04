// NAME: Muthu Palaniappan, Amy Seo
// EMAIL: muthupal@g.ucla.edu, amyseo@ucla.edu
// ID: 005292734, 505328863

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "ext2_fs.h"

int fd = 0;
unsigned int inodes_count = 0;
unsigned int blocks_count = 0;
unsigned int block_size = 0;
unsigned int i_node_size;
unsigned int blocks_per_group;
unsigned int i_nodes_per_group;
unsigned int first_nr_i_node;

char* translateTime(unsigned int time) {
    time_t originalTime = time;
    struct tm* timeData = gmtime(&originalTime);
    char* timeBuff = malloc(32 * sizeof(char));
    strftime(timeBuff, 32, "%m/%d/%y %H:%M:%S", timeData);
    return timeBuff;
}

void printSuper(struct ext2_super_block super){
    if (pread(fd, &super, sizeof(super), EXT2_MIN_BLOCK_SIZE) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    inodes_count = super.s_inodes_count;
    blocks_count = super.s_blocks_count;
    block_size = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;
    i_node_size = super.s_inode_size;
    blocks_per_group = super.s_blocks_per_group;
    i_nodes_per_group = super.s_inodes_per_group;
    first_nr_i_node = super.s_first_ino;
    
    // SUPERBLOCK
    // total number of blocks (decimal)
    // total number of i-nodes (decimal)
    // block size (in bytes, decimal)
    // i-node size (in bytes, decimal)
    // blocks per group (decimal)
    // i-nodes per group (decimal)
    // first non-reserved i-node (decimal)

    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", 
        blocks_count, inodes_count, block_size, i_node_size, blocks_per_group, i_nodes_per_group, first_nr_i_node);

}

void printGroup(struct ext2_group_desc groupDescriptor){
    if (pread(fd, &groupDescriptor, sizeof(groupDescriptor), EXT2_MIN_BLOCK_SIZE+sizeof(struct ext2_super_block)) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    // GROUP
    // group number (decimal, starting from zero)
    // total number of blocks in this group (decimal)
    // total number of i-nodes in this group (decimal)
    // number of free blocks (decimal)
    // number of free i-nodes (decimal)
    // block number of free block bitmap for this group (decimal)
    // block number of free i-node bitmap for this group (decimal)
    // block number of first block of i-nodes in this group (decimal)
    unsigned int groupNum = 0;
    unsigned int numBlocks = blocks_count;
    unsigned int numINodes = i_nodes_per_group;
    unsigned int numFreeBlocks = groupDescriptor.bg_free_blocks_count;
    unsigned int numFreeINodes = groupDescriptor.bg_free_inodes_count;
    unsigned int freeBlockBitmap = groupDescriptor.bg_block_bitmap;
    unsigned int freeINodeBitmap = groupDescriptor.bg_inode_bitmap;
    unsigned int firstBlockINodes = groupDescriptor.bg_inode_table;

    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", 
        groupNum, numBlocks, numINodes, numFreeBlocks, numFreeINodes, freeBlockBitmap, freeINodeBitmap, firstBlockINodes);
}

void scanFreeBlock(struct ext2_group_desc groupDescriptor){
    if (pread(fd, &groupDescriptor, sizeof(groupDescriptor), EXT2_MIN_BLOCK_SIZE+sizeof(struct ext2_super_block)) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    // BFREE
    // number of the free block (decimal)
    unsigned char *bitmap;
    bitmap = malloc(block_size);
    if (pread(fd, bitmap, block_size, groupDescriptor.bg_block_bitmap * block_size) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    for(unsigned int i = 0; i < block_size; i++){
        for (int j = 0; j < 8; j++){
            if ((bitmap[i] & (1 << j)) == 0){
                int blockCurr = 8*i + j + 1;
                printf("BFREE,%d\n", blockCurr);
            }
        }
    }
    free(bitmap);
}

void scanFreeInodes(struct ext2_group_desc groupDescriptor){
    if (pread(fd, &groupDescriptor, sizeof(groupDescriptor), EXT2_MIN_BLOCK_SIZE+sizeof(struct ext2_super_block)) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    // IFREE
    // number of the free I-node (decimal)
    unsigned char *iNodeBitmap;
    iNodeBitmap = malloc(block_size);
    if (pread(fd, iNodeBitmap, block_size, groupDescriptor.bg_inode_bitmap * block_size) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    for(unsigned int i = 0; i < block_size; i++){
        for (int j = 0; j < 8; j++){
            if ((iNodeBitmap[i] & (1 << j)) == 0){
                int iNodeFree = 8*i + j + 1;
                printf("IFREE,%d\n", iNodeFree);
            }
        }
    }
    free(iNodeBitmap);
}

void scanIndirectBlock(__u32* block, int level, int offset, int itr) {
    // INDIRECT
    // I-node number of the owning file (decimal)
    // (decimal) level of indirection for the block being scanned ... 1 for single indirect, 2 for double indirect, 3 for triple
    // logical block offset (decimal) represented by the referenced block. If the referenced block is a data block, this is the logical block offset of that block within the file. If the referenced block is a single- or double-indirect block, this is the same as the logical offset of the first data block to which it refers.
    // block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), but the lower level block that contains the block reference reported by this entry.
    // block number of the referenced block (decimal)
    if (level == 0 || *block == 0) {
        return;
    }
    __u32 *tmp_block = malloc(sizeof(__u32));
    unsigned int num_entries = EXT2_MIN_BLOCK_SIZE / sizeof(__u32);
    for (unsigned int i = 0; i < num_entries; i++) {
        if (pread(fd, tmp_block, sizeof(__u32), EXT2_MIN_BLOCK_SIZE * (*block) + i * sizeof(__u32)) == -1) {
            fprintf(stderr, "Error in pread.\n");
            exit(2);
        }
        if (*tmp_block != 0) {
            printf("INDIRECT,%d,%d,%d,%d,%d\n", itr+1, level, offset+i, *block, *tmp_block);
            scanIndirectBlock(tmp_block, level-1, offset, itr);
        }
    }
}

void scanInode(struct ext2_group_desc groupDescriptor) {
    if (pread(fd, &groupDescriptor, sizeof(groupDescriptor), EXT2_MIN_BLOCK_SIZE+sizeof(struct ext2_super_block)) == -1) {
        fprintf(stderr, "Error in pread.\n");
        exit(2);
    }
    struct ext2_inode inode;
    for (unsigned int i = 0; i < i_nodes_per_group; i++) {
        if (pread(fd, &inode, sizeof(struct ext2_inode), groupDescriptor.bg_inode_table * block_size + i * sizeof(struct ext2_inode)) == -1) {
            fprintf(stderr, "Error in pread.\n");
            exit(2);
        }
        if (inode.i_mode == 0 || inode.i_links_count == 0) continue;
        char ftype;
        if (S_ISDIR(inode.i_mode)) {
            ftype = 'd';
        }
        else if(S_ISREG(inode.i_mode)) {
            ftype = 'f';
        }
        else if(S_ISLNK(inode.i_mode)) {
            ftype = 's';
        }
        else {
            ftype = '?';
        }
        // INODE
        // inode number (decimal)
        // file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
        // mode (low order 12-bits, octal ... suggested format "%o")
        // owner (decimal)
        // group (decimal)
        // link count (decimal)
        // time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
        // modification time (mm/dd/yy hh:mm:ss, GMT)
        // time of last access (mm/dd/yy hh:mm:ss, GMT)
        // file size (decimal)
        // number of (512 byte) blocks of disk space (decimal) taken up by this file
        char *creationTime = translateTime(inode.i_ctime);
        char *modTime = translateTime(inode.i_mtime);
        char *accessTime = translateTime(inode.i_atime);
        printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",i+1,ftype,inode.i_mode & 0xFFF,inode.i_uid,inode.i_gid,
            inode.i_links_count,creationTime,modTime,accessTime,inode.i_size,inode.i_blocks);
        
        if (ftype == 'f' || ftype == 'd' || (ftype == 's' && inode.i_size >= 60)) {
            printf("%c", ',');
            for(unsigned int i = 0; i < 15; i++) {
                printf("%d", inode.i_block[i]);
                if (i != 14) {
                    printf("%c", ',');
                }
            }
        }
        printf("\n");
        // DIRENT
        // parent inode number (decimal) ... the I-node number of the directory that contains this entry
        // logical byte offset (decimal) of this entry within the directory
        // inode number of the referenced file (decimal)
        // entry length (decimal)
        // name length (decimal)
        // name (string, surrounded by single-quotes). Don't worry about escaping, we promise there will be no single-quotes or commas in any of the file names.
        if (ftype == 'd'){
            for(int k = 0; k < EXT2_NDIR_BLOCKS; k++){
                if (inode.i_block[k] == 0) continue;
                unsigned int iter = 0;
                struct ext2_dir_entry entry;
                while(iter < block_size) {
                    if (pread(fd, &entry, sizeof(struct ext2_dir_entry), inode.i_block[k]*EXT2_MIN_BLOCK_SIZE + iter) == -1) {
                        fprintf(stderr, "Error in pread.\n");
                        exit(2);
                    }
                    if (entry.inode == 0){
                        iter += entry.rec_len;
                        continue;
                    }
                    char file_name[EXT2_NAME_LEN+1];
                    memcpy(file_name, entry.name, entry.name_len);
                    file_name[entry.name_len] = '\0'; /* append null char to the file name */
                    printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", i+1, iter, entry.inode, entry.rec_len, entry.name_len, file_name);
                    iter += entry.rec_len;
                }
            }
            
        }
        if (ftype == 'd' || ftype == 'f') {
            int offset = 12;
            scanIndirectBlock(&inode.i_block[EXT2_IND_BLOCK], 1, offset, i);
            offset += EXT2_MIN_BLOCK_SIZE / sizeof(__u32);
            scanIndirectBlock(&inode.i_block[EXT2_DIND_BLOCK], 2, offset, i);
            offset += (EXT2_MIN_BLOCK_SIZE / sizeof(__u32) * EXT2_MIN_BLOCK_SIZE / sizeof(__u32));
            scanIndirectBlock(&inode.i_block[EXT2_TIND_BLOCK], 3, offset, i);
        }
    }
}

int main (int argc, char **argv)
{
    struct ext2_super_block super;
    struct ext2_group_desc groupDescriptor; 

    if (argc < 2){
        fprintf(stderr, "No file name was given in command line. Example usage: ./lab3a <filename>\n");
        exit(1);
    }
    else if ((fd = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Error opening file.\n");
        exit(1);
    }
    printSuper(super);
    printGroup(groupDescriptor);
    scanFreeBlock(groupDescriptor);
    scanFreeInodes(groupDescriptor);
    scanInode(groupDescriptor);
    exit(0);
}