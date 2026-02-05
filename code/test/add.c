/* add.c
 *	Simple program to test whether the systemcall interface works.
 *
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main() {
   	int i;
	for(i=0; i<10; i++) PrintString("child process\n");

	Exit(0);

    /* not reached */
}
