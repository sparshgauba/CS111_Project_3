#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

typedef struct superblock{
  uint32_t inodes_count;
  uint32_t blocks_count;
  char buf0[16];
  uint32_t s_log_block_size;
  char buf1[4];
  uint32_t s_blocks_per_group;
  char buf2[4];
  uint32_t s_inodes_per_group;
  char buf3[40];
  uint32_t s_first_inode;
  uint16_t s_inode_size;
  char buf4[934];  
} superblock_t;

typedef struct block_descriptor_table{
	uint32_t bg_block_bitmap;
	char buf0[1020];
} block_group_descriptor_t;

block_group_descriptor_t groupdescriptor_ptr;
superblock_t *superblock_ptr;
void exit_1(char *str)
{
  fprintf(stderr, "Error: ");
  if(strlen(str) != 0)
    fprintf(stderr, "%s\n", str);
  if(errno != 0)
    perror("");
  fprintf(stderr, "Exiting with error code 1.\n");
  exit(1);
  
   
}

void block_output()
{
  uint32_t blocksize = 1024 << superblock_ptr->s_log_block_size;

  
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock_ptr->blocks_count,
	   superblock_ptr->inodes_count, blocksize, superblock_ptr->s_inode_size,
	   superblock_ptr->s_blocks_per_group, superblock_ptr->s_inodes_per_group,
	   superblock_ptr->s_first_inode);
 
}

void group_output()
{
  
}

int main(int argc, char **argv)
{

  char *file = argv[1];
  int fd = open(file, O_RDONLY, 0444);
  if(fd == -1)
    exit_1("");
  
  uint8_t superblock_read[1024];
  if(!pread(fd, superblock_read, 1024, 1024))
    exit_1("");
  superblock_ptr = (superblock_t*) superblock_read;
  block_output();

  uint8_t blockgroup_read[1024];
  if(!pread(fd, blockgroup_read, 1024, 2048);

  /*Get where the block bitmap is from the block descriptor table
  	block group descriptor is block 2*/
	


  

  exit(0);
       
  
}
