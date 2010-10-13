#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstrlib.h"

#define CHUNK_SIZE 1000

char* newString(int* size);
char* addToString(char* existing, char* new, int* size);

char* getHTML()
{
    // Read in lines
    bstring stdinContents = bread ((bNread) fread, stdin);
    if (stdinContents == NULL) return "There was an error reading from stdin.";
    char* body = bstr2cstr(stdinContents, '-');
    bdestroy(stdinContents);

    return body;
}
