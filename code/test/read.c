#include "syscall.h"

int main() {
    char buf[100];
    int n;

    n = Read(buf, 100, _ConsoleInput);
    if (n > 0) {
        Write(buf, n, _ConsoleOutput);
    }

    Exit(0);
}