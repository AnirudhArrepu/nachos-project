/* exec.c
 *	Simple program to test the exec system call.
 */

#include "syscall.h"
#define stdin 0
#define stdout 1

int main() {
    int pid, pid2, i;
    pid = Exec("../test/add");
    pid2 = Exec("../test/abs");
    if (pid < 0) {
        Write("Exec failed: ", 14, stdout);
        PrintNum(pid);
    } else{
	for(i=0; i<10; i++) PrintString("parent process\n");
	
    }
        Join(pid);
	Join(pid2);
}
