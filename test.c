#include <stdio.h>
#include "Reader.h"
#include "bstrlib.h"

int main()
{
    alignStrings(bfromcstr("<head><title>Something else</title></head>"), bfromcstr("<head><title>Title</title></head>"));
    return 0;
}
