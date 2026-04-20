#include "copyright.h"
#include "main.h"       // must be before pcb.h because we use kernel
#include "pcb.h"
#include <string.h>     // for memset and strcpy

// PipeBuffer constructor & destructor
PipeBuffer::PipeBuffer() {
    memset(data, 0, sizeof(data));
    readPos   = 0;
    writePos  = 0;
    count     = 0;
    refCount  = 2;
    lock      = new Lock("PipeLock");
    dataAvail = new Semaphore("PipeDataAvail", 0);
    spaceAvail= new Semaphore("PipeSpaceAvail", PIPE_BUFFER_SIZE);
}

PipeBuffer::~PipeBuffer() {
    delete lock;
    delete dataAvail;
    delete spaceAvail;
}

// PCB constructor (the one with parameter)
PCB::PCB(int id) {
    processID = id;                    // better than kernel->currentThread->processID here
    parentID  = -1;
    joinsem   = new Semaphore("joinsem", 0);
    exitsem   = new Semaphore("exitsem", 0);
    multex    = new Semaphore("multex", 1);

    // Initialize fd_table
    for (int i = 0; i < MAX_FD_ENTRY; i++) {
        fd_table[i].type = FD_EMPTY;
        fd_table[i].pipe = NULL;
    }

    thread = NULL;
    numwait = 0;
    exitcode = 0;
    filename[0] = '\0';
}

// Default constructor (if used somewhere)
PCB::~PCB() {
    delete joinsem;
    delete exitsem;
    delete multex;

    if (thread) {
        thread->FreeSpace();
        thread->Finish();
        // delete thread;
    }
    for (int i = 0; i < MAX_FD_ENTRY; i++) {
        if (fd_table[i].type != FD_EMPTY)
            FreeFD(i);
    }

    delete[] filename;
}

int PCB::AllocFD() {
    // Start from 3 to skip stdin/stdout/stderr
    for (int i = 3; i < MAX_FD_ENTRY; i++) {
        if (fd_table[i].type == FD_EMPTY)
            return i;
    }
    return -1;   // table full
}

void PCB::FreeFD(int fd) {
    if (!ValidFD(fd)) return;

    PipeBuffer* pipe = fd_table[fd].pipe;
    fd_table[fd].type = FD_EMPTY;
    fd_table[fd].pipe = NULL;

    if (pipe != NULL) {
        pipe->refCount--;
        if (pipe->refCount <= 0)
            delete pipe;   // last reference gone → free the buffer
    }
}

bool PCB::ValidFD(int fd) {
    return (fd >= 0 && fd < MAX_FD_ENTRY && fd_table[fd].type != FD_EMPTY);
}

void StartProcess_2(void* pid) {
    int id;
    id = *((int*)pid);
    // Lay fileName cua process id nay
    char* fileName = kernel->pTab->GetFileName(id);

    AddrSpace* space;
    space = new AddrSpace(fileName);

    if (space == NULL) {
        printf("\nPCB::Exec: Can't create AddSpace.");
        return;
    }

    space->Execute();
    // kernel->currentThread->space = space;

    // space->InitRegisters();	// set the initial register values
    // space->RestoreState();	// load page table register

    // kernel->machine->Run();	// jump to the user progam
    ASSERT(FALSE);  // machine->Run never returns;
                    // the address space exits
                    // by doing the syscall "exit"
}

int PCB::Exec(char* filename, int id) {
    // cerr << filename << ' ' << pid << endl;
    multex->P();

    this->thread = new Thread(filename, true);
    if (this->thread == NULL) {
        printf("\nPCB::Exec: Not enough memory!\n");
        multex->V();  // Nha CPU de nhuong CPU cho tien trinh khac
        return -1;    // Tra ve -1 neu that bai
    }

    //  Đặt processID của thread này là id.
    this->thread->processID = id;
    // Đặt parrentID của thread này là processID của thread gọi thực thi Exec
    this->parentID = kernel->currentThread->processID;
    // Gọi thực thi Fork(StartProcess_2,id) => Ta cast thread thành kiểu int,
    // sau đó khi xử ký hàm StartProcess ta cast Thread về đúng kiểu của nó.

    // Không được sử dụng biến id ở đây, vì biến id là biến cục bộ,
    // nên khi hàm này kết thúc thì giá trị của biến này cũng bị xóa
    // Đừng hỏi tôi đã mất bao lâu để nhận ra điều này :)
    this->thread->Fork(StartProcess_2, &this->thread->processID);
	kernel->currentThread->Yield();
    multex->V();
    // Trả về id.
    return id;
}

int PCB::GetID() { return thread->processID; }

int PCB::GetNumWait() { return numwait; }

void PCB::JoinWait() {
    // Gọi joinsem->P() để tiến trình chuyển sang trạng thái block và ngừng lại,
    // chờ JoinRelease để thực hiện tiếp.
    joinsem->P();
}

void PCB::ExitWait() {
    // Gọi exitsem-->V() để tiến trình chuyển sang trạng thái block và ngừng
    // lại, chờ ExitReleaseđể thực hiện tiếp.
    exitsem->P();
}

void PCB::JoinRelease() {
    // Gọi joinsem->V() để giải phóng tiến trình gọi JoinWait().
    joinsem->V();
}

void PCB::ExitRelease() {
    // Gọi exitsem->V() để giải phóng tiến trình đang chờ.
    exitsem->V();
}

void PCB::IncNumWait() {
    multex->P();
    ++numwait;
    multex->V();
}

void PCB::DecNumWait() {
    multex->P();
    if (numwait > 0) --numwait;
    multex->V();
}

void PCB::SetExitCode(int ec) { exitcode = ec; }

int PCB::GetExitCode() { return exitcode; }

void PCB::SetFileName(char* fn) { strcpy(filename, fn); }

char* PCB::GetFileName() {
    // cerr << "get file name" << ' ' << filename << endl;
    return filename;
}
