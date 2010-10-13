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
} lineData;

char * getHTMLHead();

void formatLine(bstring base, bstring content, char * type, int padding);

char * typeString(enum lineType type);

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
            }
            else if (bisstemeqblk(inputLines->entry[i], newFileId, 3) == 1)
            {
                type = NEW_FILE;
                useR = 1;
                padding = 4;
            }
            else if (bisstemeqblk(inputLines->entry[i], lineInfoId, 3) == 1)
            {
                type = INFO;
            }
            else if (bisstemeqblk(inputLines->entry[i], headerId, 3) == 1)
            {
                type = HEADER;
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
                        lineMapR[lineMapPosR].inputPos = -1;
                        lineMapR[lineMapPosR].type = EMPTY;
                        lineMapR[lineMapPosR].padding = 0;
                        lineMapPosR++;
                    }
                }
                else if (difference < 0)
                {
                    int j;
                    for (j = 0; j < (difference * -1); j++)
                    {
                        lineMapL[lineMapPosL].inputPos = -1;
                        lineMapL[lineMapPosL].type = EMPTY;
                        lineMapL[lineMapPosL].padding = 0;
                        lineMapPosL++;
                    }
                }
            }

            if (useL)
            {
                lineMapL[lineMapPosL].inputPos = i;
                lineMapL[lineMapPosL].type = type;
                lineMapL[lineMapPosL].padding = padding;
                lineMapPosL++;
            }

            if (useR)
            {
                lineMapR[lineMapPosR].inputPos = i;
                lineMapR[lineMapPosR].type = type;
                lineMapR[lineMapPosR].padding = padding;
                lineMapPosR++;
            }

        }

        // Mapping complete. Quick sanity check that both L and R cols have the
        // same length.
        if (lineMapPosL != lineMapPosR)
        {
            char * error = malloc(100 * sizeof(char));
            snprintf(error, 100, "Columns not equal in length. L:%i R:%i\n", lineMapPosL, lineMapPosR);
            return error;
        }

        // Now we do the formatting work based on the map.
        for (i = 0; i < lineMapPosL; i++)
        {
            bstring inputLineL = inputLines->entry[lineMapL[i].inputPos];
            bstring inputLineR = inputLines->entry[lineMapR[i].inputPos];

            bcatcstr(html, "<tr>\n<td>\n");

            if (lineMapL[i].type == EMPTY)
            {
                bcatcstr(html, "<div class='line empty'>&nbsp;</div>");
            }
            else
            {
                formatLine(html, inputLineL, typeString(lineMapL[i].type), lineMapL[i].padding);
            }

            bcatcstr(html, "\n</td>\n<td>\n");

            if (lineMapR[i].type == EMPTY)
            {
                bcatcstr(html, "<div class='line empty'>&nbsp;</div>");
            }
            else
            {
                formatLine(html, inputLineR, typeString(lineMapR[i].type), lineMapR[i].padding);
            }

            bcatcstr(html, "\n</td>\n</tr>\n");
        }

        bcatcstr(html, "</table>\n</body>\n</html>\n");

        free(lineMapL);
        free(lineMapR);
    }

    bstrListDestroy(inputLines);

    //bconcat(html, stdinContents);
    bdestroy(stdinContents);

    char * result = bstr2cstr(html, '-');
    bdestroy(html);

    return result;
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
        "    }\n"
        "    .line.new {\n"
        "      background: #aaffaa;\n"
        "    }\n"
        "    .line.old {\n"
        "      background: #ffaaaa;\n"
        "    }\n"
        "    .line.empty {\n"
        "      background: #dddddd;\n"
        "    }\n"
        "    table {\n"
        "      border-collapse: collapse;\n"
        "    }\n"
        "    td {\n"
        "      vertical-align: top;\n"
        "    }\n"
        "  </style>\n"
        "</head>\n";
}

void formatLine(bstring base, bstring content, char * type, int padding)
{
    // TODO: there's a lot of string manipulation going on here. It might be
    // good for performance to call ballocmin and boost the base string size by
    // a big chunk.
    bcatcstr(base, "<div class='line ");
    bcatcstr(base, type);
    bcatcstr(base, "'>");
    content = bmidstr(content, padding, content->slen);
    if (content->slen == 0) content = bfromcstr("&nbsp;");
    bfindreplace(content, bfromcstr(" "), bfromcstr("&nbsp;"), 0);
    bfindreplace(content, bfromcstr("<"), bfromcstr("&lt;"), 0);
    bfindreplace(content, bfromcstr(">"), bfromcstr("&gt;"), 0);
    bconcat(base, content);
    bcatcstr(base, "</div>");
}

char * typeString(enum lineType type)
{
    switch (type)
    {
        case SAME:
            return "same";
        case OLD:
            return "old";
        case NEW:
            return "new";
        case CHANGE:
            return "change";
        case EMPTY:
            return "empty";
        case HEADER:
            return "header";
        case INFO:
            return "info";
        case OLD_FILE:
            return "old_file";
        case NEW_FILE:
            return "new_file";
    }

    return "";
}
