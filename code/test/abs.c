#include "syscall.h"

int main() {
    OpenFileId in = _ConsoleInput;
    OpenFileId out = _ConsoleOutput;

    char buffer[64];
    int n;

    while ((n = Read(buffer, 64, in)) > 0) {
        Write(buffer, n, out);
    }

    Exit(0);
}