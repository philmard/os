#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char **argv){

//dieukrinisi 1
if(argc!=2){
    printf("Usage: %s filename\n", argv[0]);
    return 1;
}

//dieukrinisi 3
if(strcmp(argv[1], "--help") == 0) {
    printf("Usage: %s filename\n", argv[0]);
    return 0;
}

//dieukrinisi 2
if (access(argv[1], F_OK) == 0) {
    // file exists
    printf("Error: %s already exists\n",argv[1]);
    return 1; 
} 

int fd;
int status;
char p_buf[100];
char c_buf[100];
fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);
if (fd == -1) {
perror("open");
return 1;
}
pid_t child_pid, parent_pid;
child_pid = fork();

if(child_pid < 0){
    printf("Error\n");
    return 1;
}
else if(child_pid == 0){
    //child's code
        sprintf(c_buf, "[CHILD] getpid()=%d, getppid()=%d\n", getpid(), getppid());
        if (write(fd, c_buf, strlen(c_buf)) < strlen(c_buf)) {
            perror("write");
            return 1;
        }
        exit(0);
} 
else{
    //parent's code
        sprintf(p_buf, "[PARENT] getpid()=%d, getppid()=%d\n", getpid(), getppid());
        wait(&status);
        if (write(fd, p_buf, strlen(p_buf)) < strlen(p_buf)) { //write returns the number of bytes successfully written into the file
            perror("write");
            return 1;
        }
        exit(0);
}

close(fd);
}
