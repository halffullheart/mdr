#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstrlib.h"

enum bool {
    FALSE = 0,
    TRUE = 1
};

enum lineType {
    SHARED,
    OLD,
    NEW,
    CHANGE,
    EMPTY,
    HEADER,
    INFO,
    OLD_FILE,
    NEW_FILE
};

enum highlightMaskValue {
    SAME,
    DIFFERENT,
    END_MASK
};

typedef struct {
    enum lineType type;
    int inputPos;
    int padding;
    int lineNo;
} lineData;


char * getHTMLHead();

void createLine(bstring base, bstring content, lineData lineMap, enum highlightMaskValue * highlightMask);

void createEmptyLine(bstring base);

char * typeString(enum lineType type);

char * lineNumberString(int lineNo);

void syncLineNumbers(bstring, int * lineNoL, int * lineNoR);

int charSimilarity(char a, char b);

int max2(int a, int b);

int max3(int a, int b, int c);


char * getHTML()
{
    bstring html = bfromcstr("<!DOCTYPE html>\n<html>\n");
    bcatcstr(html, getHTMLHead());
    bcatcstr(html, "<body>\n<table cellpadding='0'>\n");

    // Read from stdin
    bstring stdinContents = bread ((bNread) fread, stdin);
    if (stdinContents == NULL)
    {
        return "There was an error reading from stdin.";
    }

    // Split into lines
    struct bstrList * inputLines;
    if ((inputLines = bsplit(stdinContents, '\n')) != NULL)
    {

        // We are going to build a map showing which lines in the input belong
        // in which lines in the output and how they should be displayed. We'll
        // allocate the left and right maps to be big enough to each hold all
        // the input data, which is more than enough.
        lineData * lineMapL = malloc(inputLines->qty * sizeof(lineData));
        lineData * lineMapR = malloc(inputLines->qty * sizeof(lineData));
        int lineMapPosL = 0;
        int lineMapPosR = 0;

        char oldFileId[3]  = "---";
        char newFileId[3]  = "+++";
        char lineInfoId[3] = "@@ ";
        char headerId[3]   = "dif";

        int useL;
        int useR;
        enum lineType type;
        int padding;
        int lineNoL = 0;
        int lineNoR = 0;
        enum bool firstInfoLine = TRUE;

        // Map input lines to their output column (left, right, or both)
        int i;
        for (i = 0; i < inputLines->qty; i++) {

            useL = 0;
            useR = 0;
            type = SHARED;
            padding = 1;

            if (bisstemeqblk(inputLines->entry[i], oldFileId, 3) == 1)
            {
                type = OLD_FILE;
                useL = 1;
                padding = 4;
                lineNoL = -1;
                lineNoR = -1;
            }
            else if (bisstemeqblk(inputLines->entry[i], newFileId, 3) == 1)
            {
                type = NEW_FILE;
                useR = 1;
                padding = 4;
                lineNoL = -1;
                lineNoR = -1;
            }
            else if (bisstemeqblk(inputLines->entry[i], lineInfoId, 3) == 1)
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
            else if (bisstemeqblk(inputLines->entry[i], headerId, 3) == 1)
            {
                type = HEADER;
                lineNoL = 0;
                lineNoR = 0;
                firstInfoLine = TRUE;
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

            // Balance
            if (type == HEADER ||
                (type == SHARED && (useL || useR)) ||
                i == inputLines->qty - 1)
            {
                int difference = lineMapPosL - lineMapPosR;
                if (difference > 0)
                {
                    int j;
                    for (j = 0; j < difference; j++)
                    {
                        lineMapR[lineMapPosR].type = EMPTY;
                        lineMapPosR++;
                    }
                }
                else if (difference < 0)
                {
                    int j;
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
                lineMapPosL++;
                lineNoL++;
            }

            if (useR)
            {
                lineMapR[lineMapPosR].inputPos = i;
                lineMapR[lineMapPosR].type = type;
                lineMapR[lineMapPosR].padding = padding;
                lineMapR[lineMapPosR].lineNo = lineNoR - 1;
                lineMapPosR++;
                lineNoR++;
            }

        }

        // Mapping complete. Quick sanity check that both L and R cols have the
        // same length.
        if (lineMapPosL != lineMapPosR)
        {
            char * error = malloc(100 * sizeof(char));
            snprintf(error, 100, "Columns not equal in length. L:%i R:%i\n", lineMapPosL, lineMapPosR);
            return error; // Caller should free().
        }

        // Now we do the formatting work based on the map.
        for (i = 0; i < lineMapPosL; i++)
        {
            bstring inputLineL;
            bstring inputLineR;

            enum highlightMaskValue * highlightMask = NULL;

            if (lineMapL[i].type != EMPTY) {
                inputLineL = inputLines->entry[lineMapL[i].inputPos];
            }

            if (lineMapR[i].type != EMPTY) {
                inputLineR = inputLines->entry[lineMapR[i].inputPos];
            }

            // Compare changed lines
            if (lineMapL[i].type == OLD && lineMapR[i].type == NEW) {

                lineMapL[i].type = CHANGE;
                lineMapR[i].type = CHANGE;

                int shorterLineLen = (inputLineL->slen < inputLineR->slen) ? inputLineL->slen : inputLineR->slen;
                int longerLineLen = (inputLineL->slen > inputLineR->slen) ? inputLineL->slen : inputLineR->slen;

                highlightMask = malloc((longerLineLen + 1) * sizeof(int));

                int j;
                for (j = 0; j < shorterLineLen; j++)
                {
                    if (inputLineL->data[j] == inputLineR->data[j])
                    {
                        highlightMask[j] = SAME;
                    }
                    else
                    {
                        highlightMask[j] = DIFFERENT;
                    }
                }
                highlightMask[j] = END_MASK;
            }

            // Format output
            bcatcstr(html, "<tr>\n");

            if (lineMapL[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(html, inputLineL, lineMapL[i], highlightMask);
            }

            if (lineMapR[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(html, inputLineR, lineMapR[i], highlightMask);
            }

            bcatcstr(html, "</tr>\n");

            if (highlightMask != NULL) free(highlightMask);
        }

        bcatcstr(html, "</table>\n</body>\n</html>\n");

        free(lineMapL);
        free(lineMapR);
    }

    bdestroy(stdinContents);
    bstrListDestroy(inputLines);

    char * result = bstr2cstr(html, '-');
    bdestroy(html);

    return result; // Caller should free()
}

char * getHTMLHead()
{
    return
       "<head>\n"
        "  <title>mdr</title>\n"
        "  <style type='text/css'>\n"
        "    body {\n"
        "      margin: 0;\n"
        "      padding: 0;\n"
        "      font-family: monospace;\n"
        "      font-size: 13px;\n"
        "    }\n"
        "    table {\n"
        "      width: 100%;\n"
        "      border-collapse: collapse;\n"
        "    }\n"
        "    td {\n"
        "      vertical-align: top;\n"
        "    }\n"
        "    .line {\n"
        "      color: #888;\n"
        "    }\n"
        "    .line.new_file,\n"
        "    .line.old_file {\n"
        "      padding: 8px 5px 8px 12px;\n"
        "      font-size: 13px;\n"
        "      font-family: 'Lucida Grande', sans-serif;\n"
        "      background: #f5f5f5;\n"
        "      color: #000;\n"
        "      border-top: 1px solid #fff;\n"
        "      border-right: 1px solid #ccc;\n"
        "      border-bottom: 1px solid #ccc;\n"
        "      border-left: 1px solid #ccc;\n"
        "    }\n"
        "    .line.new {\n"
        "      background: #a7ff92;\n"
        "      color: #000;\n"
        "    }\n"
        "    .line.old {\n"
        "      background: #c0bfff;\n"
        "      color: #000;\n"
        "    }\n"
        "    .line.change {\n"
        "      background: #9af2ed;\n"
        "      color: #000;\n"
        "    }\n"
        "    .line.empty {\n"
        "      background: #f5f5f5;\n"
        "    }\n"
        "    .line.info {\n"
        "      height: 22px;\n"
        "      background: #ddd;\n"
        "      border-top: 1px solid #ccc;\n"
        "      border-bottom: 1px solid #fff;\n"
        "      border-left: 1px solid #ccc;\n"
        "      font-size: 11px;\n"
        "      text-align: center;\n"
        "    }\n"
        "    .line_number {\n"
        "      padding: 0 5px;\n"
        "      background: #eee;\n"
        "      color: #888;\n"
        "      text-align: right;\n"
        "      border-right: 1px solid #ccc;\n"
        "      border-left: 1px solid #ccc;\n"
        "    }\n"
        "    .line em {\n"
        "      font-style: normal;\n"
        "      background: #60d1cb;\n"
        "    }\n"
        "  </style>\n"
        "</head>\n";
}

void createLine(bstring base, bstring content, lineData lineMap, enum highlightMaskValue * highlightMask)
{
    if (lineMap.type == INFO)
    {
        content = bfromcstr("");
        lineMap.lineNo = 0;
        lineMap.padding = 0;
    }

    // Remove diff formatting from beginning of line.
    content = bmidstr(content, lineMap.padding, content->slen);

    int position = 0;
    int needToCloseLastHighlightBeforeEscapingHTML = FALSE;

    if (highlightMask != NULL)
    {
        enum highlightMaskValue lastState = SAME;
        int advanceBy;
        int i;
        for (i = lineMap.padding; i < content->slen; i++)
        {
            advanceBy = 1; // Normally advance by one char.

            if (highlightMask[i] == END_MASK)
            {
                // If we've reached the end of the mask then this is the longer
                // line and the rest of it should be marked as different (or
                // continue to be).
                if (lastState == SAME)
                {
                    binsert(content, position, bfromcstr("<em>"), ' ');
                    position += 4;
                }
                needToCloseLastHighlightBeforeEscapingHTML = TRUE;
                break;
            }

            // Escape HTML as we go.
            if (content->data[position] == '<')
            {
                breplace(content, position, 1, bfromcstr("&lt;"), ' ');
                advanceBy += 3;
            }
            if (content->data[position] == '>')
            {
                breplace(content, position, 1, bfromcstr("&gt;"), ' ');
                advanceBy += 3;
            }
            if (content->data[position] == ' ')
            {
                breplace(content, position, 1, bfromcstr("&emsp;"), ' ');
                advanceBy += 5;
            }

            if (highlightMask[i] != lastState)
            {
                if (highlightMask[i] == DIFFERENT)
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
    bfindreplace(content, bfromcstr(" "), bfromcstr("&emsp;"), position);
    bfindreplace(content, bfromcstr("<"), bfromcstr("&lt;"), position);
    bfindreplace(content, bfromcstr(">"), bfromcstr("&gt;"), position);

    // Put something in blank lines.
    if (content->slen == 0) bcatcstr(content, "&emsp;");

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
        bcatcstr(base, "<td class='line_number'>");
        bcatcstr(base, lineNo);
        bcatcstr(base, "</td>\n");
        bcatcstr(base, "<td class='line ");
        free(lineNo);
    }
    else
    {
        bcatcstr(base, "<td colspan='2' class='line ");
    }
    bcatcstr(base, typeString(lineMap.type));
    bcatcstr(base, "'>");
    bconcat(base, content);
    bcatcstr(base, "</td>\n");

}

void createEmptyLine(bstring base)
{
    bcatcstr(base, "<td class='line_number'>&emsp;</td><td class='line empty'>&emsp;</td>\n");
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
    if (lineNo <= 0) {
        strcpy(s, "&emsp;");
    } else {
        snprintf(s, 100, "%i", lineNo);
    }
    return s;
}

void syncLineNumbers(bstring infoString, int * lineNoL, int * lineNoR)
{
    sscanf(bstr2cstr(infoString, ' '), "@@ -%d,%*d +%d,%*d", lineNoL, lineNoR);
}

/* Align two strings using dynamic programming. Based on an algorithm
 * described at http://www.biorecipes.com/DynProgBasic/code.html for aligning
 * sequences of DNA base pairs */
bstring alignStrings(bstring s, bstring t)
{
    int n = s->slen;
    int m = t->slen;

    int cols = n + 1;
    int rows = m + 1;

    int ** D;
    int i, j;
    int x, y;

    int gapScore = -2;

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
        for(y = 0; y < cols; y++)
        {
            D[x][y] = 0;
        }
    }

    // Calculate values for first row.
    for (j = 0; j <= n; j++)
    {
        D[0][j] = gapScore * j;
    }

    // Calculate values for first column.
    for (i = 0; i <= m; i++)
    {
        D[i][0] = gapScore * i;
    }

    // Calculate inside of matrix.
    for (i = 1; i <= m; i++)
    {
        for (j = 1; j <= n; j++)
        {
            int match = D[i-1][j-1] + charSimilarity(s->data[j-1], t->data[i-1]);
            int sGap = D[i][j-1] + gapScore;
            int tGap = D[i-1][j] + gapScore;
            D[i][j] = max3(match, sGap, tGap);
        }
    }

    bstring sAlign = bfromcstralloc(max2(m, n), "");
    bstring tAlign = bfromcstralloc(max2(m, n), "");

    for (x = 0; x < rows; x++)
    {
        for (y = 0; y < cols ; y++)
        {
            printf(" %i ", D[x][y]);
        }
        printf("\n");
    }

    i = m;
    j = n;
    while (i > 0 && j > 0)
    {
        if ((D[i][j] - charSimilarity(s->data[j-1], t->data[i-1])) == D[i-1][j-1])
        {
            binsertch(tAlign, 0, 1, t->data[i-1]);
            binsertch(sAlign, 0, 1, s->data[j-1]);
            i--;
            j--;
        }
        else if (D[i][j] - gapScore == D[i][j-1])
        {
            binsertch(sAlign, 0, 1, s->data[j-1]);
            binsertch(tAlign, 0, 1, '_');
            j--;
        }
        else if (D[i][j] - gapScore == D[i-1][j])
        {
            binsertch(tAlign, 0, 1, t->data[i-1]);
            binsertch(sAlign, 0, 1, '_');
            i--;
        }
        else
        {
            printf("<GOB>I've made a huge mistake.</GOB>\n");
        }
    }

    if (j > 0)
    {
        while (j > 0)
        {
            binsertch(sAlign, 0, 1, s->data[j-1]);
            binsertch(tAlign, 0, 1, '_');
            j--;
        }
    }
    else if (i > 0)
    {
        while (i > 0)
        {
            binsertch(tAlign, 0, 1, t->data[i-1]);
            binsertch(sAlign, 0, 1, '_');
            i--;
        }
    }

    printf("Alignment:\n%s\n%s\n", bdata(sAlign), bdata(tAlign));

    bdestroy(sAlign);
    bdestroy(tAlign);

    // Deallocate matrix memory.
    for (x = 0; x < rows; x++)
    {
        free(D[x]);
    }
    free(D);

    return bfromcstr("no-op");
}

int charSimilarity(char a, char b)
{
    // For now, just give 2 pts for equality, everything else gets -1.
    return (a == b) ? 2 : -1;
}

int max2(int a, int b)
{
    if (a > b) return a;
    else return b;
}

int max3(int a, int b, int c)
{
    if (a > b) return max2(a, c);
    else return max2(b, c);
}
