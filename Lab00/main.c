#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<dirent.h>
#include<string.h>


struct Node {
  char * filename; 
  struct Node * next;
};


struct Node * getNewNode() {

  struct Node * newNode = ( struct Node * ) malloc( sizeof( struct Node ) );
  
  if ( newNode == NULL ) {
    fprintf( stderr, "Not enouph memory\n" );
    
    exit( 1 );
  }
  
  newNode->next = NULL;
  newNode->filename = NULL;

  return newNode;
}


struct Node * getNode( char * filename ) {
  
  struct Node * newNode = getNewNode();
  
  newNode->filename = filename;
  
  return newNode;
}


void freeNodes( struct Node * list ) {
  struct Node * next;
  while ( list != NULL ) {
    next = list->next;
    list->next = NULL;
    free( list );
    list = next;
  }
}

 
void parseCommandline( int argc, char * argv[], struct Node ** list, bool * lFlag ) {
 
  struct Node * curList = NULL;
  
  *lFlag = false;
  
  for ( int i = 1; i < argc; i++ ) {
    
    if ( strcmp( argv[ i ], ( char * ) "-l" ) == 0 ) {
      *lFlag = true;
      continue;
    
    } else {
      
      struct Node * newNode = getNode( argv[ i ] );
      
      newNode->next = curList;
      curList = newNode;
    }
  }
  
  if ( curList == NULL ) {
    
    curList = getNode( ( char * ) "." );
  }
  
  *list = curList;
}


void printList( struct Node * list ) {
  if ( list == NULL ) {
    printf( "filename list is empty\n" );
  } else {
    while ( list != NULL ) {
      printf( "%s\n", list->filename );
      list = list->next;
    }
  }
  printf( "\n" );
}


void printVerboseObjectInfo( char * filename, char * shortName ) {

  
  struct stat statbuf;
  
  int result = lstat( filename, & statbuf );
  
  if ( result != 0 ) {
    
    fprintf( stderr, "Filename '%s' caused an error: ", filename );
    perror( NULL );
    
    exit( 1 );
  }

  
  mode_t mode = statbuf.st_mode;
  
  if ( S_ISLNK( mode ) )
    printf( "l" );
  
  else if ( S_ISDIR( mode ) )
    printf( "d" );
  
  else if ( S_ISCHR( mode ) )
    printf( "c" );
  
  else if ( S_ISBLK( mode ) )
    printf( "b" );
 
  else if ( S_ISFIFO( mode ) )
    printf( "p" );
  
  else if ( S_ISSOCK( mode ) )
    printf( "s" );
  
  else 
    printf( "-" );

  
  if ( mode & S_IRUSR ) 
    printf( "r" );
  
  else
    printf( "-" );
  
  if ( mode & S_IWUSR ) 
    printf( "w" );
  
  else
    printf( "-" );
 
  if ( mode & S_ISUID )
    printf( "s" );
  
  else if ( mode & S_IXUSR ) 
    printf( "x" );
  
  else
    printf( "-" );

  
  if ( mode & S_IRGRP ) 
    printf( "r" );
  
  else
    printf( "-" );
  
  if ( mode & S_IWGRP ) 
    printf( "w" );
  
  else
    printf( "-" );
  
  if ( mode & S_ISGID )
    printf( "s" );
  
  else if ( mode & S_IXGRP ) 
    printf( "x" );
  
  else
    printf( "-" );

  
  if ( mode & S_IROTH ) 
    printf( "r" );
  
  else
    printf( "-" );
  
  if ( mode & S_IWOTH ) 
    printf( "w" );
  
  else
    printf( "-" );
  
  if ( mode & S_ISVTX )
    printf( "t" );
  
  else if ( mode & S_IXOTH ) 
    printf( "x" );
  
  else
    printf( "-" );
  printf( ". " );
  

  printf( "%ld ", statbuf.st_nlink );
  
  struct passwd * pwd = getpwuid( statbuf.st_uid );
  
  if ( pwd == NULL ) {
    printf( "-no_user_name- " );
  } else {
    printf( "%s ", pwd->pw_name );
  }
  
  struct group * grp = getgrgid( statbuf.st_gid );
  
  if ( grp == NULL ) {
    printf( "-no_group_name- " );
  } else {
    printf( "%s ", grp->gr_name );
  }
 
  printf( "%ld ", statbuf.st_size );
  
  struct tm * t = localtime( & statbuf.st_mtime );
  
  printf( "%02d.%02d.%04d %02d:%02d:%02d ", 
	  t->tm_mday, t->tm_mon+1, t->tm_year+1900, 
	  t->tm_hour, t->tm_min, t->tm_sec
        );
  
  printf( "%s\n", shortName );
}


void printObjectInfo( char * filename, bool lFlag ) {

  
  char buf[ 1024 ];
  
  char shortName[ 1024 ];
  
  size_t count = 0;

  
  struct stat statbuf;
  int result = lstat( filename, & statbuf );
  if ( result != 0 ) {
    fprintf( stderr, "Filename '%s' caused an error: ", filename );
    perror( NULL );
    exit( 1 );
  }

  
  if ( S_ISDIR( statbuf.st_mode ) ) {

    
    strcpy( buf, filename );
    
    count = strlen( filename );
    
    if ( buf[ count-1 ] != '/' ) {
      
      buf[ count ] = '/';
      count++;
      
      buf[ count ] = '\0';
    }
    
    printf( "%s:\n", buf );

    
    DIR * dir = opendir( filename );
    
    if ( dir == NULL ) {
      fprintf( stderr, "Cannot open directory '%s': ", filename );
      perror( NULL );
      exit( 1 );
    }

    
    while ( 1 ) { 
      
      struct dirent * dircont = readdir( dir );
      
      if ( dircont == NULL ) 
	break;
      
      strcpy( buf + count, dircont->d_name );
      
      strcpy( shortName, dircont->d_name );
      
      if ( lFlag ) {
	printVerboseObjectInfo( buf, shortName );
      } else 
	
	printf( "%s\n", shortName );
    }
    
    closedir( dir );

  
  } else {

    if ( lFlag ) {
      printVerboseObjectInfo( filename, filename );
   
    } else {
      printf( "%s\n", filename );
    }
  }
}


int main( int argc, char * argv[] ) {

  
  bool lFlag;
  
  struct Node * filenames = NULL;

  
  parseCommandline( argc, argv, & filenames, & lFlag );

 
  struct Node * list = filenames;
  while ( list != NULL ) {
    printObjectInfo( list->filename, lFlag );
    list = list->next;
  }
  
 
  freeNodes( filenames );

  return 0;
}
