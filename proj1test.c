/*
   proj1test.c

   This code can be used to test your layer 5 implementation
    (which in turn should use your layer 4 implementation, etc.)

   Included is a set of functions that can be used as a layer 1
    implementation, and a main program.
*/

#include <stdio.h>	/* printf, fprintf, etc */
#include <unistd.h>   	/* need this for read(), write() */
#include <stdlib.h>	/* needed for exit()  */
#include <string.h>	/* strlen, etc */


/*------------------------------------------------------------------
   Sample layer 1 implementation - this can be used to
   provide half-duplex communication by using the shell to
   create a pipe between a sender process and a receiver process.
*/

/* sample layer1_read just calls read on stdin */
int layer1_read( char * b )
{
  if ( read( 0, b, 1 ) != 1 )
  {
    return -1;
  }
  else
  {
    return 1;
  }
}

/* sample layer1_write just calls write to stdout */
int layer1_write( char b )
{
  if ( write( 1, &b, 1 ) != 1 )
  {
    return -1;
  }
  else
  {
    return 1;
  }
}

/* definition of student */

typedef struct {
  char * firstname;
  char * lastname;
  int rin;
  double gpa;
} student;


/* layer 5 prototypes */

int layer5_write( student * stu );
int layer5_read( student * stu );

/*------------------------------------------------------------------
   Main program. This program can be a sender or a receiver,
    depending on how many command-line arguments are supplied
    when the program is run.
*/

int main( int argc, char *argv[] )
{
  student x;

  /* If there are 4 command-line arguments, then this program will send
     the first (argv[1]) as the "firstname" and the second as the "lastname"
     and the third as the rin and the fourth as gpa.

     It uses layer5_write() to send the structure.

     If there are not 4 command line arguments, the program assumes it should
     be a reader, so it calls layer5_read() to get the named value.
  */

  if ( argc != 5 )
  {

    /* I'm a reader - read the structure */

    if ( layer5_read( &x ) == -1 )
    {
      fprintf( stderr, "Reading error\n" );
      exit( 1 );
    }

    /* print out the record */

    printf( "Name: %s %s\n", x.firstname, x.lastname );
    printf( "RIN: %d\n", x.rin );
    printf( "GPA: %0.3lf\n", x.gpa );

  } else {

    /* I'm a writer (argv == 5) */

    /* make sure strings are not too large */

    if ( strlen( argv[1] ) > 80 )
    {
      fprintf( stderr, "Error - first name is too long\n" );
      return 0;
    }

    if ( strlen( argv[2] ) > 80 )
    {
      fprintf( stderr, "Error - last name is too long\n" );
      return 0;
    }


    /*  the first command-line argument is the firstname, the second
	is the lastname. */

    x.firstname = argv[1];
    x.lastname = argv[2];

    /* argv[3] is the id */

    x.rin = atoi( argv[3] );

    /* argv[4] is the gpa */

    x.gpa = atof( argv[4] );

    /* send the struct */

    if ( layer5_write( &x ) == -1 )
    {
      /* something went wrong when sending */
      fprintf( stderr, "Error sending record\n" );
      exit( 1 );
    }

  }

  return 1;
}
