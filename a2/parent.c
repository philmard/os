#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h> //sigaction()
#include <string.h>

/**
 * \brief If ret is negative, print an error message and exit with EXIT_FAILURE
 * status code
 *
 * \param ret Value to check
 * \param msg Error message to prepend
 */

 
int N;
char *initial_state; //malloc
pid_t *child_pids; //malloc

void describe_wait_status(pid_t pid, int status) {
  if (pid < 1) {
    perror("wait() call failed");
  }

  if (pid == 0) {
    printf("Nothing happened");
  }

  if (WIFSTOPPED(status)) { // Returns true if the child process exited due to a SIGSTOP, SIGTSTP, SIGTTIN, or SIGTTOU signal.
    printf("Child with PID %d stopped\n", pid); //the child process with the given PID has been stopped
  } else if (WIFCONTINUED(status)) { // Returns true if the child process was resumed by a SIGCONT signal
    printf("Child with PID %d continued\n", pid); //child process with the given PID has been resumed after being stopped.
  } else if (WIFEXITED(status)) { // Returns true if the child process exited normally (with an exit status between 0 and 255)
    printf("Child with PID %d exited with status code %d\n", pid, WEXITSTATUS(status)); //child process with the given PID has exited normally
  } else if (WIFSIGNALED(status)) { // Returns true if the child process exited due to a signal other than SIGSTOP, SIGTSTP, SIGTTIN, or SIGTTOU.
    printf("Child with PID %d terminated by signal %d with status code %d\n", pid, WSTOPSIG(status), WEXITSTATUS(status)); //child process with the given PID has been terminated by a signal
  }
}

void check_neg(int ret, const char *msg) { // for fork(), execv()
  if (ret < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

void functionSIGUSR1(int shma) { //++elegxos(?)
  for (int i=0; i<N; i++) {
    kill(child_pids[i],shma);
  }
}

void functionSIGUSR2(int shma) { //++elegxos(?)
  for (int i=0; i<N; i++) {
    kill(child_pids[i],shma);
  }
}

void functionSIGTERM (int shma) {
  int number=N;
  int i=0;
  while (number > 0) {
    printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(),number);
    kill(child_pids[i],shma); 
    printf("[PARENT/PID=%d] Child with PID=%d terminated successfully with exit status code 0!\n",getpid(), child_pids[i]); //++elegxos
    number--;
    i++;
  }
  printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid());
  exit(0);
}

void functionSIGCHLD (int shma) {
  int status; //dokimazw me WNOHANG
  pid_t pid = waitpid(-1, &status, 0);
  //here, we know status and pid of dead child

  pid_t old_pid=getpid(); //father_pid
  
  if (WIFEXITED(status)) { //WIFSIGNALED kanonika..
    int k=0;
    //printf("Child process %d was killed, creating a new one...\n", pid);
    pid_t new_pid = fork();
    check_neg(new_pid, "error fork"); //elegxos fork()

    for (k=0; k<N; k++) { //replace dead_child_pid with new_child_pid
      if (child_pids[k]==pid) {
        child_pids[k]=new_pid;
        break;
      }
    }

    if (getpid()==old_pid) { //print once (when in parent process)
      printf("[PARENT/PID=%d] Child %d with PID=%d exited \n", getpid(), k, pid);
    }

    if (new_pid == 0) { //new_child's code
       //problem here, but does execv(?)(?)(?)(?)(?)
      if  (initial_state[k]=='f') {
        printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state 'f'", old_pid, k, child_pids[k]); //doesn't work..(?)
        char *const argv[] = {"./child", "f", NULL};
        int check = execv("./child", argv); //This replaces the current process image with a new process image loaded from the specified executable file.
        /* on success, execution will never(!) reach this line */ 
        check_neg(check, "Failed to create child");     
      }
      else if (initial_state[k]=='t') {
        printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state 't'", old_pid, k, child_pids[k]); //doesn't work..(?)
        char *const argv[] = {"./child", "t", NULL};
        int check = execv("./child", argv); //This replaces the current process image with a new process image loaded from the specified executable file.
        /* on success, execution will never(!) reach this line */ 
        check_neg(check, "Failed to create child");
      }
    }
    //
  }
}

/**
 * \brief Parse result of wait() system call and print a descriptive message to
 * stdout.
 *
 * Refer to `man 2 wait` for more details regarding the meaning of pid and
 * status.
 *
 * \param pid PID returned by wait()
 * \param status Status returned by wait()
 */


int main(int argc, char **argv) {

  if(argc!=2) {
    printf("wrong usage. Type 2 arguments\n");
    return 1;
  }
  char char_input[strlen(argv[1])];
  strcpy(char_input,argv[1]);
  for(int i=0; i<strlen(char_input); i++) {
        if((char_input[i] != 'f') && (char_input[i] != 't')) {
          printf("Wrong usage. In 2nd argument type 'f' for closed gate and 't' for open gate\n");
            return 1;
        }
    }

  struct sigaction sig1,sig2,trm,cld; //defined in <signal.h> 
  trm.sa_flags = 0;
  sig1.sa_flags = 0;
  sig2.sa_flags = 0;
  cld.sa_flags = 0;
  sig1.sa_handler =functionSIGUSR1;
  sig2.sa_handler =functionSIGUSR2;
  trm.sa_handler =functionSIGTERM;
  cld.sa_handler =functionSIGCHLD;
  
  //it also has sa_sigaction, sa_mask, ... (we don't need those)

  //orismos handler tou kathe shmatos
  sigaction(SIGUSR1, &sig1, NULL); //NULL for old handler
  sigaction(SIGUSR2, &sig2, NULL);
  sigaction(SIGTERM, &trm, NULL);
  sigaction(SIGCHLD, &cld, NULL);

  N = strlen(argv[1]);

  initial_state = (char *) malloc(N * sizeof(char));
  child_pids = (pid_t *) malloc(N * sizeof(pid_t));


  for(int i = 0;i < N; i++) {
    pid_t p = fork();
    check_neg(p, "fork");
    child_pids[i] = p;
    initial_state[i] = argv[1][i];
    
    if (p == 0) { //child code
      /* child, load "./child" executable */
      if  (initial_state[i]=='f') {
        char *const argv[] = {"./child", "f", NULL};
        int check = execv("./child", argv); //This replaces the current process image with a new process image loaded from the specified executable file.
        /* on success, execution will never(!) reach this line */ 
        check_neg(check, "Failed to create child");      
      }
      else if (initial_state[i]=='t') {
        char *const argv[] = {"./child", "t", NULL};
        int check = execv("./child", argv); //This replaces the current process image with a new process image loaded from the specified executable file.
        /* on success, execution will never(!) reach this line */ 
        check_neg(check, "Failed to create child");
      }
    }
    //parent's code
    printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, p, initial_state[i]);
  }

while(1) {

}
  /*int status;
  for (int i = 0; i < N; i++) {
    while (waitpid(-1,&status,0) <= 0) { //not positive
      //
    }
  }*/
  return 0;
}
