#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<semaphore.h>
#include<signal.h>
#include<unistd.h>
#include<errno.h>
#include"ids.h"

int main() {
    

  if ( sem_unlink( SEMAPHORE_NAME ) == -1 ) {
    perror( "Cannot remove semaphore" );
  }

  
  if ( shm_unlink( SHM_NAME ) == -1 ) {
    perror( "Cannot remove shared memory" );
  }

  return 0;
}
