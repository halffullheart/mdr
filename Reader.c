#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getHTML()
{
    const int memChunkSize = 1000;

    char* body;
    int usedLength = 0;
    int allocations = 1;
    body = (char*)malloc(memChunkSize * sizeof(char));

    while(!feof(stdin)) {
        char line[500];
        fgets(line, 500, stdin);
        usedLength += strlen(line) + 1; // Add 1 for null-terminator
        if (usedLength > allocations * memChunkSize)
        {
            allocations++;
            body = (char*)realloc(body, allocations * memChunkSize * sizeof(char));
        }
        strcat(body, line);
    }

    return body;
}
