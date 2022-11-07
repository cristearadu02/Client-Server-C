#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

int main(void)
{
 if (access("fifo.txt", F_OK) == -1)   
        {
          if (mknod("fifo.txt", S_IFIFO | 0666, 0) == -1) // verificam situatia in care apare vreo eroare la crearea fisierului
          { 
            perror("Unable to create fifo. ");
            exit(EXIT_FAILURE);
          }
        } 
        
    if(access("fifo2.txt", F_OK) == -1)
    {
       if (mknod("fifo2.txt", S_IFIFO | 0666, 0) == -1) // verificam situatia in care apare vreo eroare la crearea fisierului
          { 
            perror("Unable to create fifo. ");
            exit(EXIT_FAILURE);
          }
    }

int fd_fifo = open("fifo.txt", O_WRONLY);
if(fd_fifo == -1)
{
    perror("Unable to open fifo");
    exit(1);
}

int fd_fifo2 = open("fifo2.txt", O_RDONLY);
if(fd_fifo2 == -1)
{
    perror("Unable to open fifo2");
    exit(2);
}
printf("Guide:\n -login : user_name \n -get-logged-users\n -get-proc-info : PID\n -logout\n -quit\n");
 
 while(true)
 {
    char* command = (char*) calloc(256,sizeof(char));
    int len_command = read(0,command,255);

    write(fd_fifo,command,len_command);

    char response[256]=""; //fc = from client
    int len_response = read(fd_fifo2,response,255);

     printf("%ld%s\n",strlen(response),response);
    if(strcmp(response,"Program completed. ") == 0)
      break;
 }
    close(fd_fifo);
    close(fd_fifo2); 
return 0;
}
