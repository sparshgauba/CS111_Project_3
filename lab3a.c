#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

#define BLOCKSIZE 1024

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
  uint32_t bg_inode_bitmap;
  uint32_t bg_inode_table;
  char buf0[1012];
} block_group_descriptor_t;

/*An inode is only 128 bytes*/
typedef struct inode{
  uint16_t i_mode;
  uint16_t i_uid;
  uint32_t i_size;
  uint32_t i_atime;
  uint32_t i_ctime;
  uint32_t i_mtime;
  uint32_t i_dtime;
  uint16_t i_gid;
  uint16_t i_links_count;
  uint32_t i_blocks;
  uint32_t i_flags;
  uint32_t i_ods1;
  uint32_t i_block[15];
  uint32_t i_generation;
  uint32_t i_file_acl;
  uint32_t i_dir_acl;
  uint32_t i_faddr;  
  char buf0[12];
} inode_t;

/****************************/
/*Block pointer declarations*/
/***************************/
block_group_descriptor_t *groupdescriptor_ptr;
superblock_t *superblock_ptr;
/********************************/
/*End block pointer declarations*/
/*******************************/

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

void superblock_output()
{
  uint32_t blocksize = BLOCKSIZE << superblock_ptr->s_log_block_size;

  
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock_ptr->blocks_count,
	   superblock_ptr->inodes_count, blocksize, superblock_ptr->s_inode_size,
	   superblock_ptr->s_blocks_per_group, superblock_ptr->s_inodes_per_group,
	   superblock_ptr->s_first_inode);
 
}

void group_output()
{
  return;
}

int is_bit_set(char byte, int index)
{
  switch(index)
    {
    case 0: return (byte & 0x80) >> 7;
    case 1: return (byte & 0x40) >> 6;
    case 2: return (byte & 0x20) >> 5;
    case 3: return (byte & 0x10) >> 4;
    case 4: return (byte & 0x8) >> 3;
    case 5: return (byte & 0x4) >> 2;
    case 6: return (byte & 0x2) >> 1;
    case 7: return (byte & 0x1);
    }
  return -1;
}

void superblock_info(int fd)
{
  uint8_t superblock_read[BLOCKSIZE];
  if(pread(fd, superblock_read, BLOCKSIZE, 1024) == -1)
    exit_1("");
  superblock_ptr = (superblock_t*) superblock_read;
  superblock_output();
}

void free_bits(char map_read[], int num_iterations, int block_flag)
{
  int i;
  for(i = 0; i < num_iterations; i++)
    {
          /*Cycle through each of the bits in a given byte*/
    int j;
    for(j = 0; j < 8; j++)
      {
  	int bit = is_bit_set(map_read[i],j);
  	if(! bit)
  	  {
  	    int index_free = i * 8 + j;
	    if(block_flag)
	      printf("BFREE,%d\n", index_free);
	    else
	      printf("IFREE,%d\n", index_free);
	    
  	  }
  	else if(bit == -1)
  	  exit_1("Error with bit-map.");
	       
      }

    }
}

int main(int argc, char **argv)
{

  char *file = argv[1];
  int fd = open(file, O_RDONLY, 0444);
  if(fd == -1)
    exit_1("");
  
  superblock_info(fd);

  /*Get where the block bitmap is from the block descriptor table
     block group descriptor is block 2*/
	

  uint8_t blockgroup_read[BLOCKSIZE];
  if(pread(fd, blockgroup_read, BLOCKSIZE, 2048) == -1)
    exit_1("");
  
  groupdescriptor_ptr = (block_group_descriptor_t*) blockgroup_read;

  /*Print the info about the block groups somewhere here?*/

  
  
  //TODO: IMPLEMENT FOR MORE THAN 1 BLOCK GROUP
  /*Get the block number where the bitmap is*/

  char blockmap[BLOCKSIZE];
  int blockmap_byte = groupdescriptor_ptr->bg_block_bitmap * BLOCKSIZE;
  if(pread(fd, blockmap, BLOCKSIZE, blockmap_byte) == -1)
    exit_1("");
  
  /*Account for variable group size*/
  //each byte is 8 bits
  int num_iterations = superblock_ptr->s_blocks_per_group / 8;
  int block_flag = 1;
  free_bits(blockmap, num_iterations, block_flag);

  
  /*Get the free inodes, pretty much the same implementation as free blocks*/
  char inodemap[BLOCKSIZE];
  int inodemap_byte = groupdescriptor_ptr->bg_inode_bitmap * BLOCKSIZE;
  if(pread(fd, inodemap, BLOCKSIZE, inodemap_byte) == -1)
    exit_1("");
  num_iterations = superblock_ptr->s_inodes_per_group / 8;
  block_flag = 0;
  free_bits(inodemap, num_iterations, block_flag);

  /*START INODE SUMMARY HERE*/
  char inode_table[BLOCKSIZE];
  int inode_table_byte = groupdescriptor_ptr->bg_inode_table * BLOCKSIZE;
  if(pread(fd, inode_table, BLOCKSIZE, inode_table_byte) == -1)
    exit_1("");
  
  /*Every 128 bytes contains an inode*/
  
  

  exit(0);
  
       
  
}
