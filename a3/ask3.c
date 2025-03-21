#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    int f_val;
    int status;
    int k=0;
    fd_set rfds; // set of "read" fd's of parent
    int retval;
    int max_fd=0;
    char buffer[256];
    int notint=0;
    int NUM_CHILDREN;
    bool israndom=false ,isrobin=true;

    if(argc!=2 && argc!=3){
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        exit(0);
    }
    if(argc==3){
        if(strcmp(argv[2],"--random")==0){
            israndom=true;
            isrobin=false;
        }
        else if(strcmp(argv[2],"--round-robin")==0){
            isrobin=true;
        }
        else{
            printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
            exit(0);
        }
    }
    // Read from stdin
    strcpy(buffer,argv[1]);

    // first, check if buffer is an int
    if ((strlen(buffer) > 0)) { // check for non-numeric characters
        for (int i = 0; i < strlen(buffer)-1; i++) {
            if (!isdigit(buffer[i])) {
                //printf("[Parent] %s is not an integer.\n", buffer);
                printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                notint=1;
                
                break;
            }
        }
        if (notint==0) { // is int
            NUM_CHILDREN=atoi(buffer);
        }
    }
    int pipes_child_to_parent[NUM_CHILDREN][2];
    int pipes_parent_to_child[NUM_CHILDREN][2];
    pid_t c_pid[NUM_CHILDREN];
    
    // create N pipes for N children
    // Create pipes for each child
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(pipes_child_to_parent[i]) == -1 || pipe(pipes_parent_to_child[i]) == -1) {
            perror("pipe() failed");
            exit(1);
        }
    }
    // pipes are ready here

    // Fork N children
    for (int i=0; i < NUM_CHILDREN; i++) {
        //sleep(1);
        c_pid[i]=fork();
        if (c_pid[i] < 0) {
            perror("fork() failed");
            exit(1);
        }
        else if (c_pid[i] == 0) { // child[i]'s code
            int val;
            while(1) { // gnwrizw to [i]
                read(pipes_parent_to_child[i][0], &val, sizeof(int)); // ola ta children kollane edw
                                                      // otan kapoio child i dextei timh, tote to child i ksekollaei
                printf("[Child %d][%d] Child received %d!\n", i, getpid(), val);
                val++;
                sleep(5);
                printf("[Child %d][%d] Child Finished hard work, writing back %d\n", i, getpid(), val);
                write(pipes_child_to_parent[i][1], &val, sizeof(int));
            }
            //printf("[Child %d] I will die now\n",i);
            c_pid[i]=getpid(); // wste c_pid[i] swsta meta to telos tou for loop.
            exit(0);
        }
        // parent's code
        printf("[Parent] I created child %d\n",i);
    }

    // parent's code, here every child has already been created.
    
    while(1) {
        notint=0;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds); // vazw stdin sto set (exei prohgh8ei #define STDIN_FILENO 0)
        for (int j=0; j < NUM_CHILDREN; j++) { // vazw akra anagnwshs tou patera mesa sto set
            FD_SET(pipes_child_to_parent[j][0], &rfds);
            if (max_fd < pipes_child_to_parent[j][0]) max_fd = pipes_child_to_parent[j][0]; // to also calculate the max_fd
        }

        //printf("[Parent] stuck on select()\n");
        retval = select(max_fd+1, &rfds, NULL, NULL, NULL); // timeout = infinite
        if (retval == -1) perror("[Parent] select() failed");
        else { 
            // edw prp na ta ksexwrizw kapws ta rfd's
            //ksexwrismos rfd's
            if (FD_ISSET(0, &rfds)) { // is true when stdin
                // Read from stdin
                fgets(buffer, sizeof(buffer), stdin);
                if(strcmp(buffer,"exit\n")==0){
                    printf("All children terminated\n");
                    for(int i=0; i<NUM_CHILDREN;i++){
                        printf("child exit\n");
                        kill(c_pid[i],SIGTERM);
                    }
                    printf("Parent exit\n");
                    exit(0);
                }
                // first, check if buffer is an int
                if ((strlen(buffer) > 0)) { // check for non-numeric characters
                    for (int i = 0; i < strlen(buffer)-1; i++) {
                        if (!isdigit(buffer[i])) {
                            //printf("[Parent] %s is not an integer.\n", buffer);
                            printf("Type a number to send job to a child!\n");
                            //memset(buffer, 0, sizeof(buffer));
                            notint=1;
                            break;
                        }
                    }
                     //printf("check5\n");
                    if (notint==0) { // is int
                        f_val=atoi(buffer);
                        printf("[Parent] Assigned %d to child %d\n", f_val, k);
                        write(pipes_parent_to_child[k][1], &f_val, sizeof(int)); // stelnw thn ergasia sto paidi
                        if(isrobin){
                            k++; // round robin
                            k=k%NUM_CHILDREN; // round robin
                            notint=0;
                        }
                        else if(israndom){
                            k=rand()%NUM_CHILDREN;
                            notint=0;
                        }
                    }
                }
            }
            else { // not stdin
                for (int j=0; j < NUM_CHILDREN; j++) { // check which child has sent smth
                    if (FD_ISSET(pipes_child_to_parent[j][0], &rfds)) { // found which child
                        read(pipes_child_to_parent[j][0], &f_val, sizeof(int)); // elave thn timh
                        printf("[Parent] Received result from child %d-->%d\n", j, f_val);
                    }
                }
            }
        }
        // reinitialize fd_set bc we 're in a loop (!)
    }
    for (int i=0; i < NUM_CHILDREN; i++) {
        wait(&status);
    }
    printf("[Parent] parent will also exit now\n");
    exit(0);
}
