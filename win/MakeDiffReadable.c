#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../Reader.h"

int main (int argc, const char * argv[])
{

    char * html;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--html") == 0)
        {
            html = getHTML();
            printf("%s", html);
            free(html);
        }
        else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
        {
            printf("%s", getVersion());
        }
        else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            printf("%s", getHelp());
        }
        else
        {
            printf("Unknown arguments.\n%s", getHelp());
        }
        return 0;
    }
    else
    {
        html = getHTML();
    }

    DWORD dwRetVal = 0;
    DWORD dwBytesWritten = 0;
    UINT uRetVal = 0;

    TCHAR lpTempPathBuffer[MAX_PATH];
    TCHAR szTempFileName[MAX_PATH];

    HANDLE hTempFile = INVALID_HANDLE_VALUE;

    dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
    uRetVal = GetTempFileName(lpTempPathBuffer, TEXT("MDR"), 0, szTempFileName);

    snprintf(szTempFileName, MAX_PATH, "%s.hta", szTempFileName);

    hTempFile = CreateFile((LPTSTR) szTempFileName, // file name 
                           GENERIC_WRITE,           // open for write 
                           0,                       // do not share 
                           NULL,                    // default security 
                           CREATE_ALWAYS,           // overwrite existing
                           FILE_ATTRIBUTE_NORMAL,   // normal file 
                           NULL);                   // no template 

    WriteFile(hTempFile, html, strlen(html), &dwBytesWritten, NULL);

    CloseHandle(hTempFile);
    free(html);

    //printf("Temp file name: %s\n", szTempFileName);

    char cmd[100];
    snprintf(cmd, 100, "start %s", szTempFileName);
    system(cmd);

    return 0;
}
