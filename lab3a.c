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
  uint32_t num_inodes;
  char temp[1020];
} superblock_t;


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

int main(int argc, char **argv)
{
  superblock_t *myblock;  
  char *file = argv[1];
  int fd = open(file, O_RDONLY, 0444);

  uint8_t block_read[1024];
  if(!pread(fd, block_read, 1024, 1024))
    exit_1("");
  myblock = (superblock_t*) block_read;
  int i;
  printf("num inodes %d\n", myblock->num_inodes);
  
  
  printf("\n");
  
  


  return 0;
       
  
}
