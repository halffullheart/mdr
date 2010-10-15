#include "Reader.h"
#include "bstrlib.h"

int main()
{
    alignStrings(bfromcstr("ACGGTAG"), bfromcstr("CCTAAG"));
    return 0;
}
