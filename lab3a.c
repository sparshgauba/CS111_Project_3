#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>


typedef struct superblock{
  uint32_t num_inodes[256];
  
} superblock_t;




int main(int argc, char **argv)
{
  superblock_t *myblock;
  
  char *file = argv[0];
  int fd = open(file, O_RDONLY, 0444);
  uint8_t block_read[1024];
  pread(fd, block_read, 1024, 1024);
  block_read[1024] = '\0';
  
  printf("%X\n", block_read);
  
  


  return 0;
       
  
}
