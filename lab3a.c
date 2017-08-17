#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include "ext2_fs.h"


int BLOCKSIZE;
int full_inodes[24];
__u8 superblock_read[EXT2_MAX_BLOCK_SIZE];

/****************************/
/*Block pointer declarations*/
/***************************/
struct ext2_super_block *superblock_ptr;
struct ext2_group_desc *groupdescriptor_ptr;
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
   int blocksize = 1024 << superblock_ptr->s_log_block_size;
   BLOCKSIZE = blocksize;


    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock_ptr->s_blocks_count,
	   superblock_ptr->s_inodes_count, blocksize, superblock_ptr->s_inode_size,
	   superblock_ptr->s_blocks_per_group, superblock_ptr->s_inodes_per_group,
	   superblock_ptr->s_first_ino);


}

void group_output()
{
  return;
}

int is_bit_set(__u8 byte, int index)
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

void superblock_info(int fd, __u8 superblock_read[])
{
  if(pread(fd, superblock_read, EXT2_MAX_BLOCK_SIZE, 1024) == -1)
    exit_1("");
  superblock_ptr = (struct ext2_super_block*) superblock_read;
  superblock_output();
}

void free_bits(__u8 map_read[], int block_flag, int full_inodes[])
{
  int i;
  int num_iterations;
  
  if(block_flag)
    num_iterations = superblock_ptr->s_blocks_per_group / 8;
  
  else
    num_iterations = superblock_ptr->s_inodes_per_group / 8;
  
  for(i = 0; i < num_iterations; i++)
    {
          /*Cycle through each of the bits in a given byte*/
    int j;
    for(j = 0; j < 8; j++)
      {
  	int bit = is_bit_set(map_read[i],j);
      	int map_index = i * 8 + j;
  	if(! bit)
  	  {

	    if(block_flag && !full_inodes)
		printf("BFREE,%d\n", map_index);
	    
	    else if(full_inodes && !block_flag)
	      {
		printf("IFREE,%d\n", map_index);
		
	        full_inodes[map_index] = 0;
	      }
	       
  	  }
	
	else if(bit == 1 && full_inodes)
          full_inodes[map_index] = 1;
	
  	else if(bit == -1)
  	  exit_1("Error with bit-map.");
	       
      }

    }
  
}

void inode_table_analysis(__u8 inode_table_read[], int full_inodes[], int NUM_INODES)
{
  int i;
  
  __u16 INODE_SIZE = superblock_ptr->s_inode_size;
  struct ext2_inode *inode_table = malloc(INODE_SIZE * NUM_INODES);
  inode_table = (struct ext2_inode*) inode_table_read;
  /*Every 128 bytes contains an inode*/
  for(i = 0; i < NUM_INODES; i++)
    {
      
      if(full_inodes[i])	
	{
	  if(inode_table[i].i_mode == 0)
	    {
	    printf("Inode,%d mode = 0\n", i);
	    continue;
	    }
	  
	  int file_size = inode_table[i].i_size;
	  printf("INODE,%d,%d\n",i,file_size);


	}
    }

}


int main(int argc, char **argv)
{

  char *file = argv[1];
  int fd = open(file, O_RDONLY, 0444);
  if(fd == -1)
    exit_1("");

  
  /*************/
  /*SUPER BLOCK*/
  /************/
  superblock_info(fd, superblock_read);


  /************************/
  /*BLOCK DESCRIPTOR TABLE*/
  /***********************/
  
  uint8_t blockgroup_read[BLOCKSIZE];
  if(pread(fd, blockgroup_read, BLOCKSIZE, 2048) == -1)
    exit_1("");
  
  groupdescriptor_ptr = (struct ext2_group_desc*) blockgroup_read;

  /*Print the info about the block groups somewhere here?*/

  
  
  /**********************/
  /*BITMAP AND INODE MAP*/
  /*********************/
  
  /*Get the block number where the bitmap is*/

  __u8 blockmap[BLOCKSIZE];
  int blockmap_byte = groupdescriptor_ptr->bg_block_bitmap * BLOCKSIZE;
  if(pread(fd, blockmap, BLOCKSIZE, blockmap_byte) == -1)
    exit_1("");
  
  /*Account for variable group size*/
  //each byte is 8 bits


  int block_flag = 1;
  free_bits(blockmap, block_flag, NULL);

  
  /*Get the free inodes, pretty much the same implementation as free blocks*/
  __u8 inodemap[BLOCKSIZE];
  
  int NUM_INODES = superblock_ptr->s_inodes_per_group;

  /*0 means empty, 1 means full*/
  int full_inodes[NUM_INODES];
  
  int inodemap_byte = groupdescriptor_ptr->bg_inode_bitmap * BLOCKSIZE;

  
  if(pread(fd, inodemap, BLOCKSIZE, inodemap_byte) == -1)
    exit_1("");
  

  
  block_flag = 0;
  
  free_bits(inodemap, block_flag, full_inodes);
  
  /**************************/
  /*START INODE SUMMARY HERE*/
  /*************************/


  __u8 inode_table_read[BLOCKSIZE];
  memset(inode_table_read, 0, BLOCKSIZE);

  int inode_table_byte = groupdescriptor_ptr->bg_inode_table * BLOCKSIZE;


  if(pread(fd, inode_table_read, BLOCKSIZE, inode_table_byte) == -1)
    exit_1("");

  inode_table_analysis(inode_table_read, full_inodes, NUM_INODES);



  exit(0);
  
       
  
}
