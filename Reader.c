#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstrlib.h"

enum lineType {
    SAME,
    OLD,
    NEW,
    CHANGE,
    EMPTY,
    HEADER,
    INFO,
    OLD_FILE,
    NEW_FILE
};

typedef struct {
    enum lineType type;
    int inputPos;
    int padding;
    int lineNo;
} lineData;


char * getHTMLHead();

void createLine(bstring base, bstring content, lineData lineMap);

void createEmptyLine(bstring base);

char * typeString(enum lineType type);

char * lineNumberString(int lineNo);

void syncLineNumbers(bstring, int * lineNoL, int * lineNoR);


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

        // Map input lines to their output column (left, right, or both)
        int i;
        for (i = 0; i < inputLines->qty; i++) {

            useL = 0;
            useR = 0;
            type = SAME;
            padding = 1;

            if (bisstemeqblk(inputLines->entry[i], oldFileId, 3) == 1)
            {
                type = OLD_FILE;
                useL = 1;
                padding = 4;
                lineNoL = 0;
                lineNoR = 0;
            }
            else if (bisstemeqblk(inputLines->entry[i], newFileId, 3) == 1)
            {
                type = NEW_FILE;
                useR = 1;
                padding = 4;
                lineNoL = 0;
                lineNoR = 0;
            }
            else if (bisstemeqblk(inputLines->entry[i], lineInfoId, 3) == 1)
            {
                type = INFO;
                syncLineNumbers(inputLines->entry[i], &lineNoL, &lineNoR);
                useR = 1;
                useL = 1;
                padding = 1;
            }
            else if (bisstemeqblk(inputLines->entry[i], headerId, 3) == 1)
            {
                type = HEADER;
                lineNoL = 0;
                lineNoR = 0;
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
                type = SAME;
                useL = 1;
                useR = 1;
            }

            // Balance
            if (type == HEADER ||
                (type == SAME && (useL || useR)) ||
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
            return error; // Caller should free()
        }

        // Now we do the formatting work based on the map.
        for (i = 0; i < lineMapPosL; i++)
        {
            bstring inputLineL;
            bstring inputLineR;

            if (lineMapL[i].type != EMPTY) {
                inputLineL = inputLines->entry[lineMapL[i].inputPos];
            }

            if (lineMapR[i].type != EMPTY) {
                inputLineR = inputLines->entry[lineMapR[i].inputPos];
            }

            if (lineMapL[i].type == OLD && lineMapR[i].type == NEW) {
                lineMapL[i].type = CHANGE;
                lineMapR[i].type = CHANGE;
            }

            bcatcstr(html, "<tr>\n");

            if (lineMapL[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(html, inputLineL, lineMapL[i]);
            }

            if (lineMapR[i].type == EMPTY)
            {
                createEmptyLine(html);
            }
            else
            {
                createLine(html, inputLineR, lineMapR[i]);
            }

            bcatcstr(html, "</tr>\n");
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
        "  <title></title>\n"
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
        "      color: #888888;\n"
        "    }\n"
        "    .line.new {\n"
        "      background: #a7ff92;\n"
        "      color: #000000;\n"
        "    }\n"
        "    .line.old {\n"
        "      background: #c0bfff;\n"
        "      color: #000000;\n"
        "    }\n"
        "    .line.change {\n"
        "      background: #9af2ed;\n"
        "      color: #000000;\n"
        "    }\n"
        "    .line.empty {\n"
        "      background: #f5f5f5;\n"
        "    }\n"
        "    .line.info {\n"
        "      height: 22px;\n"
        "      background: #cccccc;\n"
        "      font-size: 11px;\n"
        "      text-align: center;\n"
        "    }\n"
        "    .line_number {\n"
        "      padding: 0 5px;"
        "      background: #cccccc;\n"
        "      text-align: right;\n"
        "    }\n"
        "  </style>\n"
        "</head>\n";
}

void createLine(bstring base, bstring content, lineData lineMap)
{
    if (lineMap.type == INFO)
    {
        content = bfromcstr("");
        lineMap.lineNo = 0;
        lineMap.padding = 0;
    }

    // TODO: there's a lot of string manipulation going on here. It might be
    // good for performance to call ballocmin and boost the base string size by
    // a big chunk.
    char * lineNo = lineNumberString(lineMap.lineNo);
    bcatcstr(base, "<td class='line_number'>");
    bcatcstr(base, lineNo);
    bcatcstr(base, "</td>\n");
    bcatcstr(base, "<td class='line ");
    bcatcstr(base, typeString(lineMap.type));
    bcatcstr(base, "'>");
    content = bmidstr(content, lineMap.padding, content->slen);
    if (content->slen == 0) content = bfromcstr("&nbsp;");
    bfindreplace(content, bfromcstr(" "), bfromcstr("&nbsp;"), 0);
    bfindreplace(content, bfromcstr("<"), bfromcstr("&lt;"), 0);
    bfindreplace(content, bfromcstr(">"), bfromcstr("&gt;"), 0);
    bconcat(base, content);
    bcatcstr(base, "</td>\n");

    free(lineNo);
}

void createEmptyLine(bstring base)
{
    bcatcstr(base, "<td class='line_number'>&nbsp;</td><td class='line empty'>&nbsp;</td>\n");
}

char * typeString(enum lineType type)
{
    switch (type)
    {
        case SAME:     return "same";
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
        strcpy(s, "&nbsp;");
    } else {
        snprintf(s, 100, "%i", lineNo);
    }
    return s;
}

void syncLineNumbers(bstring infoString, int * lineNoL, int * lineNoR)
{
    sscanf(bstr2cstr(infoString, ' '), "@@ -%d,%*d +%d,%*d", lineNoL, lineNoR);
}
