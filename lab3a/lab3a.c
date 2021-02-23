/*
 NAME:	Bryan Tang, Zhengtong Liu
 EMAIL: tangtang1228@ucla.edu, ericliu2023@g.ucla.edu
 ID:    605318712, 505375562
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include "ext2_fs.h"

#define SB_OFFSET 1024
#define BLOCK_SIZE 1024

int fd = -1;
struct ext2_super_block sb;
struct ext2_inode inode;
struct ext2_group_desc* group;
struct ext2_dir_entry* dir_entry;

uint32_t blockSize;
uint32_t inodeSize;

int numOfGroups;

void pread_error();
void close_and_exit();
void superblock_summary();
void group_summary();
void get_time_GMT(time_t t, char* buffer);
unsigned long get_offset (int block);
char get_filetype (__u16 i_mode);
void directory_entries(unsigned int block_num, int inode_num);
void indirect_summary (unsigned int inode_num, unsigned int block_num, int l, int start_offset, char file_tyle);
void inode_summary (unsigned int offset, unsigned int inode_num);
void scan_free_block(int num, unsigned int block);
void scan_inode (int num, int block, int inode_table_index);

void pread_error()
{
    fprintf(stderr, "Error with pread: %s\n", strerror(errno));
    exit(1);
}

void close_and_exit()
{
    if (close(fd) < 0)
    {
        fprintf(stderr, "Error when closing the file: %s\n", strerror(errno));
        exit(2);
    }
}

void superblock_summary() {
    if(pread(fd, &sb, sizeof(struct ext2_super_block), SB_OFFSET) < 0){
        pread_error();
    }
    if (sb.s_magic != EXT2_SUPER_MAGIC)
    {
        fprintf(stderr, "Error with superblock by checking the s_magic field\n");
        exit(2);
    }
    blockSize = 1024 << sb.s_log_block_size;
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", 
		sb.s_blocks_count,      //number of blocks
		sb.s_inodes_count,      //number of inodes
		blockSize,	            //block size
		sb.s_inode_size,        //inode size
		sb.s_blocks_per_group,  //blocks per group
		sb.s_inodes_per_group,  //inodes per group
		sb.s_first_ino          //first non-reserved inode 
	);
}

void group_summary() {
    numOfGroups = sb.s_blocks_count / sb.s_blocks_per_group + 1;

    group = malloc(numOfGroups * sizeof(struct ext2_group_desc));

    for (int i = 0; i < numOfGroups; i++){
        if(pread(fd, &group[i], sizeof(struct ext2_group_desc), SB_OFFSET + blockSize + i * sizeof(struct ext2_group_desc)) < 0){
            pread_error();
        }
        // if the last group, will contain the remainder of the blocks
        int blocksPerGroup = (i == numOfGroups-1) ? (sb.s_blocks_count % sb.s_blocks_per_group) : sb.s_blocks_per_group;
        int inodesPerGroup = (i == numOfGroups-1 && i != 0) ? (sb.s_inodes_count % sb.s_inodes_per_group) : sb.s_inodes_per_group;
        fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
            i, //group number
            blocksPerGroup, //total number of blocks in this group
            inodesPerGroup, //total number of inodes in this group
            group[i].bg_free_blocks_count, //number of free blocks 
            group[i].bg_free_inodes_count, //number of free inodes
            group[i].bg_block_bitmap, //block number of free block bitmap for this group
            group[i].bg_inode_bitmap, //block number of free inode bitmap for this group
            group[i].bg_inode_table //block number of first block of i-nodes in this group
        );
        scan_free_block(i, group[i].bg_block_bitmap);
        scan_inode(i, group[i].bg_inode_bitmap, group[i].bg_inode_table);
    }

    free(group);
}

void get_time_GMT(time_t t, char* buffer)
{
    time_t to_convert = t;
    struct tm ts = *gmtime(&to_convert);
    strftime(buffer, 80, "%m/%d/%y %H:%M:%S", &ts);
}

unsigned long get_offset (int block)
{
    return (unsigned long) blockSize * (block - 1) + SB_OFFSET;
}


void scan_free_block(int num, unsigned int block)
{
    unsigned int index = sb.s_first_data_block + sb.s_blocks_per_group * num;
    char* bitmap = (char *) malloc(blockSize);
    if (pread(fd, bitmap, blockSize, get_offset(block)) < 0) {
        pread_error();
    }

    for (unsigned int j = 0; j < blockSize; j++)
    {
        char c = bitmap[j];
        for (int k = 0; k < 8; k++)
        {
            // note that 1 indicates the block is used
            // 0 indicates the block is free
            int bit = c & 1;
            if (!bit)
                fprintf(stdout, "BFREE,%d\n", index);
            // shift the c to the left to get the next bit
            c = c >> 1;
            index++;  
        }
    }
    free(bitmap);
}

char get_filetype (__u16 i_mode)
{
    char file_type;

    uint16_t file_descriptor = i_mode & 0xF000;
    // regular file
    if (file_descriptor == 0x8000)
        file_type = 'f';
    // directory
    else if (file_descriptor == 0x4000)
        file_type = 'd';
    // symbolic link
    else if (file_descriptor == 0xA000)
        file_type = 's';
    // unknown type
    else
        file_type = '?';
    return file_type;
}

void directory_entries(unsigned int block_num, int inode_num)
{
    dir_entry = malloc(sizeof(struct ext2_dir_entry));
    unsigned int iter = 0;

        if (block_num != 0)
        {
            while(iter < blockSize)
            {
                if(pread(fd, dir_entry, sizeof(struct ext2_dir_entry), get_offset(block_num) + iter) < 0)
                    pread_error();
                if(dir_entry -> inode != 0)
                {
                    char file_name[EXT2_NAME_LEN+1];
                    memcpy(file_name, dir_entry -> name, dir_entry -> name_len);
                    file_name[dir_entry -> name_len] = 0;
                    fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
                        inode_num,
                        iter,
                        dir_entry -> inode,
                        dir_entry -> rec_len,
                        dir_entry -> name_len,
                        file_name);
                }
                iter += dir_entry -> rec_len;
            }
        }


    free(dir_entry);
}

void indirect_summary (unsigned int inode_num, unsigned int block_num, int l, int start_offset, char file_tyle)
{
    uint32_t *block_pts = malloc(blockSize);
    unsigned long offset = get_offset(block_num);
    if(pread(fd, block_pts, blockSize, offset) < 0)
        pread_error();
    
    int i_block_num = blockSize/4;

    for (int k = 0; k < i_block_num; k++)
    {
        if (block_pts[k] != 0)
        {
            int logical_offset = start_offset;
            if (l == 1)
                logical_offset += k;
            else if (l == 2)
            {
                int second_start_offset = start_offset + i_block_num * k;
                indirect_summary(inode_num, block_pts[k], l-1, second_start_offset, file_tyle);
            }
            else
            {
                int third_start_offset = start_offset + i_block_num * i_block_num * k;
                indirect_summary(inode_num, block_pts[k], l-1, third_start_offset, file_tyle);
            }
            if (file_tyle == 'd' && l == 1)
                directory_entries(block_pts[k], inode_num);
            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
            inode_num,
            l,
            logical_offset,
            block_num,
            block_pts[k]);
        }
    }

    free(block_pts);
}


void inode_summary (unsigned int offset, unsigned int inode_num)
{

    if (pread(fd, &inode, sizeof(inode), offset) < 0)
        pread_error();
    
    char file_type = get_filetype(inode.i_mode);

    // check for non-zero mode and links count
    if ((!inode.i_mode) || (!inode.i_links_count))
        return;
    
    char creation_time[20];
    char modified_time[20];
    char access_time[20];
    get_time_GMT(inode.i_ctime, creation_time);
    get_time_GMT(inode.i_mtime, modified_time);
    get_time_GMT(inode.i_atime, access_time);

    fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
        inode_num, // the inode number
        file_type, // the file type
        inode.i_mode & 0xFFF, // the lower 12 bits of imode
        inode.i_uid, // owner uid
        inode.i_gid, // group id
        inode.i_links_count, // links count
        creation_time, // creation time
        modified_time, // modified time
        access_time, // access time
        inode.i_size, // file size
        inode.i_blocks // num of blocks
    );
    
    // symbolic link case not sure
    if (file_type == 's' && inode.i_size < 60)
        fprintf(stdout, ",%u", inode.i_block[0]);
    else
    {
        for (int i = 0; i < 15; i++)
            fprintf(stdout, ",%u", inode.i_block[i]);
        fprintf(stdout, "\n");

        //12 direct
        if (file_type == 'd')
        {
            for (int k = 0; k < 12; k++)
            {
                unsigned int block_num = inode.i_block[k];
                directory_entries(block_num, inode_num);
            }
        }
            
        // indirect
        if (inode.i_block[EXT2_IND_BLOCK] != 0) // 13th
            indirect_summary(inode_num, inode.i_block[EXT2_IND_BLOCK], 1, 12,file_type);
            
        if (inode.i_block[EXT2_DIND_BLOCK] != 0) // 14th
        {
            int start_offset = blockSize / 4 + 12;
            indirect_summary(inode_num, inode.i_block[EXT2_DIND_BLOCK], 2, start_offset,file_type);
        }
        
        if (inode.i_block[EXT2_TIND_BLOCK] != 0) // 15th
        {
            int start_offset = (blockSize / 4) * (blockSize / 4) + (blockSize / 4) + 12;
            indirect_summary(inode_num, inode.i_block[EXT2_TIND_BLOCK], 3, start_offset,file_type);
        }
    }
    

}


// num stands for the index of group
void scan_inode (int num, int block, int inode_table_index)
{
    unsigned int index = sb.s_first_data_block + sb.s_blocks_per_group * num;
    unsigned int start = index;
    // s.inode_per_group is bit in unit, convert to byte in unit
    char* bitmap = (char *) malloc(sb.s_inodes_per_group/8);

    if (pread(fd, bitmap, sb.s_inodes_per_group/8, get_offset(block)) < 0) {
        pread_error();
    }

    for (unsigned int j = 0; j < sb.s_inodes_per_group/8; j++)
    {
        char c = bitmap[j];
        for (int k = 0; k < 8; k++)
        {
            // note that 1 indicates the block is used
            // 0 indicates the block is free
            int bit = c & 1;
            if (!bit)
                fprintf(stdout, "IFREE,%d\n", index);
            else
            {
                unsigned int offset = get_offset(inode_table_index) + sizeof(inode) * (index - start);
                inode_summary(offset, index);
            }
            // shift the c to the left to get the next bit
            c = c >> 1;
            index++;  
        }
    }
    free(bitmap);
}



int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "%s\n", "Bad arguments");
        exit(1);
    }

    if((fd = open(argv[1], O_RDONLY)) < 0)
    {
        fprintf(stderr, "%s: %s\n", "Fail to mount disk image", strerror(errno));
        exit(2);
    }
    atexit(1);

    superblock_summary();
    group_summary();
}