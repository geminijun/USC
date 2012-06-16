/* p2p2t1.c
 *	Simple program to test whether running a user program works.
 */

#include "syscall.h"
int a[3];
int b, c;

void ayush();

int main()
{
    /*Exec("ayush", 5);*/
    /* not reached */
	Fork(&ayush);
	Exit(0);
}

void ayush()
{
	OpenFileId fd;
  int bytesread;
  char buf[20];

    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);
}
