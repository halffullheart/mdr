#include <stdio.h>
#include <unistd.h>
#include "Reader.h"
#include "bstrlib.h"

int main()
{
    /*
    bstring s = bfromcstr("<head><title>Something else</title></head>");
    bstring t = bfromcstr("<head><title>Title</title></head>");
    bstring result = alignStrings(s, t);
    bdestroy(result);
    bdestroy(s);
    bdestroy(t);
    sleep(2);
    */

    bstring t = bfromcstr("Sunday");
    bstring s = bfromcstr("Saturday");
    int dist = editDistance(s, t);
    printf("Edit distance: %i\n", dist);
    return 0;
}
