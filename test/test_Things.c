#include <stdlib.h>
#include "unity.h"
#include "mdr.h"
#include "bstrlib.h"

void setUp(void) {}
void tearDown(void) {}

highlightMask * highlightMaskRep(char stringMask[])
{
  size_t length = strlen(stringMask);

  highlightMask * mask = malloc(length * sizeof(highlightMask));
  if (mask == NULL)
  {
      free(mask);
      printf("Memory allocation error.\n");
      exit(-1);
  }

  for (int i = 0; i < length; i++) {
    char c = stringMask[i];
    mask[i] = (c == '-' ? MASK_DIFFERENT : MASK_SAME);
  }

  return mask;
}

void test_EditDistance(void)
{
  bstring s = bfromcstr("Saturday");
  bstring t = bfromcstr("Sunday");

  TEST_ASSERT_EQUAL(3, editDistance(s, t));

  bdestroy(s);
  bdestroy(t);
}

void test_Sequences(void)
{
  seq seq1 = initSeq(10);
  TEST_ASSERT_EQUAL(10, seq1.mlen);
  TEST_ASSERT_EQUAL(0, seq1.alen);

  setSeq(&seq1, 0, 5);
  TEST_ASSERT_EQUAL(5, seq1.val[0]);

  setSeq(&seq1, 1, 3);
  TEST_ASSERT_EQUAL(3, seq1.val[1]);

  unshiftSeq(&seq1, 8);
  TEST_ASSERT_EQUAL(3, seq1.alen);
  TEST_ASSERT_EQUAL(8, seq1.val[0]);
  TEST_ASSERT_EQUAL(5, seq1.val[1]);
  TEST_ASSERT_EQUAL(3, seq1.val[2]);

  freeSeq(&seq1);
  TEST_ASSERT_EQUAL(0, seq1.mlen);


  bstring bstr1 = bfromcstr("String");
  seq seq2 = stringToSeq(bstr1);
  // assert(seq2.mlen == 6);
  TEST_ASSERT_EQUAL(6, seq2.mlen);
  // assert(seq2.alen == 6);
  TEST_ASSERT_EQUAL(6, seq2.alen);
  // assert(seq2.val[0] == 'S');
  TEST_ASSERT_EQUAL('S', seq2.val[0]);
  bdestroy(bstr1);
  freeSeq(&seq2);
}

void test_DetermineAlignmentAndHighlighting(void)
{
  seq seq3out = initSeq(6);
  seq seq4out = initSeq(6);
  bstring str3 = bfromcstr("ABCDEF");
  bstring str4 = bfromcstr("ACEX");
  seq seq3in = stringToSeq(str3);
  seq seq4in = stringToSeq(str4);

  determineAlignment(seq3in, seq4in, &compareChars, &seq3out, &seq4out);
  TEST_ASSERT_EQUAL(seq3out.alen, seq4out.alen);
  TEST_ASSERT_EQUAL('A',          seq3out.val[0]);
  TEST_ASSERT_EQUAL('B',          seq3out.val[1]);
  TEST_ASSERT_EQUAL('A',          seq4out.val[0]);
  TEST_ASSERT_EQUAL(ALIGN_GAP,    seq4out.val[1]);
  TEST_ASSERT_EQUAL('C',          seq4out.val[2]);
  TEST_ASSERT_EQUAL(ALIGN_GAP,    seq4out.val[3]);

  highlightMask * mask3 = NULL;
  highlightMask * mask4 = NULL;
  determineLineHighlighting(str3, str4, &mask3, &mask4);
  TEST_ASSERT_EQUAL(MASK_SAME,      mask3[0]);
  TEST_ASSERT_EQUAL(MASK_DIFFERENT, mask3[1]);
  TEST_ASSERT_EQUAL(MASK_DIFFERENT, mask3[2]);
  TEST_ASSERT_EQUAL(MASK_DIFFERENT, mask3[3]);
  TEST_ASSERT_EQUAL(MASK_DIFFERENT, mask3[4]);
  TEST_ASSERT_EQUAL(MASK_DIFFERENT, mask3[5]);

  freeSeq(&seq3out);
  freeSeq(&seq4out);
  bdestroy(str3);
  bdestroy(str4);
  freeSeq(&seq3in);
  freeSeq(&seq4in);
  free(mask3);
  free(mask4);
}

void test_DetermineHighlighting(void)
{
  bstring          strL         = bfromcstr(         "ABCDEF"  );
  highlightMask *  expectMaskL  = highlightMaskRep(  " -----"  );

  bstring          strR         = bfromcstr(         "ACEX"    );
  highlightMask *  expectMaskR  = highlightMaskRep(  " ---"    );

  highlightMask * maskL;
  highlightMask * maskR;

  determineLineHighlighting(strL, strR, &maskL, &maskR);

  for (int i = 0; i < strL->slen; i++)
  {
    char msg[20];
    sprintf(msg, "Position %i", i);
    TEST_ASSERT_EQUAL_MESSAGE(expectMaskL[i], maskL[i], msg);
  }

  for (int i = 0; i < strR->slen; i++)
  {
    char msg[20];
    sprintf(msg, "Position %i", i);
    TEST_ASSERT_EQUAL_MESSAGE(expectMaskR[i], maskR[i], msg);
  }

  free(maskL);
  free(maskR);
  free(expectMaskL);
  free(expectMaskR);
  bdestroy(strL);
  bdestroy(strR);
}
