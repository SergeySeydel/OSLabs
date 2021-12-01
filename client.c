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
    
    if ( sem_close( semaphore ) == -1 ) {
      perror( "Cannot close semaphore" );
    }
    
    semaphore_open = 0;
  }
 
  if ( buf != NULL ) {
    
    if ( munmap( buf, BUF_SIZE ) == -1 ) {
      perror( "Cannot unmap shared memory" );
    }
    buf = NULL;
  }
  
  if ( shared_memory_open ) {
    
    close( fd );
    
    shared_memory_open = 0;
  }
}

int main() {

  
  semaphore = sem_open( SEMAPHORE_NAME, 0 );
  
  if ( semaphore == SEM_FAILED ) {
    
    if ( errno == ENOENT ) {
      fprintf( stderr, 
	       "Semaphore '%s' is not exist. No server is running?\n", 
	       SEMAPHORE_NAME );
    
    } else {
      perror( "Cannot open semaphore" );
    }
    exit( 1 );
  }
  
  semaphore_open = 1;

 
  fd = shm_open( SHM_NAME, O_RDONLY, 0 );
  
  if ( fd == -1 ) {
    
    if ( errno == ENOENT ) {
      fprintf( stderr, 
	       "Shared memory '%s' is not already exist. No server is running?", 
	       SHM_NAME );
    
    } else {
      perror( "Cannot open shared memory" );
    }
    exit( 1 );
  }
  
  shared_memory_open = 1;

 
  buf = mmap( NULL, BUF_SIZE, PROT_READ, MAP_SHARED, fd, 0 );
  
  if ( buf == MAP_FAILED ) {
    buf = NULL;
    perror( "Cannot mmap shared memory" );
    exit( 1 );
  }

 
  if ( sem_wait( semaphore ) == -1 ) {
    perror( "Cannot decrease semaphore value" );
    exit( 1 );
  }

  printf( "Client with PID=%d.\nRead from server string '%s'\n", getpid(), buf + 1 );

  
  if ( sem_post( semaphore ) == -1 ) {
    perror( "Cannot increase semaphore value" );
    exit( 1 );
  }

  
  release_resources();


  return 0;
}
