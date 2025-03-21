#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int t=15;
char gate_state;
time_t start_time,end_time;
double time_passed;

void functionSIGUSR1c(int shma) {
  end_time=time(NULL);
  double time_passed = difftime(end_time, start_time);
  if (gate_state=='f') {
    gate_state = 't';
    printf("[GATE=0/PID=%d/TIME=%lds] The gates are open!\n", getpid(),(long)time_passed);
  }
  else {
    gate_state = 'f';
    printf("[GATE=0/PID=%d/TIME=%lds] The gates are closed!\n", getpid(),(long)time_passed);
  }
}

void functionSIGUSR2c(int shma) {
  end_time=time(NULL);
  double time_passed = difftime(end_time, start_time);
  if (gate_state=='f') {
    printf("[GATE=0/PID=%d/TIME=%lds] The gates are closed!\n", getpid(),(long)time_passed);
  }
  else {
    printf("[GATE=0/PID=%d/TIME=%lds] The gates are open!\n", getpid(),(long)time_passed);
  }
}

void functionSIGTERMc(int shma) {
  kill(getppid(),SIGCHLD); //stelnetai shma ston patera (stelnetai mono tou (!))
  exit(0);
  /*kill(getpid(), SIGTERM);
  exit(EXIT_FAILURE); // this shouldnt happen*/
}

void functionSIGALRMc(int shma) {
  if (gate_state=='t') {
    printf("[GATE=0/PID=%d/TIME=%ds] The gates are open!\n", getpid(),t);
  }
  else {
    printf("[GATE=0/PID=%d/TIME=%ds] The gates are closed!\n", getpid(),t);
  }
  t += 15;
  alarm(15);
}

int main(int argc, char **argv) {

  struct sigaction sig1,sig2,trm,alm;
  sig1.sa_handler =functionSIGUSR1c;
  sig2.sa_handler =functionSIGUSR2c;
  trm.sa_handler =functionSIGTERMc;
  alm.sa_handler =functionSIGALRMc;
  trm.sa_flags = 0;
  sig1.sa_flags = 0;
  sig2.sa_flags = 0;
  alm.sa_flags = 0; //or else error "Alarm Clock"

  sigaction(SIGUSR1, &sig1, NULL);
  sigaction(SIGUSR2, &sig2, NULL);
  sigaction(SIGTERM, &trm, NULL);
  sigaction(SIGALRM, &alm, NULL);

  gate_state = argv[1][0];
  if (gate_state=='t') {
    printf("[GATE=0/PID=%d/TIME=0s] The gates are open!\n", getpid());
  }
  else {
    printf("[GATE=0/PID=%d/TIME=0s] The gates are closed!\n", getpid());
  }

  start_time = time(NULL);
  alarm(15);

  while(1) {
    //
  }
return 0;
}
