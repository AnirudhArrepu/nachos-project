#ifndef PCB_H
#define PCB_H

#include "synch.h"      // for Semaphore, Lock (forward is not enough anymore)

#define MAX_FD_ENTRY     20
#define PIPE_BUFFER_SIZE 256

// Entry types
#define FD_EMPTY     0
#define FD_PIPE_RD   1
#define FD_PIPE_WR   2

// One pipe buffer shared between read and write end
struct PipeBuffer {
    char        data[PIPE_BUFFER_SIZE];
    int         readPos;
    int         writePos;
    int         count;
    int         refCount;
    Lock*       lock;
    Semaphore*  dataAvail;
    Semaphore*  spaceAvail;

    PipeBuffer();   // declaration only
    ~PipeBuffer();
};

// One slot in file descriptor table
struct FDEntry {
    int         type;
    PipeBuffer* pipe;
};

class PCB {
private:
    Semaphore *joinsem;
    Semaphore *exitsem;
    Semaphore *multex;
    int exitcode;
    int numwait;
    Thread *thread;
    char filename[128];

public:
    int parentID;
    int processID;

    // ===== NEW: File Descriptor Table for Pipes =====
    FDEntry fd_table[MAX_FD_ENTRY];

    PCB();
    PCB(int id);
    ~PCB();

    int  Exec(char *filename, int pid);
    int  GetID();
    int  GetNumWait();
    void JoinWait();
    void ExitWait();
    void JoinRelease();
    void ExitRelease();
    void IncNumWait();
    void DecNumWait();
    void SetExitCode(int ec);
    int  GetExitCode();
    void SetFileName(char *fn);
    char* GetFileName();

    // ===== NEW: FD management functions =====
    int  AllocFD();
    void FreeFD(int fd);
    bool ValidFD(int fd);
};

#endif // PCB_H
