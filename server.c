#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include<sys/wait.h> 
#include <utmp.h>
#include <sys/socket.h>
#include <time.h>

#define Login "login : "
#define READ 0
#define WRITE 1

int Login_Check = 0;

int O_Login(char user_name[])
{ 
    user_name[strlen(user_name)-1]='\0';
 // printf("AM intrat in login\n");
    int pipe_fds[2];
    pipe(pipe_fds);
    pid_t pid = fork();

    if(pid == -1)
    {
        perror("Unable to fork in login. ");
        exit(1);
    }
        else if( pid == 0 )
            { //CHILD.
              //printf("Aici in copil\n");
              close(pipe_fds[READ]);

               int fd_users = open("Users.txt", O_RDONLY);
               if(fd_users == -1)
               {
                perror("Unable to open Users.txt");
                exit(1);
               } 
               else
               { //CHILD
                /*
                  char * line_length = NULL;
                  int line_length_size = 0;
                  ssize_t read_user; // un fel de len
                  while(read_user = getline_length(&line_length,&line_length_size,fd_users) != -1)
                    {  //printf("AM intrat in fgets.\n"); 
                       printf("\n%s\n",line_length);
                      //if(strcmp(line_length,user_name)==0)
                      // printf("Login succeded. Username: %s",user_name);
                      
                    }
                    close(fd_users);
                    if(line_length)
                      free(line_length);
                 printf("N am gasit");
                 */
                  
                  char users[1000];
                  int len = read(fd_users, &users, 999);
                  users[strlen(users)] = '\0';
                  close(fd_users);
                  char *p = NULL;
                  p = strtok(users,"\n");
                  bool found = 0;
                  
                 while(p && found == 0)
                 {
                   
                   //printf("%s p %ld, %s p %ld\n",p,strlen(p),user_name,strlen(user_name));
                   if(strcmp(p,user_name )==0 && strlen(p) == strlen(user_name))
                     {
                      write(pipe_fds[WRITE], "true", 4);
                        found = 1;
                     }
                    p = strtok(NULL,"\n");
                  }
                  if(found == 0)
                   write(pipe_fds[WRITE], "false", 5);
                
                }
              close(pipe_fds[WRITE]);

              int var=17;
              exit(var);  
            } 
            else
            {//PARENT
                char ok[10]="";
                close(pipe_fds[WRITE]);
                int len_ok = read(pipe_fds[READ],ok, 5);
                  if(strcmp(ok,"false")==0)
                   return 0;
                    else
                     return 1;

                close(pipe_fds[READ]);

                int child_return;
                wait(&child_return);
            }
}

int main(void)
{
     if (access("fifo.txt", F_OK) == -1)   
        {
          if (mknod("fifo.txt", S_IFIFO | 0666, 0) == -1) 
          { 
            perror("Unable to create fifo. ");
            exit(EXIT_FAILURE);
          }
        } 

    if(access("fifo2.txt", F_OK) == -1)
    {
       if (mknod("fifo2.txt", S_IFIFO | 0666, 0) == -1) 
          { 
            perror("Unable to create fifo. ");
            exit(EXIT_FAILURE);
          }
    }

 int fd_fifo = open("fifo.txt", O_RDONLY);
  if(fd_fifo == -1)
  {
    perror("Unable to open fifo.");
    exit(1);
  }

int fd_fifo2 = open("fifo2.txt", O_WRONLY);
if(fd_fifo2 == -1)
{
    perror("Unable to open fifo2");
    exit(1);
}


while(true)
 {  
    char command_fc[256]=""; //fc = from client
    int len_command = read(fd_fifo,command_fc,255);
     //printf("%s", command_fc);
    
    if( strstr(command_fc,"login")!=NULL )
       {   char user_name[25]="";
          int diff = strlen(command_fc)-8;
          strncpy(user_name,command_fc+8,diff);
          user_name[strlen(user_name)]='\0';
         // printf("%s", user_name);
         // write(fd_fifo2,"true",4);
          int ok = O_Login(user_name);
          if(Login_Check == 1)
           {
            write(fd_fifo2,"A user is already logged in.",29);
           }
           else if (ok == 1)
           {
            write(fd_fifo2,"Login succesfully.",19);
            Login_Check = 1;
            }
              else if(ok == 0)
              {
                write(fd_fifo2,"Can't find the specified user.",31);
              }
      }  
      else
        if(strcmp(command_fc,"get-logged-users\n")==0)
          {
            if(Login_Check == 1)
            {   
              int sockets[2];
              pid_t pid;
              socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);

              pid = fork();
              if(pid == -1)
              {
                perror("Unable to fork.");
                exit(1);
              } 
              else if(pid == 0)
                 {//CHILD - 0
                    close(sockets[1]);
                     
                     struct utmp *n;
                     setutent();
                     n = getutent();
                     char output[500]="";
                     while(n)
                      { 
                        if(n->ut_type == USER_PROCESS)
                       {
                        strncat(output,n->ut_user,32);
                        strcat(output,"\n");
                        strncat(output,n->ut_host,32);
                        strcat(output,"\n");
                         time_t tmp;  
                         tmp = n->ut_tv.tv_sec;
                         char timeS[100]="";
                         strcpy(timeS,ctime(&tmp));
                         timeS[strlen(timeS)]='\0';
                        
                         strcat(output,timeS);
                       }
                       n = getutent();
                      }

                      output[strlen(output)]='\0';
                      write(sockets[0],output,strlen(output));
              
                      close(sockets[0]);

                    int var=16;
                    exit(var);  
                 } 
                 else
                 {//PARENT - 1
                    close(sockets[0]);
                    
                    char output[4096]="";
                
                    int len_output = read(sockets[1],output,4096);
                    write(fd_fifo2,output,len_output);

                    close(sockets[1]);

                    int child_return;
                    wait(&child_return);
                 }
            }
            else
              {
                write(fd_fifo2,"Please login first.",20);  
                Login_Check = 0;
              }
          }
          else if(strcmp(command_fc,"logout\n")==0)
          { if(Login_Check == 1)
              {
                write(fd_fifo2,"Logout completed.",18);
                Login_Check = 0;
              }
              else
                {
                  Login_Check = 0;
                  write(fd_fifo2,"No user logged in. ",20);
                }
            Login_Check = 0;
                
          }
          else if(strcmp(command_fc,"quit\n")==0)
          {
            write(fd_fifo2,"Program completed. ",20);
             break; 
          }
          else if(strstr(command_fc,"get-proc-info : "))
           {
             if(Login_Check==0)
              { Login_Check = 0;
                write(fd_fifo2,"Please login first.",20);
               }
            else
            {
              int pipe_proc[2];
              pid_t pid;
              pipe(pipe_proc);
                pid = fork();
                if(pid == -1)
                  { 
                    perror("Unable to fork");
                    exit(1);
                  }
                  else if(pid == 0)
                  {//CHILD
                    close(pipe_proc[READ]);
                    char PID[20]="";
                    char source[60]="";
                    char pid_aux[20]="";
                    strcpy(pid_aux,command_fc+16);
                    strncpy(PID,pid_aux,strlen(pid_aux)-1);
                    strcpy(source,"/proc/");
                    strcat(source,PID);
                    strcat(source,"/status");
                    source[strlen(source)]='\0';
                    //printf("%s\n",source);
                    int fd_usernames;
                    if((fd_usernames = open(source,O_RDONLY)) == -1)
                      {
                        write(pipe_proc[WRITE],"Unable to find specified PID.",30);
                      }
                      else { 
                    char output[500]="";
                    char ProcSpecs[5000]; 
                    ProcSpecs[0] = '\0'; 
                    int line_length = read(fd_usernames,ProcSpecs,5000); 
                    ProcSpecs[line_length]='\0';
                       // printf("\n%s\n",ProcSpecs);
                    while(line_length != 0)
                      {
                        char *p = strtok(ProcSpecs,"\n");
                        while(p)
                        { // name state ppid uid vmsize
                            if(strstr(p,"Uid")!=NULL || strstr(p,"Name")!=NULL || strstr(p,"PPid")!=NULL || strstr(p,"State")!= NULL)
                            {   strcat(output,p);
                                strcat(output,"\n");
                                //write(pipe_proc[WRITE],p,strlen(p)-1);
                               // write(pipe_proc[WRITE],"; \n",3);
                            }
                            else if(strstr(p,"VmSize")!=NULL)
                            {
                              strcat(output,p);
                            }
                            p=strtok(NULL,"\n");
                        }
                        line_length = read(fd_usernames,ProcSpecs,5000); 
                        ProcSpecs[line_length]='\0';

                          }
                       output[strlen(output)]='\0';
                       write(pipe_proc[WRITE], output,strlen(output));     
                       close(fd_usernames);
                     }
                     close(pipe_proc[WRITE]);
                  }
                   else
                   {//PARENT
                    close(pipe_proc[WRITE]);

                    char aux[256]; 
                    aux[0]='\0'; 
                    int auxCitit; 
                    auxCitit=read(pipe_proc[READ],aux,256);

                    aux[auxCitit]='\0';

                    write(fd_fifo2,aux,auxCitit);
                    close(pipe_proc[READ]);
                   }
              }

            }
            else
            {
              write(fd_fifo2,"Unable to identify command.",28);
            }
               
 }
    close(fd_fifo);
    close(fd_fifo2); 

}