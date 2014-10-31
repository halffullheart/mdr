#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mdr.h"
#include "version.h"
#include "bstrlib.h"
#include "style.css.h"

char * getVersion()
{
    return MDR_APP_NAME " " MDR_VERSION_STR "\n";
}

char * getHelp()
{
    return "Usage: mdr [options]\n"
           "Options:\n"
           "  -h, --help         Display this help message.\n"
           "  -v, --version      Display version of mdr.\n"
           "mdr accepts standard input. Examples:\n"
           "  mdr < changes.diff\n"
           "  hg diff | mdr\n";
}

char * getHtmlFromStdIn()
{
    bstring diffContents = getStdInContents();
    char * html = getHtmlFromDiff(diffContents);
    bdestroy(diffContents);
    return html;
}

bstring getStdInContents()
{
    // Read from stdin
    return bread((bNread)fread, stdin);
}

char * getHtmlFromDiff(bstring diffContents)
{
    bstring html = bfromcstr("<!DOCTYPE html>\n<html>\n");
    balloc(html, style_css_len);
    bcatcstr(html, "<head>");
    bcatcstr(html, "<title>mdr</title>");
    bcatcstr(html, "<style type='text/css'>");
    bcatcstr(html, (char *)style_css);
    bcatcstr(html, "</style>");
    bcatcstr(html, "</head>");
    bcatcstr(html, "<body>\n<table cellpadding='0'>\n");

    // Split into lines
    struct bstrList * inputLines;
    if ((inputLines = bsplit(diffContents, '\n')) != NULL)
    {

        // We are going to build a map showing which lines in the input belong
        // in which lines in the output and how they should be displayed. We'll
        // allocate the left and right maps to be big enough to each hold all
        // the input data, which is more than enough.
        lineData * lineMapL = malloc(inputLines->qty * sizeof(lineData));
        if (lineMapL == NULL)
        {
            free(lineMapL);
            printf("Memory allocation error.\n");
            exit(-1);
        }
        lineData * lineMapR = malloc(inputLines->qty * sizeof(lineData));
        if (lineMapR == NULL)
        {
            free(lineMapR);
            printf("Memory allocation error.\n");
            exit(-1);
        }
        int lineMapPosL = 0;
        int lineMapPosR = 0;

        int useL;
        int useR;
        enum lineType type;
        int padding;
        int lineNoL = 0;
        int lineNoR = 0;
        int firstInfoLine = TRUE;
        int startNewFileOk = TRUE;
        int startOldFileOk = TRUE;

        // Map input lines to their output column (left, right, or both)
        int i;
        for (i = 0; i < inputLines->qty; i++) {

            useL = 0;
            useR = 0;
            type = SHARED;
            padding = 1;

            if (startOldFileOk && stringStartsWith(inputLines->entry[i], "---"))
            {
                type = OLD_FILE;
                useL = 1;
                padding = 4;
                lineNoL = -1;
                lineNoR = -1;
                startOldFileOk = FALSE;
            }
            else if (startNewFileOk && stringStartsWith(inputLines->entry[i], "+++"))
            {
                type = NEW_FILE;
                useR = 1;
                padding = 4;
                lineNoL = -1;
                lineNoR = -1;
                startNewFileOk = FALSE;
            }
            else if (stringStartsWith(inputLines->entry[i], "@@"))
            {
                syncLineNumbers(inputLines->entry[i], &lineNoL, &lineNoR);
                if (firstInfoLine)
                {
                    // Don't print the info row but still increment the line
                    // numbers normally.
                    // TODO: Might be better to mark the row as the first and
                    // hide it with CSS instead of just not printing it.
                    lineNoL++;
                    lineNoR++;
                }
                else
                {
                    type = INFO;
                    useR = 1;
                    useL = 1;
                    padding = 1;
                }
                firstInfoLine = FALSE;
            }
            else if (bdata(inputLines->entry[i])[0] == '-')
            {
                type = OLD;
                useL = 1;
            }
            else if (bdata(inputLines->entry[i])[0] == '+')
            {
                type = NEW;
                useR = 1;
            }
            else if (bdata(inputLines->entry[i])[0] == ' ')
            {
                type = SHARED;
                useL = 1;
                useR = 1;
            }
            else
            {
                type = HEADER;
                lineNoL = 0;
                lineNoR = 0;
                firstInfoLine = TRUE;
                startNewFileOk = TRUE;
                startOldFileOk = TRUE;
            }

            // Balance.
            if (type == HEADER ||
                (type == SHARED && (useL || useR)) ||
                i == inputLines->qty - 1)
            {
                int difference = lineMapPosL - lineMapPosR;
                int j;

                if (difference > 0)
                {
                    for (j = 0; j < difference; j++)
                    {
                        lineMapR[lineMapPosR].type = EMPTY;
                        lineMapPosR++;
                    }
                }
                else if (difference < 0)
                {
                    for (j = 0; j < (difference * -1); j++)
                    {
                        lineMapL[lineMapPosL].type = EMPTY;
                        lineMapPosL++;
                    }
                }
            }

            if (useL)
            {
                lineMapL[lineMapPosL].inputPos = i;
                lineMapL[lineMapPosL].type = type;
                lineMapL[lineMapPosL].padding = padding;
                lineMapL[lineMapPosL].lineNo = lineNoL - 1;
                lineMapL[lineMapPosL].leadingSpaces = 0;
                lineMapPosL++;
                lineNoL++;
            }

            if (useR)
            {
                lineMapR[lineMapPosR].inputPos = i;
                lineMapR[lineMapPosR].type = type;
                lineMapR[lineMapPosR].padding = padding;
                lineMapR[lineMapPosR].lineNo = lineNoR - 1;
                lineMapR[lineMapPosR].leadingSpaces = 0;
                lineMapPosR++;
                lineNoR++;
            }

        }

        // Mapping complete. Quick sanity check that both L and R cols have the
        // same length.
        if (lineMapPosL != lineMapPosR)
        {
            return "Error displaying diff (generated columns not equal in length).";
        }

        // Now we do the formatting work based on the map.
        for (i = 0; i < lineMapPosL; i++)
        {
            highlightMask * highlightMaskA = NULL;
            highlightMask * highlightMaskB = NULL;
            bstring contentL;
            bstring contentR;
            int leadingSpacesL = 0;
            int leadingSpacesR = 0;

            if (lineMapL[i].type != EMPTY)
            {
                contentL = getContentFromLine(
                    inputLines->entry[lineMapL[i].inputPos],
                    lineMapL[i].padding,
                    &leadingSpacesL
                );
                lineMapL[i].leadingSpaces = leadingSpacesL;
            }

            if (lineMapR[i].type != EMPTY)
            {
                contentR = getContentFromLine(
                    inputLines->entry[lineMapR[i].inputPos],
                    lineMapR[i].padding,
                    &leadingSpacesR
                );
                lineMapR[i].leadingSpaces = leadingSpacesR;
            }

            // Compare changed lines
            if (lineMapL[i].type == OLD && lineMapR[i].type == NEW) {

                lineMapL[i].type = CHANGE;
                lineMapR[i].type = CHANGE;

                determineLineHighlighting(
                    contentL,
                    contentR,
                    &highlightMaskA,
                    &highlightMaskB
                );

            }

            // Format output
            bcatcstr(html, "<tr>\n");

            if (lineMapL[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(LEFT, html, contentL, lineMapL[i], highlightMaskA);
                bdestroy(contentL);
            }

            if (lineMapR[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(RIGHT, html, contentR, lineMapR[i], highlightMaskB);
                bdestroy(contentR);
            }

            bcatcstr(html, "</tr>\n");

            free(highlightMaskA);
            free(highlightMaskB);
        }

        bcatcstr(html, "</table>\n</body>\n</html>\n");

        free(lineMapL);
        free(lineMapR);
    }

    bstrListDestroy(inputLines);

    char * result = bstr2cstr(html, '-');
    bdestroy(html);

    return result; // Caller should free()
}

bstring getContentFromLine(bstring line, int formatPaddingLen, int * leadingSpaces)
{
    // Remove padding from front of string.
    bstring content = bmidstr(line, formatPaddingLen, line->slen);
    *leadingSpaces = 0;
    // Remove and count leading whitespace.
    while (content->slen > 0 && content->data[0] == ' ')
    {
        bdelete(content, 0, 1);
        (*leadingSpaces)++;
    }
    return content;
}

void createLine(int side, bstring base, bstring content, lineData lineMap, highlightMask * highlightMask)
{
    if (lineMap.type == INFO)
    {
        content = bfromcstr("");
        lineMap.lineNo = 0;
    }

    int position = 0;
    int needToCloseLastHighlightBeforeEscapingHTML = FALSE;

    if (highlightMask != NULL)
    {
        int lastState = MASK_SAME;
        int advanceBy;
        int i;
        int contentLen = content->slen; // Copy this because it will change as we work.
        for (i = 0; i < contentLen; i++)
        {
            advanceBy = 1; // Normally advance by one char.

            // Escape HTML as we go.
            if (content->data[position] == '&')
            {
                breplace(content, position, 1, bfromcstr("&amp;"), ' ');
                advanceBy += 4;
            }
            else if (content->data[position] == '<')
            {
                breplace(content, position, 1, bfromcstr("&lt;"), ' ');
                advanceBy += 3;
            }
            else if (content->data[position] == '>')
            {
                breplace(content, position, 1, bfromcstr("&gt;"), ' ');
                advanceBy += 3;
            }

            if (highlightMask[i] != lastState)
            {
                if (highlightMask[i] == MASK_DIFFERENT)
                {
                    binsert(content, position, bfromcstr("<em>"), ' ');
                    advanceBy += 4;
                }
                else
                {
                    binsert(content, position, bfromcstr("</em>"), ' ');
                    advanceBy += 5;
                }
            }

            position += advanceBy;
            lastState = highlightMask[i];
        }
    }

    // Escape HTML.
    // TODO: This can't possibly be good enough.
    bfindreplace(content, bfromcstr("&"), bfromcstr("&amp;"), position);
    bfindreplace(content, bfromcstr("<"), bfromcstr("&lt;"), position);
    bfindreplace(content, bfromcstr(">"), bfromcstr("&gt;"), position);

    // Put something in blank lines.
    if (content->slen == 0) bcatcstr(content, "&#32;");

    if (needToCloseLastHighlightBeforeEscapingHTML)
    {
        bcatcstr(content, "</em>");
    }

    // TODO: there's a lot of string manipulation going on here. It might be
    // good for performance to call ballocmin and boost the base string size by
    // a big chunk.

    if (lineMap.lineNo >= 0 && lineMap.type != INFO)
    {
        char * lineNo = lineNumberString(lineMap.lineNo);
        bcatcstr(base, "<td class='line_number ");
        bcatcstr(base, typeString(lineMap.type));
        bcatcstr(base, " ");
        bcatcstr(base, (side == LEFT) ? "left" : "right");
        bcatcstr(base, "' width='*'>");
        bcatcstr(base, lineNo);
        bcatcstr(base, "</td>\n");
        bcatcstr(base, "<td class='line ");
        free(lineNo);
    }
    else
    {
        bcatcstr(base, "<td colspan='2' class='line ");
    }
    bstring whitespace;

    bcatcstr(base, typeString(lineMap.type));
    bcatcstr(base, " ");
    bcatcstr(base, (side == LEFT) ? "left" : "right");
    bcatcstr(base, "' width='49%'>");
    bconcat(base, whitespace = getWhitespace(lineMap.leadingSpaces));
    bconcat(base, content);
    bcatcstr(base, "</td>\n");

    bdestroy(whitespace);
}

bstring getWhitespace(int spaces)
{
    bstring whitespace = bfromcstralloc(spaces * 5, "");
    while (whitespace->slen < spaces * 5)
    {
        bcatcstr(whitespace, "&#32;");
    }
    return whitespace;
}

void createEmptyLine(bstring base)
{
    bcatcstr(base, "<td class='line_number'> </td><td class='line empty'> </td>\n");
}

char * typeString(enum lineType type)
{
    switch (type)
    {
        case SHARED:   return "shared";
        case OLD:      return "old";
        case NEW:      return "new";
        case CHANGE:   return "change";
        case EMPTY:    return "empty";
        case HEADER:   return "header";
        case INFO:     return "info";
        case OLD_FILE: return "old_file";
        case NEW_FILE: return "new_file";
    }

    return "";
}

char * lineNumberString(int lineNo)
{
    char * s = malloc(100 * sizeof(char));
    if (s == NULL)
    {
        free(s);
        printf("Memory allocation error.\n");
        exit(-1);
    }
    if (lineNo <= 0) {
        strcpy(s, "&#32;");
    } else {
        snprintf(s, 100, "%i", lineNo);
    }
    return s;
}

void syncLineNumbers(bstring infoString, int * lineNoL, int * lineNoR)
{
    sscanf(bstr2cstr(infoString, ' '), "@@ -%d,%*d +%d,%*d", lineNoL, lineNoR);
}

void determineLineHighlighting(bstring a, bstring b, highlightMask ** maskPtrA, highlightMask ** maskPtrB)
{
    seq alignA = initSeq(a->slen * 1.5);
    seq alignB = initSeq(b->slen * 1.5);

    seq seqA = stringToSeq(a);
    seq seqB = stringToSeq(b);

    determineAlignment(seqA, seqB, &compareChars, &alignA, &alignB);

    assert(alignA.alen == alignB.alen);
    int len = alignA.alen;

    // Both sequences should be the same length. We'll just get the length of
    // one, which should be the upper limit needed for the masks.

    // Allocate memory for two integer masks.
    highlightMask * maskA = malloc(len * sizeof(highlightMask));
    if (maskA == NULL)
    {
        free(maskA);
        printf("Memory allocation error.\n");
        exit(-1);
    }

    highlightMask * maskB = malloc(len * sizeof(highlightMask));
    if (maskB == NULL)
    {
        free(maskB);
        printf("Memory allocation error.\n");
        exit(-1);
    }

    int i; // Position along the aligned sequences.

    int firstValueInBlockA = -1;
    int firstPosInBlockA = -1;
    int lastComparisonA = MASK_SAME;

    // Positions in each mask.
    int posA = 0;
    int posB = 0;

    for (i = 0; i < len; i++)
    {
        int currentComparisonA = compareStringPositions(alignA, alignB, i);
        int currentComparisonB = compareStringPositions(alignB, alignA, i);

        // Look ahead and back a place in the strings to see if we have an
        // isolated matching character among differences.
        if (currentComparisonA == MASK_SAME &&
            i > 0 &&
            i < len - 1 &&
            compareStringPositions(alignA, alignB, i - 1) != MASK_SAME &&
            compareStringPositions(alignA, alignB, i + 1) != MASK_SAME)
        {
            if (firstPosInBlockA > 0 && firstValueInBlockA == seqA.val[posA])
            {
                // Special case for when the char we are about to smooth is
                // actually at the beginning of the highlighted block.
                maskA[firstPosInBlockA] = MASK_SAME;
                currentComparisonA = MASK_DIFFERENT;
                //printf("%c|%c\n", seqA.val[posA-1], seqA.val[posA]);
            }
            else
            {
                // Pretend the matching characters are different to make the diff
                // look more readable.
                //printf("S");
                if (currentComparisonA != MASK_GAP) currentComparisonA = MASK_DIFFERENT;
                if (currentComparisonB != MASK_GAP) currentComparisonB = MASK_DIFFERENT;
            }
        }
        else {
            //printf("-");
        }

        // Entering region of difference.
        if (currentComparisonA != lastComparisonA &&
            currentComparisonA == MASK_DIFFERENT)
        {
            // Record position and value.
            firstValueInBlockA = seqA.val[posA];
            firstPosInBlockA = posA;
        }

        if (currentComparisonA != MASK_GAP)
        {
            maskA[posA] = currentComparisonA;
            posA++;
        }
        if (currentComparisonB != MASK_GAP)
        {
            maskB[posB] = currentComparisonB;
            posB++;
        }

        if (currentComparisonA != MASK_GAP)
        {
            lastComparisonA = currentComparisonA;
        }
    }
    //printf("\n");

    *maskPtrA = maskA;
    *maskPtrB = maskB;

    freeSeq(&seqA);
    freeSeq(&seqB);

    freeSeq(&alignA);
    freeSeq(&alignB);
}

highlightMask compareStringPositions(seq seqA, seq seqB, int i)
{
    highlightMask result = MASK_GAP;

    if (seqA.val[i] == ALIGN_GAP)
    {
        // no-op
    }
    else if (seqA.val[i] == seqB.val[i])
    {
        result = MASK_SAME;
    }
    else {
        result = MASK_DIFFERENT;
    }

    return result;
}


/* Align two strings using dynamic programming. Based on an algorithm
 * described at http://www.biorecipes.com/DynProgBasic/code.html for aligning
 * sequences of DNA base pairs */
void determineAlignment(seq s, seq t, int (*compare)(seq, seq, int, int), seq * alignSPtr, seq * alignTPtr)
{
    int n = s.alen;
    int m = t.alen;

    int cols = n + 1;
    int rows = m + 1;

    int ** D;
    int i, j;
    int x, y;

    int gapScore = 0;

    // Allocate memory for two integer arrays.
    int aryMemLen = m + n;
    seq alignS = initSeq(aryMemLen);
    seq alignT = initSeq(aryMemLen);

    int p;

    // Handle if one or both strings are empty.
    if (s.alen == 0 || t.alen == 0)
    {
        if (s.alen == 0)
        {
            // Fill S with gaps
            for (p = 0; p < s.alen; p++) alignS.val[p] = ALIGN_GAP;
        }
        else
        {
            // Fill S with normal mapping
            for (p = 0; p < s.alen; p++) alignS.val[p] = s.val[p];
        }

        if (t.alen == 0)
        {
            // Fill T with gaps
            for (p = 0; p < t.alen; p++) alignT.val[p] = ALIGN_GAP;
        }
        else
        {
            // Fill T with normal mapping
            for (p = 0; p < t.alen; p++) alignT.val[p] = t.val[p];
        }

        return;
    }


    // Regularly scheduled programming...

    // Allocate pointer memory for the first dimension of the matrix.
    D = malloc(rows * sizeof(int *));
    if (D == NULL)
    {
        free(D);
        printf("Memory allocation error.\n");
        exit(-1);
    }

    // Allocate integer memory for the second dimension of the matrix.
    for (x = 0; x < rows; x++)
    {
        D[x] = malloc(cols * sizeof(int));
        if (D[x] == NULL)
        {
            free(D[x]);
            printf("Memory allocation error.\n");
            exit(-1);
        }
    }

    // Initialize matrix to be filled with 0's.
    for (x = 0; x < rows; x++)
    {
        for (y = 0; y < cols; y++)
        {
            D[x][y] = 0;
        }
    }

    // Calculate values for first row.
    /*
    for (j = 0; j <= n; j++)
    {
        D[0][j] = gapScore * j;
    }
    */

    // Calculate values for first column.
    /*
    for (i = 0; i <= m; i++)
    {
        D[i][0] = gapScore * i;
    }
    */

    // Calculate inside of matrix.
    for (i = 1; i <= m; i++)
    {
        for (j = 1; j <= n; j++)
        {
            int match = D[i-1][j-1] + compare(s, t, j-1, i-1);
            int sGap = D[i][j-1] + gapScore;
            int tGap = D[i-1][j] + gapScore;
            int max = max3(match, sGap, tGap);
            D[i][j] = (max > 0 ? max : 0);
        }
    }

    /*
    for (x = 0; x < rows; x++)
    {
        for (y = 0; y < cols ; y++)
        {
            printf(" %i ", D[x][y]);
        }
        printf("\n");
    }
    */

    // Trace back through the matrix to find where we came from (which
    // represents the way to align the strings).

    i = m;
    j = n;

    while (i > 0 && j > 0)
    {
        if ((D[i][j] - compare(s, t, j-1, i-1)) == D[i-1][j-1])
        {
            // Both chars
            unshiftSeq(&alignS, s.val[j - 1]);
            unshiftSeq(&alignT, t.val[i - 1]);
            i--;
            j--;
        }
        else if (D[i][j] - gapScore == D[i][j-1])
        {
            // Gap in t/right
            unshiftSeq(&alignS, s.val[j - 1]);
            unshiftSeq(&alignT, ALIGN_GAP);
            j--;
        }
        else if (D[i][j] - gapScore == D[i-1][j])
        {
            // Gap in s/left
            unshiftSeq(&alignS, ALIGN_GAP);
            unshiftSeq(&alignT, t.val[i - 1]);
            i--;
        }
        else
        {
            printf("<GOB>I've made a huge mistake.</GOB>\n");
            exit(-1);
        }
    }

    if (j > 0)
    {
        while (j > 0)
        {
            // Gap in t/right
            unshiftSeq(&alignS, s.val[j - 1]);
            unshiftSeq(&alignT, ALIGN_GAP);
            j--;
        }
    }
    else if (i > 0)
    {
        while (i > 0)
        {
            // Gap in s/left
            unshiftSeq(&alignS, ALIGN_GAP);
            unshiftSeq(&alignT, t.val[i - 1]);
            i--;
        }
    }

    // Deallocate matrix memory.
    for (x = 0; x < rows; x++)
    {
        free(D[x]);
    }
    free(D);

    *alignSPtr = alignS;
    *alignTPtr = alignT;

    // DEBUG
    //printf("\nAlignment(%i, %i):\n%s\n%s\n", s->slen, t->slen, bstr2cstr(sAlign, '_'), bstr2cstr(tAlign, '_'));
}

int charSimilarity(char a, char b)
{
    return (a == b) ? 2 : 0;
}

int compareChars(seq a, seq b, int ai, int bi)
{
    return (a.val[ai] == b.val[bi]) ? 2 : 0;
}

int max2(int a, int b)
{
    if (a > b) return a;
    else return b;
}

int min2(int a, int b)
{
    if (a < b) return a;
    else return b;
}

int max3(int a, int b, int c)
{
    if (a > b) return max2(a, c);
    else return max2(b, c);
}

int min3(int a, int b, int c)
{
    if (a < b) return min2(a, c);
    else return min2(b, c);
}

int editDistance(bstring s, bstring t)
{
    int m = s->slen;
    int n = t->slen;

    int rows = m + 1;
    int cols = n + 1;

    int ** D;
    int i, j;

    // Allocate pointer memory for the first dimension of the matrix.
    D = malloc(rows * sizeof(int *));
    if (D == NULL)
    {
        free(D);
        printf("Memory allocation error.\n");
        exit(-1);
    }

    // Allocate integer memory for the second dimension of the matrix.
    for (i = 0; i < rows; i++)
    {
        D[i] = malloc(cols * sizeof(int));
        if (D[i] == NULL)
        {
            free(D[i]);
            printf("Memory allocation error.\n");
            exit(-1);
        }
    }

    // Initialize matrix to be filled with 0's.
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            D[i][j] = 0;
        }
    }

    for (i = 0; i <= m; i++)
    {
        D[i][0] = i; // Deletion
    }
    for (j = 0; j <= n; j++)
    {
        D[0][j] = j; // Insertion
    }

    for (j = 1; j <= n; j++)
    {
        for (i = 1; i <= m; i++)
        {
            if (s->data[i-1] == t->data[j-1])
            {
                D[i][j] = D[i-1][j-1];
            }
            else
            {
                D[i][j] = min3(
                    D[i-1][j] + 1, // Deletion
                    D[i][j-1] + 1, // Insertion
                    D[i-1][j-1] + 1  // Substitution
                );
            }
        }
    }

    int result = D[m][n];

    // Deallocate matrix memory.
    for (i = 0; i < rows; i++)
    {
        free(D[i]);
    }
    free(D);

    return result;
}

seq initSeq(int size)
{
    seq s;

    s.val = malloc(size * sizeof(int));
    if (s.val == NULL)
    {
        free(s.val);
        printf("Memory allocation error.\n");
        exit(-1);
    }

    s.mlen = size;
    s.alen = 0;

    return s;
}

void setSeq(seq * s, int pos, int val)
{
    if (pos > (*s).mlen)
    {
        printf("Position out of seq bounds.\n");
        exit(-1);
    }

    (*s).val[pos] = val;

    if (pos >= (*s).alen)
    {
        (*s).alen = pos + 1;
    }
}

void freeSeq(seq * s)
{
    free((*s).val);
    (*s).mlen = 0;
    (*s).alen = 0;
}

void unshiftSeq(seq * s, int i)
{
    if ((*s).alen == (*s).mlen)
    {
        printf("Seq out of memory.\n");
        exit(-1);
    }

    (*s).alen++;

    int x;
    for (x = (*s).alen - 1; x > 0; x--)
    {
        (*s).val[x] = (*s).val[x - 1];
    }
    (*s).val[0] = i;
}

seq stringToSeq(bstring str)
{
    seq s = initSeq(str->slen);

    int x;
    for (x = 0; x < str->slen; x++)
    {
        setSeq(&s, x, (int)str->data[x]);
    }

    return s;
}

int stringStartsWith(bstring base, char * start)
{
    return bisstemeqblk(base, start, strlen(start));
}
