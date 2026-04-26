/* malloctest.c - NachOS malloc test, C89, no ConsoleOutput dependency */
#include "syscall.h"

/* ConsoleOutput = 1, hardcoded in case syscall.h doesn't expose it */
#define STDOUT 1

static void print(char *s, int len)
{
    Write(s, len, STDOUT);
}

int main()
{
    char *a;
    char *b;
    char *c;
    int i;
    int ok;

    /* Test 1: basic allocation */
    a = (char *)Malloc(100);
    if (a == 0) { print("FAIL T1: NULL\n", 14); Exit(1); }
    for (i = 0; i < 100; i++) a[i] = 1;
    print("T1 PASS: alloc 100\n", 19);

    /* Test 2: second alloc, no overlap */
    b = (char *)Malloc(200);
    if (b == 0) { print("FAIL T2: NULL\n", 14); Exit(1); }
    for (i = 0; i < 200; i++) b[i] = 2;
    ok = 1;
    for (i = 0; i < 100; i++) if (a[i] != 1) { ok = 0; break; }
    if (!ok) { print("FAIL T2: a corrupted\n", 21); Exit(1); }
    print("T2 PASS: no overlap\n", 20);

    /* Test 3: free then realloc */
    Free(a);
    c = (char *)Malloc(100);
    if (c == 0) { print("FAIL T3: NULL\n", 14); Exit(1); }
    for (i = 0; i < 100; i++) c[i] = 3;
    print("T3 PASS: realloc\n", 17);

    /* Test 4: free(NULL) */
    Free(0);
    print("T4 PASS: free NULL\n", 19);

    Free(b);
    Free(c);
    print("All passed\n", 11);
    Exit(0);
    return 0;
}
