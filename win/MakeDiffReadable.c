#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../Reader.h"

int main (int argc, const char * argv[])
{

    char * html = getHTML();

    if (argc > 1 && strcmp(argv[1], "--html") == 0)
    {
        printf("HTML Output:\n%s\n", html);
        free(html);
        return 0;
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
