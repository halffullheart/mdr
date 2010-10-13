#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstrlib.h"

#define CHUNK_SIZE 1000

char* newString(int* size);
char* addToString(char* existing, char* new, int* size);

char* getHTML()
{
    int bodyLen = 0;
    char* body = newString(&bodyLen);

    // Left file
    int lenL = 0;
    int indL = CHUNK_SIZE;
    char** fileL;

    // Right file
    int lenR = 0;
    int indR = 0;
    char** fileR = NULL;

    // Read in lines
    while(!feof(stdin)) {
        char line[CHUNK_SIZE];
        fgets(line, CHUNK_SIZE, stdin);
        if (line[0] == '-')
        {
            fileL[indL] = line;
            indL++;
        }
        /*
        else if (line[0] == '+')
        {
            indR = addStringToArray(fileR, line, &lenR, indR);
        }
        else
        {
            indL = addStringToArray(fileL, line, &lenL, indL);
            indR = addStringToArray(fileR, line, &lenR, indR);
        }
        */
    }

    // Create output
    /*
    for (int i; i < linesL; i++) {
    }
    */

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
    strcat(existing, new); //TODO: change to strncat?
    return existing;
}

int addStringToArray(char** array, char* string, int* arraySize, int arrayCurrentIndex)
{
    if (arrayCurrentIndex == *arraySize)
    {
        // Feel free to change the initial number of refs and the rate at which
        // refs are allocated.
        if (*arraySize == 0)
        {
            *arraySize = 3; // Start off with 3 refs
        }
        else
        {
            *arraySize *= 2; // Double the number
        }

        // Make the reallocation transactional by using a temporary variable
        // first
        void* _tmp = realloc(array, (*arraySize * sizeof(char*)));

        // If the reallocation didn't go so well, inform the user and bail out
        if (!_tmp)
        {
            fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
            return(-1);
        }

        // Things are looking good so far
        array = (char**)_tmp;
    }

    array[arrayCurrentIndex] = string;
    arrayCurrentIndex++;

    return arrayCurrentIndex;
}

