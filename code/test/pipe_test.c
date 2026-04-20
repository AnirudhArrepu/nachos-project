#include "syscall.h"

int main() {
    int r, w;
    char msg[] = "Hello World";
    char buf[20];

    Pipe(&r, &w);

    if (Fork() == 0) {
        Read2(r, buf, 11);
        PrintString(buf);
    } else {
        Write2(w, msg, 11);
    }

    Exit(0);
}
