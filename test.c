#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Reader.h"
#include "bstrlib.h"

void printSeq(seq s)
{
    int i;
    for (i = 0; i < s.alen; i++)
    {
        printf("%i ", s.val[i]);
    }
    printf("\n");
}

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

    // Edit distance.
    bstring t = bfromcstr("Sunday");
    bstring s = bfromcstr("Saturday");
    int dist = editDistance(s, t);
    assert(dist == 3);
    bdestroy(t);
    bdestroy(s);

    // Sequences.
    seq seq1 = initSeq(10);
    setSeq(&seq1, 0, 5);
    assert(seq1.val[0] == 5);
    setSeq(&seq1, 1, 3);
    assert(seq1.val[1] == 3);
    unshiftSeq(&seq1, 8);
    assert(seq1.alen == 3);
    assert(seq1.mlen == 10);
    assert(seq1.val[0] == 8);
    assert(seq1.val[1] == 5);
    assert(seq1.val[2] == 3);
    freeSeq(&seq1);
    assert(seq1.mlen == 0);

    bstring bstr1 = bfromcstr("String");
    seq seq2 = stringToSeq(bstr1);
    assert(seq2.mlen == 6);
    assert(seq2.alen == 6);
    assert(seq2.val[0] == 'S');
    bdestroy(bstr1);
    freeSeq(&seq2);

    seq seq3out = initSeq(6);
    seq seq4out = initSeq(6);
    bstring str3 = bfromcstr("ABCDEF");
    bstring str4 = bfromcstr("ACEX");
    seq seq3in = stringToSeq(str3);
    seq seq4in = stringToSeq(str4);

    determineAlignment(seq3in, seq4in, &compareChars, &seq3out, &seq4out);
    assert(seq3out.alen == seq4out.alen);
    assert(seq3out.val[0] == 'A');
    assert(seq3out.val[1] == 'B');
    assert(seq4out.val[0] == 'A');
    assert(seq4out.val[1] == ALIGN_GAP);
    assert(seq4out.val[2] == 'C');

    highlightMask * mask3 = NULL;
    highlightMask * mask4 = NULL;
    determineLineHighlighting(str3, str4, &mask3, &mask4);
    assert(mask3[0] == MASK_SAME);
    assert(mask3[5] == MASK_DIFFERENT);
//    printf("%i %i %i %i %i %i\n", mask3[0], mask3[1], mask3[2], mask3[3], mask3[4], mask3[5]);

    freeSeq(&seq3out);
    freeSeq(&seq4out);
    bdestroy(str3);
    bdestroy(str4);
    freeSeq(&seq3in);
    freeSeq(&seq4in);
    free(mask3);
    free(mask4);



    /*
    printSeq(seq3in);
    printSeq(seq4in);
    printSeq(seq3out);
    printSeq(seq4out);
    */

    printf("Wheeee!\n");
    return 0;
}
