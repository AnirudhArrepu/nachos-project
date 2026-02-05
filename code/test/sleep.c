#include <syscall.h>

int main(){
	PrintString("before sleeping");
	Sleep(5);
	PrintString("after sleeping");
}
