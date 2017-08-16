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
  char buf0[1016];
} block_group_descriptor_t;


block_group_descriptor_t *groupdescriptor_ptr;
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
    case 0: return (byte & 128) >> 7;
    case 1: return (byte & 64) >> 6;
    case 2: return (byte & 32) >> 5;
    case 3: return (byte & 16) >> 4;
    case 4: return (byte & 8) >> 3;
    case 5: return (byte & 4) >> 2;
    case 6: return (byte & 2) >> 1;
    case 7: return (byte & 1);
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
  int i;
  char blockmap[BLOCKSIZE];
  int blockmap_byte = groupdescriptor_ptr->bg_block_bitmap * BLOCKSIZE;
  if(pread(fd, blockmap, BLOCKSIZE, blockmap_byte) == -1)
    exit_1("");
  
    /*Account for variable group size*/
  //each byte is 8 bits
  int num_iterations = superblock_ptr->s_blocks_per_group / 8;
  for(i = 0; i < num_iterations; i++)
  {
    /*Cycle through each of the bits in a given byte*/
    int j;
    for(j = 0; j < 8; j++)
      {
  	int bit = is_bit_set(blockmap[i],j);
  	if(! bit)
  	  {
  	    int block = i * 8 + j;
  	    printf("BFREE,%d\n", block);
	    
  	  }
  	else if(bit == -1)
  	  exit_1("Error with block bit-map.");
	       
      }
  }

  

  char inodemap[BLOCKSIZE];
  int inodemap_byte = groupdescriptor_ptr->bg_inode_bitmap * BLOCKSIZE;
  if(pread(fd, inodemap, BLOCKSIZE, inodemap_byte) == -1)
    exit_1("");

  num_iterations = superblock_ptr->s_inodes_per_group / 8;
  for(i = 0; i < num_iterations; i++)
    {
      int j;
      for(j = 0; j < 8; j++)
	{
	  int bit = is_bit_set(inodemap[i],j);
	  if(! bit)
	    {
	      int inode = i * 8 + j;
	      printf("IFREE,%d\n", inode);
	    }
	  else if(bit == -1)
	    exit_1("Error with inode bitmap.");
	}
    }
  
  

  exit(0);
       
  
}
