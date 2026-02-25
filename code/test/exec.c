/* exec.c
 *	Simple program to test the exec system call.
 */

#include "syscall.h"
#define stdin 0
#define stdout 1

int main() {
    int pid, pid2, i;
    pid = Exec("../test/add", '\0', '\0');
    pid2 = Exec("../test/abs", '\0', '\0');
    if (pid < 0) {
        Write("Exec failed: ", 14, stdout);
        PrintNum(pid);
    } else{
	for(i=0; i<10; i++) PrintString("Pparent process\n");
	
    }
        Join(pid);
	Join(pid2);
}
