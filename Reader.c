#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 500

char* newString(int* size);
char* addToString(char* existing, char* new, int* size);

char* getHTML()
{
    int bodyLen = 0;
    char* body = newString(&bodyLen);

    while(!feof(stdin)) {
        char line[CHUNK_SIZE];
        fgets(line, CHUNK_SIZE, stdin);
        body = addToString(body, line, &bodyLen);
    }

    return body;
}

char* newString(int* size)
{
    *size = CHUNK_SIZE;
    return (char*)malloc(CHUNK_SIZE * sizeof(char));
}

char* addToString(char* existing, char* new, int* size)
{
    if (strlen(existing) + strlen(new) + 1 > *size) // Add 1 to account for null-terminator
    {
        existing = (char*)realloc(existing, (*size + CHUNK_SIZE) * sizeof(char));
        *size += CHUNK_SIZE;
    }
    strcat(existing, new);
    return existing;
}
