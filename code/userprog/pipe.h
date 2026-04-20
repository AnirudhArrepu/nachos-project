#ifndef PIPE_H
#define PIPE_H

#include "synch.h"

#define PIPE_SIZE 256

class Pipe {
public:
    char buffer[PIPE_SIZE];
    int readPos, writePos;

    Semaphore *empty;
    Semaphore *full;
    Lock *lock;

    Pipe();
    int Write(char *data, int size);
    int Read(char *data, int size);
};

#endif
