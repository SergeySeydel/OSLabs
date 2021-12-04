#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#include<getopt.h>


#define TMP_FILE "._tmp_archive_"


char magicSequence[] = { 0xf9, 0xed, 0xce, 0x83 };


void writeMagicSequence( int fd ) {
  
  lseek( fd, 0L, SEEK_SET );
  ssize_t len = sizeof( magicSequence );
 
  if ( write( fd, magicSequence, len ) != len ) {
    
    perror( NULL );
    
    exit( EXIT_FAILURE );
  }
}


int isMagicSequenceCorrect( int fd ) {
  
  lseek( fd, 0L, SEEK_SET );
  ssize_t len = sizeof( magicSequence );
  
  char * buf = ( char * ) malloc( (size_t) len );
  
  if ( read( fd, buf, len ) != len ) {
    
    return 0;
  }
  
  int result = strncmp( buf, magicSequence, len );
  
  free( buf );
  
  if ( result == 0 )
    
    return 1;
  
  return 0;
}


void usage( int code ) {
  printf( "usage:\n" );
  printf( "1) add file '<file_name>' to archive '<arch_name>':\n" );
  printf( " ./archiver <arch_name> -i <file_name>\n" );
  printf( " ./archiver <arch_name> --input <file_name>\n" );
  printf( "2) extract file '<file_name>' from archive '<arch_name>':\n" );
  printf( " ./archiver <arch_name> -e <file_name>\n" );
  printf( " ./archiver <arch_name> --extract <file_name>\n" );
  printf( "3) print state of archive '<arch_name>':\n" );
  printf( " ./archiver -s <arch_name>\n" );
  printf( " ./archiver --stat <arch_name>\n" );
  printf( "4) remove file '<file_name>' from archive '<arch_name>':\n" );
  printf( " ./archiver <arch_name> -r <file_name>\n" );
  printf( " ./archiver <arch_name> --remove <file_name>\n" );
  printf( "5) print help message':\n" );
  printf( " ./archiver -h\n" );
  printf( " ./archiver --help\n" );
  
  exit( code );
}


enum CommandType { ADD, EXTRACT, STATE, HELP, REMOVE, UNKNOWN };


struct ParsedCL {
  
  enum CommandType command;
 
  char * arch_name;
  
  char * file_name;
};


void parseCommandLine( int argc, char * argv[], struct ParsedCL * cl ) {

  
  cl->command = UNKNOWN;

  
  const struct option long_options[] = {
    { "input",     1,  NULL,   'i'},
    { "extract",   1,  NULL,   'e'},
    { "stat",      1,  NULL,   's'},
    { "remove",    1,  NULL,   'r'},
    { "help",      0,  NULL,   'h'},
    { NULL,   0,        NULL, 0}  
  };
  const char * const short_options = "-i:e:s:r:h";
  
  int next_option;

  
  cl->arch_name = NULL;
  cl->file_name = NULL;

  
  do {
    
    next_option = getopt_long( argc, argv, short_options, 
	                       long_options, NULL);
    
    switch ( next_option ) {
      
      case 'i':
	
	if ( cl->command != UNKNOWN ) {
	  fprintf( stderr, "Bad command line\n" );
	  exit( EXIT_FAILURE );
	}
	cl->command = ADD;
	cl->file_name = optarg;
        break;
      
      case 'e':
	
	if ( cl->command != UNKNOWN ) {
	  fprintf( stderr, "Bad command line\n" );
	  exit( EXIT_FAILURE );
	}
	cl->command = EXTRACT;
	cl->file_name = optarg;
        break;
      
      case 's':
	
	if ( cl->command != UNKNOWN ) {
	  fprintf( stderr, "Bad command line\n" );
	  exit( EXIT_FAILURE );
	}
	cl->command = STATE;
	cl->arch_name = optarg;
        break;
      
      case 'r':
	
	if ( cl->command != UNKNOWN ) {
	  fprintf( stderr, "Bad command line\n" );
	  exit( EXIT_FAILURE );
	}
	cl->command = REMOVE;
	cl->file_name = optarg;
        break;
      
      case 'h':
	
	if ( cl->command != UNKNOWN ) {
	  fprintf( stderr, "Bad command line\n" );
	  exit( EXIT_FAILURE );
	}
	cl->command = HELP;
        break;
      
      case 1:
	
	if ( cl->arch_name == NULL )
	  cl->arch_name = optarg;
	break;
      
      case '?':
	fprintf( stderr, "Bad command line\n" );
	
	usage( EXIT_FAILURE );
	break;
    }

  } while ( next_option != -1 );

  
  if ( cl->command == UNKNOWN ) {
    fprintf( stderr, "Bad command line\n" );
    exit( EXIT_FAILURE );
  }
 
  if ( ( cl->command != HELP ) && ( cl->arch_name == NULL ) ) {
    fprintf( stderr, "Bad command line\n" );
    exit( EXIT_FAILURE );
  }
}


struct Record {
  
  off_t filename;
 
  off_t filename_size;
  
  off_t data;
  
  off_t data_size;
};


off_t sizeOfFile( int fd ) {
  
  off_t cur_pos = lseek( fd, 0L, SEEK_CUR );
  
  off_t len = lseek( fd, 0L, SEEK_END );
  
  lseek( fd, cur_pos, SEEK_SET );
  
  return len;
}


ssize_t writeSizeToFile( int fd, off_t size ) {
  
  unsigned char buf[ 8 ];
  for ( int i = 0; i < 8; i++ ) {
    
    buf[ i ] = size % 256;
    
    size >>= 8;
  }
  
  size_t result = write( fd, buf, 8 );
  
  if ( result != 8 ) {
   
    perror( NULL );
    
    return 0;
  }
  
  return 1;
}


off_t readSizeFromFile( int fd ) {
  
  unsigned char buf[ 8 ];
  
  size_t result = read( fd, buf, 8 );
  
  if ( result != 8 ) {
    
    perror( NULL );
    
    return 0;
  }
  
  off_t size = 0;
  for ( int i = 7; i >=0 ; i-- ) {
    
    size <<= 8;
    
    size += buf[ i ];
  }
  
  if ( size < 0L )
    ы
    return 0;
  
  return size;
}


int readRecord( int fd, struct Record * rec ) {
  
  rec->filename_size = readSizeFromFile( fd );
  
  if ( rec->filename_size < 0 )
    return 0;
  
  rec->filename = lseek( fd, 0L, SEEK_CUR );
 
  if ( rec->filename < 0 )
    return 0;
  
  off_t result = lseek( fd, rec->filename_size, SEEK_CUR );
  
  if ( result < 0 )
    return 0;
  
  rec->data_size = readSizeFromFile( fd );
  
  if ( rec->data_size < 0 )
    return 0;
  
  rec->data = lseek( fd, 0L, SEEK_CUR );
  
  result = lseek( fd, rec->data_size, SEEK_CUR );
  
  if ( result < 0 )
    return 0;
  
  return 1;
}


int isArchiveCorrect( int fd ) {
  
  if ( ! isMagicSequenceCorrect( fd ) ) {
    
    return 0;
  }
  
  off_t file_size = sizeOfFile( fd );
  
  off_t cur_pos;
  
  struct Record rec;
  
  while ( 1 ) {
    
    cur_pos = lseek( fd, 0L, SEEK_CUR );
    
    if ( cur_pos == file_size )
      
      return 1;
    
    int result = readRecord( fd, & rec );
    
    if ( ! result ) {
      
      return 0;
    }
  }
  return 1;
}


int extractFileRecord( int fd, char * filename, struct Record * rec ) {
  
  off_t file_size = sizeOfFile( fd );
  
  off_t cur_pos = lseek( fd, sizeof( magicSequence ), SEEK_SET );
  if ( cur_pos < 0 ) {
    fprintf( stderr, "Cannot reset position in the archive\n" );
    
    close( fd );
   
    exit( EXIT_FAILURE );
  }

  
  while ( 1 ) {
    
    cur_pos = lseek( fd, 0L, SEEK_CUR );
    
    if ( cur_pos == file_size ) 
      
      break;
    
    int result = readRecord( fd, rec );
    
    if ( ! result ) {
      
      fprintf( stderr, "Program need debug!\n" );
      
      close( fd );
      
      exit( EXIT_FAILURE );
    }
   
    if ( rec->filename_size == ( off_t ) strlen( filename ) ) {
      
      char * rec_filename;
      
      rec_filename = ( char * ) malloc( rec->filename_size + 1 );
      
      if ( rec_filename == NULL ) {
	fprintf( stderr, "Not enough memory\n!" );
	
	close( fd );
	
	exit( EXIT_FAILURE );
      }
      
      if ( pread( fd, rec_filename, rec->filename_size, rec->filename ) != rec->filename_size ) {
  	
  	fprintf( stderr, "Program needs debug\n" );
  	
  	close( fd );
  	
  	exit( EXIT_FAILURE );
      }
      
      rec_filename[ rec->filename_size ] = '\0';
      
      if ( strcmp( rec_filename, filename ) == 0 )
	
	return 1;
      
      free( rec_filename );
    }
  }
 
  return 0;
}


void addFileToArchive( struct ParsedCL * cl ) {
  
  int newArchiveFlag = 0;
  
  int fd_file = open( cl->file_name, O_RDONLY );
  
  if ( fd_file == -1 ) {
    
    perror( cl->file_name );
    
    exit( EXIT_FAILURE );
  }
  
  int fd_arch = open( cl->arch_name, O_RDWR );
 
  if ( ( fd_arch == -1 ) && ( errno == ENOENT ) ) {
    
    umask( S_ISUID | S_IXUSR | S_IRGRP | S_ISGID | S_IXGRP | S_IROTH | S_IWOTH | S_ISVTX | S_IXOTH );
    
    fd_arch = open( cl->arch_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    newArchiveFlag = 1;
  }
  
  if ( fd_arch == -1 ) {
    
    perror( cl->arch_name );
    
    exit( EXIT_FAILURE );
  }

  
  if ( newArchiveFlag ) {
    
    writeMagicSequence( fd_arch );
  
  } else {
    
    if ( ! isArchiveCorrect( fd_arch ) ) {
      fprintf( stderr, "The archive file '%s' is not correct!\n", cl->arch_name );
      
      close( fd_arch );
      close( fd_file );
      
      exit( EXIT_FAILURE );
    }
  }

  
  struct Record rec;
  
  if ( extractFileRecord( fd_arch, cl->file_name, & rec ) ) {
    
    fprintf( stderr, "File '%s' is already in the archive\n", cl->file_name );
    
    close( fd_file );
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  lseek( fd_arch, 0L, SEEK_END );
      
  
  off_t name_size = strlen( cl->file_name );
 
  ssize_t result = writeSizeToFile( fd_arch, name_size );
 
  if ( ! result ) {
    fprintf( stderr, "Cannot write to the archive\n" );
    
    close( fd_arch );
    close( fd_file );
    
    exit( EXIT_FAILURE );
  }
 
  result = write( fd_arch, cl->file_name, name_size );
  if ( result != name_size ) {
    fprintf( stderr, "Cannot write to the archive\n" );
   
    close( fd_arch );
    close( fd_file );
   
    exit( EXIT_FAILURE );
  }
  
  off_t file_size = sizeOfFile( fd_file );
  
  result = writeSizeToFile( fd_arch, file_size );
  
  if ( ! result ) {
    fprintf( stderr, "Cannot write to the archive\n" );
    
    close( fd_arch );
    close( fd_file );
    
    exit( EXIT_FAILURE );
  }
  
  char buf[ 1024 ];
  
  while ( 1 ) {
    
    ssize_t s;
    if ( file_size > 1024 )
      s = 1024;
    else
      s = file_size;
    
    ssize_t num = read( fd_file, buf, s );
    
    if ( num != s ) {
      fprintf( stderr, "Cannot read from file\n" );
      
      close( fd_arch );
      close( fd_file );
      
      exit( EXIT_FAILURE );
    }
    
    num = write( fd_arch, buf, s );
    
    if ( num != s ) {
      fprintf( stderr, "Cannot write to the archive\n" );
      
      close( fd_arch );
      close( fd_file );
      
      exit( EXIT_FAILURE );
    }
    
    file_size -= s;
    
    if ( file_size == 0 )
     
      break;
  }

 
  close( fd_file );
  close( fd_arch );
}


void extractFileFromArchive( struct ParsedCL * cl ) {
  
  umask( S_ISUID | S_IXUSR | S_IRGRP | S_ISGID | S_IXGRP | S_IROTH | S_IWOTH | S_ISVTX | S_IXOTH );
  
  int fd_arch = open( cl->arch_name, O_RDONLY );
  
  if ( fd_arch == -1 ) {
    
    perror( cl->arch_name );
    
    exit( EXIT_FAILURE );
  }

 
  if ( ! isArchiveCorrect( fd_arch ) ) {
    fprintf( stderr, "The archive file '%s' is not correct!\n", cl->arch_name );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  off_t cur_pos = lseek( fd_arch, sizeof( magicSequence ), SEEK_SET );
  if ( cur_pos < 0 ) {
    fprintf( stderr, "Cannot reset position in the archive\n" );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  struct Record rec;
  
  if ( ! extractFileRecord( fd_arch, cl->file_name, & rec ) ) {
    
    fprintf( stderr, "File '%s' is not in the archive\n", cl->file_name );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  int fd_file = open( cl->file_name, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );  
  
  if ( fd_file == -1 ) {
    
    perror( cl->file_name );
   
    exit( EXIT_FAILURE );
  }
  
  off_t file_size = rec.data_size;
  
  lseek( fd_arch, rec.data, SEEK_SET );
      
  
  char buf[ 1024 ];
  
  while ( 1 ) {
    
    ssize_t s;
    if ( file_size > 1024 )
      s = 1024;
    else
      s = file_size;
   
    ssize_t num = read( fd_arch, buf, s );
    
    if ( num != s ) {
      fprintf( stderr, "Cannot read from archive\n" );
      
      close( fd_arch );
      close( fd_file );
      
      exit( EXIT_FAILURE );
    }
    
    num = write( fd_file, buf, s );
   
    if ( num != s ) {
      fprintf( stderr, "Cannot write to the file\n" );
      
      close( fd_arch );
      close( fd_file );
      
      exit( EXIT_FAILURE );
    }
    
    file_size -= s;
    
    if ( file_size == 0 )
      
      break;
  }

  
  close( fd_file );
  close( fd_arch );
}


void removeFileFromArchive( struct ParsedCL * cl ) {
  
  umask( S_ISUID | S_IXUSR | S_IRGRP | S_ISGID | S_IXGRP | S_IROTH | S_IWOTH | S_ISVTX | S_IXOTH );
  
  int fd_arch = open( cl->arch_name, O_RDONLY );
  
  if ( fd_arch == -1 ) {
    
    perror( cl->arch_name );
    
    exit( EXIT_FAILURE );
  }

 
  if ( ! isArchiveCorrect( fd_arch ) ) {
    fprintf( stderr, "The archive file '%s' is not correct!\n", cl->arch_name );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  off_t cur_pos = lseek( fd_arch, sizeof( magicSequence ), SEEK_SET );
  if ( cur_pos < 0 ) {
    fprintf( stderr, "Cannot reset position in the archive\n" );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  struct Record rec;
  
  if ( ! extractFileRecord( fd_arch, cl->file_name, & rec ) ) {
    
    fprintf( stderr, "File '%s' is not in the archive\n", cl->file_name );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  

 
  off_t file_size = sizeOfFile( fd_arch );

  
  cur_pos = lseek( fd_arch, sizeof( magicSequence ), SEEK_SET );
  if ( cur_pos < 0 ) {
    fprintf( stderr, "Cannot reset position in the archive\n" );
    
    close( fd_arch );
   
    exit( EXIT_FAILURE );
  }

 
  int fd_tmp_arch = open( TMP_FILE, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
 
  if ( fd_tmp_arch == -1 ) {
    
    perror( TMP_FILE );
   
    exit( EXIT_FAILURE );
  }

  
  writeMagicSequence( fd_tmp_arch );

  
  while ( 1 ) {
    
    if ( lseek( fd_arch, 0L, SEEK_CUR ) == file_size )
      
      break;

    
    int result = readRecord( fd_arch, & rec );
    
    if ( ! result ) {
      
      fprintf( stderr, "Program need debug!\n" );
      
      close( fd_arch );
      close( fd_tmp_arch );
      
      unlink( TMP_FILE );
     
      exit( EXIT_FAILURE );
    }

    
    int notSpecifiedFile = 1;

    
    if ( rec.filename_size == ( off_t ) strlen( cl->file_name ) ) {
     
      char * rec_filename;
     
      rec_filename = ( char * ) malloc( rec.filename_size + 1 );
      
      if ( rec_filename == NULL ) {
	fprintf( stderr, "Not enough memory\n!" );
	
	close( fd_arch );
	close( fd_tmp_arch );
	
	unlink( TMP_FILE );
	
	exit( EXIT_FAILURE );
      }
     
      if ( pread( fd_arch, rec_filename, rec.filename_size, rec.filename ) != rec.filename_size ) {
  	
  	fprintf( stderr, "Program needs debug\n" );
	
	close( fd_arch );
	close( fd_tmp_arch );
	
	unlink( TMP_FILE );
  	
  	exit( EXIT_FAILURE );
      }
      
      rec_filename[ rec.filename_size ] = '\0';
    
      if ( strcmp( rec_filename, cl->file_name ) == 0 )
	
	notSpecifiedFile = 0;
      
      free( rec_filename );
    } 

    
    if ( notSpecifiedFile ) {
     
      lseek( fd_arch, rec.filename-8L, SEEK_SET );

      
      ssize_t data_size = 16L + rec.filename_size + rec.data_size;
      
      
      char buf[ 1024 ];
      
      while ( 1 ) {
	
	ssize_t s;
	if ( data_size > 1024 )
	  s = 1024;
	else
	  s = data_size;
	
	ssize_t num = read( fd_arch, buf, s );
	
	if ( num != s ) {
	  fprintf( stderr, "Cannot read from archive\n" );
	  
	  close( fd_arch );
	  close( fd_tmp_arch );
	  
	  unlink( TMP_FILE  );
          
	  exit( EXIT_FAILURE );
	}
        
        num = write( fd_tmp_arch, buf, s );
        
        if ( num != s ) {
          fprintf( stderr, "Cannot write to the tmp archive\n" );
	  
	  close( fd_arch );
	  close( fd_tmp_arch );
	  
	  unlink( TMP_FILE  );
          
	  exit( EXIT_FAILURE );
        }
        
        data_size -= s;
        
        if ( data_size == 0 )
          
          break;
      }
    }
  }

  
  close( fd_arch );
  close( fd_tmp_arch );

  
  if ( unlink( cl->arch_name ) == -1 ) {
    
    perror( cl->arch_name );
    
    exit( EXIT_FAILURE );
  }
  
  if ( rename( TMP_FILE, cl->arch_name ) == -1 ) {
   
    perror( TMP_FILE );
    
    exit( EXIT_FAILURE );
  }
}


void printStateOfArchive( struct ParsedCL * cl ) {
  
  int fd_arch = open( cl->arch_name, O_RDONLY );
  
  if ( fd_arch == -1 ) {
    
    perror( cl->arch_name );
    
    exit( EXIT_FAILURE );
  }

  
  if ( ! isArchiveCorrect( fd_arch ) ) {
    fprintf( stderr, "The archive file '%s' is not correct!\n", cl->arch_name );
   
    close( fd_arch );
   
    exit( EXIT_FAILURE );
  }

  
  off_t file_size = sizeOfFile( fd_arch );

 
  off_t cur_pos = lseek( fd_arch, sizeof( magicSequence ), SEEK_SET );
  if ( cur_pos < 0 ) {
    fprintf( stderr, "Cannot reset position in the archive\n" );
    
    close( fd_arch );
    
    exit( EXIT_FAILURE );
  }

  
  struct Record rec;

  
  while ( 1 ) {
    
    cur_pos = lseek( fd_arch, 0L, SEEK_CUR );
    
    if ( cur_pos == file_size ) 
    
      break;
   
    int result = readRecord( fd_arch, & rec );
    
    if ( ! result ) {
      
      fprintf( stderr, "Program need debug!\n" );
      
      close( fd_arch );
      
      exit( EXIT_FAILURE );
    }
    
    char * filename;
    
    filename = ( char * ) malloc( rec.filename_size + 1 );
    
    if ( filename == NULL ) {
      fprintf( stderr, "Not enough memory\n!" );
      
      close( fd_arch );
      
      exit( EXIT_FAILURE );
    }
    
    if ( pread( fd_arch, filename, rec.filename_size, rec.filename ) != rec.filename_size ) {
      
      fprintf( stderr, "Program needs debug\n" );
     
      close( fd_arch );
    
      exit( EXIT_FAILURE );
    }
    
    filename[ rec.filename_size ] = '\0';
    
    printf( "file '%s' with size %ld bytes\n", filename, rec.data_size );
    
    free( filename );
  }

 
  close( fd_arch );
}


void print_off( off_t o ) {
  off_t t = 1;
  for ( int i = 0; i < 8*8; i++ ) {
    if ( o & t )
      printf( "1" );
    else
      printf( "0" );
    t <<= 1;
  }
  printf( "\n" );
}

int main( int argc, char * argv[] ) {

  
  struct ParsedCL cl;
  
  parseCommandLine( argc, argv, & cl );

  
  switch ( cl.command ) {
    
    case ADD:
      addFileToArchive( & cl );
      break;
    
    case STATE:
      printStateOfArchive( & cl );
      break;
    
    case EXTRACT:
      extractFileFromArchive( & cl );
      break;
    
    case REMOVE:
      removeFileFromArchive( & cl );
      break;
    
    case HELP:
      usage( EXIT_SUCCESS );
      break;
   
    default:
      fprintf( stderr, "Your program need debug\n" );
      exit( EXIT_FAILURE );
      break;
  }

  return 0;
}
