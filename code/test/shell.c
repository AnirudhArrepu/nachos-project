#include "syscall.h"
#define NULL 0

int main() {
    SpaceId newProc, newProc2;
    OpenFileId input = _ConsoleInput;
    OpenFileId output = _ConsoleOutput;
    char prompt[2], ch, buffer[60];
    char buffer1[60], buffer2[60];
    int i,j,k;

    prompt[0] = '-';
    prompt[1] = '-';

    while (1) {
        Write(prompt, 2, output);

        i = 0;

        do {
            Read(&buffer[i], 1, input);

        } while (buffer[i++] != '\n');

        buffer[--i] = '\0';

        if (i > 0) {
            j=0;
            k=0;
            while(buffer[j]!='|') {buffer1[k++]=buffer[j++];}
            buffer1[k-1]='\0';
            k=0; j++;
            while(buffer[j]!='\0'){
                buffer2[k++]=buffer[j++];
            }
            buffer2[k]='\0';
            PrintString(buffer1);
            PrintString(buffer2);
            Create("pipe.tmp");
            PrintString("creating tmp file");
            newProc = Exec(buffer1, NULL, "pipe.tmp");
            Join(newProc);
            newProc2 = Exec(buffer2, "pipe.tmp", NULL);
            Join(newProc2);
        }
    }
}   
