#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>
#include<string.h>
#include<limits.h>


void err_sys( const char * info ) {
  perror( info );
  exit( 1 );
}

int main() {
  
  int fd[ 2 ];
 
  pid_t pid;

 
  char buf[ 512 ];

 
  if ( pipe( fd ) < 0 ) {
    err_sys( "pipe error" );
  }

 
  if ( ( pid = fork() ) < 0 ) {
    
    err_sys( "fork error" );


  
  } else if ( pid == 0 ) {

   
    close( fd[ 1 ] );

    
    int len;

    
    if ( read( fd[0], buf, 1 ) != 1 ) {
        
	err_sys( "read operation to pipe falied" );
    }

    
    len = buf[ 0 ];

    
    if ( read( fd[0], buf, len ) != len ) {
        
	err_sys( "read operation to pipe falied" );
    }

    printf( "Received message from parent: '%s'\n", buf );

   
    time_t t = time( NULL );
  
    struct tm * tm_t = localtime( & t );

    
    sprintf( buf, "%02d.%02d.%04d %02d:%02d:%02d", 
	     tm_t->tm_mday, tm_t->tm_mon+1, tm_t->tm_year+1900, 
	     tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec );

    
    printf( "My PID=%d. Current time: %s\n", getpid(), buf );

    
    close( fd[ 0 ] );

  
  } else {

   
    close( fd[ 0 ] );

    
    time_t t = time( NULL );
    
    struct tm * tm_t = localtime( & t );

    
    sprintf( buf+1, "PID: %d, time: %02d.%02d.%04d %02d:%02d:%02d", 
	     getpid(), tm_t->tm_mday, tm_t->tm_mon+1, tm_t->tm_year+1900, 
	     tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec );
    
    buf[ 0 ] = ( char ) ( strlen( buf + 1 ) + 1 );

    
    sleep( 6 );

    
    if ( write( fd[1], buf, buf[0]+1 ) != buf[0] + 1 ) {
     
      err_sys( "write operation to pipe falied" );
    }

   
    close( fd[ 1 ] );

  }


  return 0;
}
