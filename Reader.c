#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* addToString(char* existing, char* new, int* size, int growByIfNeeded);

char* getHTML()
{
    const int memChunkSize = 200;

    char* body;
    int bodylen = memChunkSize;
    body = (char*)malloc(memChunkSize * sizeof(char));

    while(!feof(stdin)) {
        char line[memChunkSize];
        fgets(line, memChunkSize, stdin);
        body = addToString(body, line, &bodylen, memChunkSize);
    }

    return body;
}

char* addToString(char* existing, char* new, int* size, int growByIfNeeded)
{
    if (strlen(existing) + strlen(new) + 1 > *size) // Add 1 to account for null-terminator
    {
        existing = (char*)realloc(existing, (*size + growByIfNeeded) * sizeof(char));
        *size += growByIfNeeded;
    }
    strcat(existing, new);
    return existing;
}
