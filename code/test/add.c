#include "syscall.h"

int main() {
    OpenFileId out = _ConsoleOutput;
    char msg[] = "helloproducer";
    int i;

    for(i = 0; i < 2; i++) {
        Write(msg, 20, out);
    }

    Exit(0);
}
