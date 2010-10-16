#include <stdio.h>
#include <unistd.h>
#include "Reader.h"
#include "bstrlib.h"

int main()
{
    bstring s = bfromcstr("<head><title>Something else</title></head>");
    bstring t = bfromcstr("<head><title>Title</title></head>");
    bstring result = alignStrings(s, t);
    bdestroy(result);
    bdestroy(s);
    bdestroy(t);
    sleep(2);
    return 0;
}
