#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
  char *file = argv[0];
  int fd = open(file, O_RDONLY, 0444);
  char inodes[1024];
  pread(fd, inodes, 1024, 1024);
  int i;
  for(i = 0; i < 1023; i++)
    {
      printf("%X", inodes[i]);
      if(i % 8 == 0)
	printf("\n");

    }
  printf("\n");
  
  return 0;
       
  
}
