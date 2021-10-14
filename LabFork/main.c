#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

void doAtExit() {
  printf( "atexit handler called!\n" );
}


void print_PID_PPID() {
  pid_t pid = getpid();
  pid_t ppid = getppid();
  printf( "My PID=%d PPID=%d\n", pid, ppid );
}

int main() {


  if ( atexit( doAtExit ) != 0 ) {

    fprintf( stderr, "cannot set exit function\n" );
    exit( 1 );
  }


  pid_t id = fork();

  if ( id == -1 ) {

    perror( NULL );
    exit( 1 );
  }


  if ( id == 0 )
	{
		
    printf( "I'm child " );
    print_PID_PPID();
    printf( "Waiting 3 seconds...\n" );
    sleep( 3 );
    printf( "Child finished its work!\n" );

	}
	else
	{
		
    int status;
    printf( "I'm parent, created child with PID=%d\n", id );
    print_PID_PPID();

    if ( waitpid( id, & status, 0 ) == -1 )
	{
		if (WIFEXITED(status)){
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
		else if (WIFSIGNALED(status)){
        psignal(WTERMSIG(status), "Exit signal");
    }
      fprintf( stderr, "waitpid in parent process failed!\n" );
      exit( 1 );
    }
    printf( "Parent process outlived child process\n" );
    printf( "Parent finished its work!\n" );
  }

  return 0;
}
