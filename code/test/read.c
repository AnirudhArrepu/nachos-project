#include "syscall.h"

int main() {
    OpenFileId out = _ConsoleOutput;
    char msg[] = "hello from producer\n";
    int i;

    for(i = 0; i < 5; i++) {
        Write(msg, 20, out);
    }

    Exit(0);
}