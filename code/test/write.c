#include "syscall.h"

int main() {
    OpenFileId output = _ConsoleOutput;
    char msg[2];
    msg[0] = 'h';
    msg[1] = 'e';


    Write(msg, 2, output);
    Exit(0);
}