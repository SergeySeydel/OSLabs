#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>

void usage() {
  printf( "usage:\n" );
  printf( "mychmod -h\n" );
  printf( "  - print help message\n" );
  printf( "mychmod -<options> <filename>" );
  printf( "  - change permissions of the <filename>\n" );
  printf( " <options> is string: \n" );
  printf( " <options>=<attrubite group flags><procedure flag><attribute flags> \n" );
  printf( "   <attrubute group flags> can contain:\n" );
  printf( "     'u' for user, 'g' for group, 'o' for others\n" );
  printf( "   <procedure flag> is 'a' for adding or 'r' for removing permissions \n" );
  printf( "   <attribute flags> can contain:\n" );
  printf( "     'r' for reading, 'w' for writing, 'x' for executing/searching \n" );
}


typedef struct {
  
  int helpFlag;
  
  
  int userFlag;
  
  int groupFlag;
  
  int otherFlag;
  
  int addingFlag;
  
  int readFlag;
  
  int writeFlag;
  
  int executeFlag;
  
  char * filename;
} CommandLine;


void parseCommandLine( int argc, char * argv[], CommandLine * cl ) {
  // Сброс всех флагов
  cl->filename = NULL;
  cl->helpFlag = 0;
  cl->userFlag = 0;
  cl->groupFlag = 0;
  cl->otherFlag = 0;
  cl->addingFlag = 0;
  cl->readFlag = 0;
  cl->writeFlag = 0;
  cl->executeFlag = 0;
  // Если флаг '-h'
  if ( ( argc == 2 ) && ( strcmp( argv[ 1 ], ( char * ) "-h" ) == 0 ) ) {
    cl->helpFlag = 1;
    return;
  }
  
  if ( argc == 3 ) {
    cl->filename = argv[ 2 ];
    char * argument = argv[ 1 ];
    size_t len = strlen( argument );
    if ( argument[ 0 ] != '-' ) {
      fprintf( stderr, "Bad command line\n" );
      usage();
      exit( 1 );
    }
    // Стадия разбора
    int stage = 0;
    // Анализируемый символ
    size_t i = 1;
    while( i < len ) {
      switch ( stage ) {
	// Стадия флагов 'u', 'g', 'o'
	case 0:
	  if ( argument[ i ] == 'u' ) {
	    cl->userFlag = 1;
	    i++;
	  } else if ( argument[ i ] == 'g' ) {
	    cl->groupFlag = 1;
	    i++;
	  } else if ( argument[ i ] == 'o' ) {
	    cl->otherFlag = 1;
	    i++;
	  } else if ( ( argument[ i ] == 'a' ) || ( argument[ i ] == 'r' ) ) {
	    stage = 1;
	  } else {
	    fprintf( stderr, "Bad command line\n" );
	    usage();
	    exit( 1 );
	  }
	  break;
	// Стадия флагов 'a' или 'r'
	case 1:
	  if ( argument[ i ] == 'a' ) {
	    cl->addingFlag = 1;
	    stage = 2;
	    i++;
	  } else if ( argument[ i ] == 'r' ) {
	    cl->addingFlag = 2;
	    stage = 2;
	    i++;
	  } else {
	    fprintf( stderr, "Bad command line\n" );
	    usage();
	    exit( 1 );
	  }
	  break;
	// Стадия флагов 'r', 'w', 'x'
	case 2:
	  if ( argument[ i ] == 'r' ) {
	    cl->readFlag = 1;
	    i++;
	  } else if ( argument[ i ] == 'w' ) {
	    cl->writeFlag = 1;
	    i++;
	  } else if ( argument[ i ] == 'x' ) {
	    cl->executeFlag = 1;
	    i++;
	  } else {
	    fprintf( stderr, "Bad command line\n" );
	    usage();
	    exit( 1 );
	  }
	  break;
	default:
	  fprintf( stderr, "Bad command line\n" );
	  usage();
	  exit( 1 );
	  break;
      }
    }
    // Должен быть хоть один флаг в каждой группе
    if ( ( ( cl->userFlag == 0 ) && ( cl->groupFlag == 0 ) && ( cl->otherFlag == 0 ) ) ||
         ( cl->addingFlag == 0 )                                                       ||
         ( ( cl->readFlag == 0 ) && ( cl->writeFlag == 0 ) && ( cl->executeFlag == 0 ) ) 
       ) {
      fprintf( stderr, "Bad command line\n" );
      usage();
      exit( 1 );
    }

    return;
  }
  fprintf( stderr, "Bad command line\n" );
  usage();
  exit( 1 );
}

void printCommandLine( CommandLine * cl ) {
  printf( "helpFlag = %d\n", cl->helpFlag );
  printf( "userFlag = %d\n", cl->userFlag );
  printf( "groupFlag = %d\n", cl->groupFlag );
  printf( "otherFlag = %d\n", cl->otherFlag );
  printf( "addingFlag = %d\n", cl->addingFlag );
  printf( "readFlag = %d\n", cl->readFlag );
  printf( "writeFlag = %d\n", cl->writeFlag );
  printf( "executeFlag = %d\n", cl->executeFlag );
  printf( "filename = '%s'\n", cl->filename );
}

void changePermissions( CommandLine * cl ) {
  
  struct stat statbuf;
  
  int result = lstat( cl->filename, & statbuf );
  if ( result != 0 ) {
    fprintf( stderr, "Filename '%s' caused an error: ", cl->filename );
    perror( NULL );
    exit( 1 );
  }
  
  mode_t mode = statbuf.st_mode;
  
  if ( cl->addingFlag == 1 ) {
    
    if ( cl->userFlag == 1 ) {
      if ( cl->readFlag == 1 )
	mode |= S_IRUSR;
      if ( cl->writeFlag == 1 )
	mode |= S_IWUSR;
      if ( cl->executeFlag == 1 )
	mode |= S_IXUSR;
    }
    
    if ( cl->groupFlag == 1 ) {
      if ( cl->readFlag == 1 )
	mode |= S_IRGRP;
      if ( cl->writeFlag == 1 )
	mode |= S_IWGRP;
      if ( cl->executeFlag == 1 )
	mode |= S_IXGRP;
    }
    
    if ( cl->otherFlag == 1 ) {
      if ( cl->readFlag == 1 )
	mode |= S_IROTH;
      if ( cl->writeFlag == 1 )
	mode |= S_IWOTH;
      if ( cl->executeFlag == 1 )
	mode |= S_IXOTH;
    }

  
  } else {
    
    if ( cl->userFlag == 1 ) {
      if ( ( cl->readFlag == 1 ) && ( mode & S_IRUSR ) )
	mode ^= S_IRUSR;
      if ( ( cl->writeFlag == 1 ) && ( mode & S_IWUSR ) )
	mode ^= S_IWUSR;
      if ( ( cl->executeFlag == 1 ) && ( mode & S_IXUSR ) )
	mode ^= S_IXUSR;
    }
    
    if ( cl->groupFlag == 1 ) {
      if ( ( cl->readFlag == 1 ) && ( mode & S_IRGRP ) )
	mode ^= S_IRGRP;
      if ( ( cl->writeFlag == 1 ) && ( mode & S_IWGRP ) )
	mode ^= S_IWGRP;
      if ( ( cl->executeFlag == 1 ) && ( mode & S_IXGRP ) )
	mode ^= S_IXGRP;
    }
    
    if ( cl->otherFlag == 1 ) {
      if ( ( cl->readFlag == 1 ) && ( mode & S_IROTH ) )
	mode ^= S_IROTH;
      if ( ( cl->writeFlag == 1 ) && ( mode & S_IWOTH ) )
	mode ^= S_IWOTH;
      if ( ( cl->executeFlag == 1 ) && ( mode & S_IXOTH ) )
	mode ^= S_IXOTH;
    }
  }

  
  result = chmod( cl->filename, mode );
  if ( result == -1 ) {
    fprintf( stderr, "Changing permissions caused an error: " );
    perror( NULL );
    exit( 1 );
  }

}

int main( int argc, char * argv[] ) {

  CommandLine cl;
  parseCommandLine( argc, argv, & cl );

  // Отладка
  //printCommandLine( & cl );

  if ( cl.helpFlag ) {
    usage();
  } else {
    changePermissions( & cl );
  }

  return 0;
}
