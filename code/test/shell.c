/* shell.c */
#include "syscall.h"

int main()
{
    int  x, y;       /* x = read fd,  y = write fd */
    int  rt, rt2;
    char buf1[] = "Hello World\0";
    char buf2[20];

    rt = Pipe(&x, &y);
    if (rt == -1) {
        Exit(-1);
    }

    rt2 = Exec("child");   /* path to the child binary */

    if (rt2 == 0) {
        /* ---- parent ---- */
        write2(y, buf1, 10);
    } else {
        /* ---- child  ---- */
        read2(x, buf2, 10);
        PrintString(buf2);
    }

    Exit(0);
}
