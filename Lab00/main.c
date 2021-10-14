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


void printVerboseObjectInfo( char * filename ) {

  
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
  
  // Число жестких ссылок на файл
  printf( "%ld ", statbuf.st_nlink );
  // Имя владельца файла
  struct passwd * pwd = getpwuid( statbuf.st_uid );
  printf( "%s ", pwd->pw_name );
  // Имя группы файла
  struct group * grp = getgrgid( statbuf.st_gid );
  printf( "%s ", grp->gr_name );
  // Размер файла в байтах
  printf( "%ld ", statbuf.st_size );
  // Время последнего изменения файла
  // Преобразование времени в структуру типа tm
  struct tm * t = localtime( & statbuf.st_mtime );
  // Вывод даты в формате ДД.ММ.ГГГГ ЧЧ:MM:СС
  printf( "%02d.%02d.%04d %02d:%02d:%02d ", 
	  t->tm_mday, t->tm_mon+1, t->tm_year+1900, 
	  t->tm_hour, t->tm_min, t->tm_sec
        );
  // Имя файла
  printf( "%s\n", filename );
}


void printObjectInfo( char * filename, bool lFlag ) {

  
  char buf[ 1024 ];
  // Использованный размер буфера
  size_t count = 0;

  // Получение данных о файле filename
  struct stat statbuf;
  int result = lstat( filename, & statbuf );
  if ( result != 0 ) {
    fprintf( stderr, "Filename '%s' caused an error: ", filename );
    perror( NULL );
    exit( 1 );
  }

  // Если тип файла - директория
  if ( S_ISDIR( statbuf.st_mode ) ) {

    // Копирование имени директории в буфер
    strcpy( buf, filename );
    // Сохраняем данные об использованной длине буфера
    count = strlen( filename );
    // Если имя директории не заканчивается на '/'
    if ( buf[ count-1 ] != '/' ) {
      // Добавляем символ '/' к буфферу
      buf[ count ] = '/';
      count++;
      // Добавляем символ конца строки
      buf[ count ] = '\0';
    }
    // Вывод имени директории
    printf( "%s:\n", buf );

    // Пытаемся открыть директорию
    DIR * dir = opendir( filename );
    // Если директорию открыть не удалось
    if ( dir == NULL ) {
      fprintf( stderr, "Cannot open directory '%s': ", filename );
      perror( NULL );
      exit( 1 );
    }


    while ( 1 ) { 
      
      struct dirent * dircont = readdir( dir );
      // Если прочитали все записи
      if ( dircont == NULL ) 
	break;
      // Добавляем к буферу имя объекта
      strcpy( buf + count, dircont->d_name );
      // Если необходим подробный вывод информации о файле
      if ( lFlag ) {
	printVerboseObjectInfo( buf );
      } else 
	// Просто вывод имени файла
	printf( "%s\n", buf );
    }
    // Закрытие директории 
    closedir( dir );

  // Тип файла - не директория
  } else {
    // Подробный вывод
    if ( lFlag ) {
      printVerboseObjectInfo( filename );
    // краткий вывод
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
