#include "pipe.h"

Pipe::Pipe() {
    readPos = 0;
    writePos = 0;
    empty = new Semaphore("empty", PIPE_SIZE);
    full = new Semaphore("full", 0);
    lock = new Lock("lock");
}

int Pipe::Write(char *data, int size) {
    for (int i = 0; i < size; i++) {
        empty->P();
        lock->Acquire();

        buffer[writePos] = data[i];
        writePos = (writePos + 1) % PIPE_SIZE;

        lock->Release();
        full->V();
    }
    return size;
}

int Pipe::Read(char *data, int size) {
    for (int i = 0; i < size; i++) {
        full->P();
        lock->Acquire();

        data[i] = buffer[readPos];
        readPos = (readPos + 1) % PIPE_SIZE;

        lock->Release();
        empty->V();
    }
    return size;
}
