#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<semaphore.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<errno.h>
#include<string.h>
#include"ids.h"


sem_t * semaphore;

int semaphore_open = 0;

int fd = 0;

int shared_memory_open = 0;


char * buf = NULL;


void release_resources() {
  
  if ( semaphore_open ) {
    // Пытаемся его закрыть
    if ( sem_close( semaphore ) == -1 ) {
      perror( "Cannot close semaphore" );
    }
    
    semaphore_open = 0;
    
    if ( sem_unlink( SEMAPHORE_NAME ) == -1 ) {
      perror( "Cannot remove semaphore" );
    }
  }
  
  if ( buf != NULL ) {
    
    if ( munmap( buf, BUF_SIZE ) == -1 ) {
      perror( "Cannot unmap shared memory" );
    }
    buf = NULL;
  }
  
  if ( shared_memory_open ) {
    
    close( fd );
    
    if ( shm_unlink( SHM_NAME ) == -1 ) {
      perror( "Cannot remove shared memory" );
    }
    
    shared_memory_open = 0;
  }
}


void call_at_exit() {
 
  release_resources();
}


void sigHandler( int signum ) {
  
  release_resources();
  exit( 1 );
}


void update_buffer() {
  
  time_t t = time( NULL );
  
  struct tm * tm_t = localtime( & t );

  
  sprintf( buf+1, "Server PID=%d %02d.%02d.%04d %02d:%02d:%02d", 
           getpid(),
           tm_t->tm_mday, tm_t->tm_mon+1, tm_t->tm_year+1900, 
           tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec );

  buf[ 0 ] = strlen( buf+1 ) + 2;
}

int main() {

  
  atexit( call_at_exit );

 
  struct sigaction sa;
 
  sa.sa_handler = sigHandler;
  
  sigemptyset( &sa.sa_mask );

  sa.sa_flags = 0;

  
  if ( sigaction( SIGINT, & sa, NULL ) == -1 ) {
    perror( "Cannot install SIGINT handler" );
    exit( 1 );
  }

  
  semaphore = sem_open( SEMAPHORE_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0 );
 
  if ( semaphore == SEM_FAILED ) {
    
    if ( errno == EEXIST ) {
      fprintf( stderr, 
	       "Semaphore '%s' already exists. Another server is already running?\n", 
	       SEMAPHORE_NAME );
   
    } else {
      perror( "Cannot exclusively open semaphore" );
    }
    exit( 1 );
  }
 
  semaphore_open = 1;

  
  fd = shm_open( SHM_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR );
 
  if ( fd == -1 ) {
    
    if ( errno == EEXIST ) {
      fprintf( stderr, 
	       "Shared memory '%s' already exists. Another server is already running?", 
	       SHM_NAME );
    
    } else {
      perror( "Cannot exclusively open shared memory" );
    }
    exit( 1 );
  }
  
  shared_memory_open = 1;

  
  if ( ftruncate( fd, BUF_SIZE ) == -1 ) {
    perror( "Cannot resize shared memory" );
    exit( 1 );
  }

 
  buf = mmap( NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
  
  if ( buf == MAP_FAILED ) {
    buf = NULL;
    perror( "Cannot mmap shared memory" );
    exit( 1 );
  }

  
  update_buffer();

 
  if ( sem_post( semaphore ) == -1 ) {
    perror( "Cannot increase semaphore value" );
    exit( 1 );
  }

 
  while ( 1 ) {
   
    sleep( 1 );
    
    if ( sem_wait( semaphore ) == -1 ) {
      
      perror( "Cannot decrease semaphore value" );
      exit( 1 );
    }
   
    update_buffer();
    
    if ( sem_post( semaphore ) == -1 ) {
      perror( "Cannot increase semaphore value" );
      exit( 1 );
    }
  }

  
  return 0;
}
