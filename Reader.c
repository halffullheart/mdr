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
    INFO
};

typedef struct {
    enum lineType type;
    int inputPos;
    int padding;
} lineData;

char * getHTMLHead();

char * getHTML()
{
    bstring html = bfromcstr("<!DOCTYPE html>");
    bcatcstr(html, getHTMLHead());

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

        char oldFileId[3] = "---";
        char newFileId[3] = "+++";
        char infoId[3] = "@@ ";

        int useL = 0;
        int useR = 0;
        enum lineType typeL = SAME;
        enum lineType typeR = SAME;

        // Map input lines to their output column (left, right, or both)
        int i;
        for (i = 0; i < inputLines->qty; i++) {

            useL = 0;
            useR = 0;
            typeL = SAME;
            typeR = SAME;

            if (bisstemeqblk(inputLines->entry[i], oldFileId, 3) == 1)
            {
                typeL = HEADER;
                useL = 1;
            }
            else if (bisstemeqblk(inputLines->entry[i], newFileId, 3) == 1)
            {
                typeR = HEADER;
                useR = 1;
            }
            else if (bdata(inputLines->entry[i])[0] == '-')
            {
                typeL = OLD;
                useL = 1;
            }
            else if (bdata(inputLines->entry[i])[0] == '+')
            {
                typeR = NEW;
                useR = 1;
            }
            else if (bdata(inputLines->entry[i])[0] == ' ')
            {
                typeL = SAME;
                typeR = SAME;
                useL = 1;
                useR = 1;
            }

            if (useL)
            {
                lineMapL[lineMapPosL].inputPos = i;
                lineMapL[lineMapPosL].type = typeL;
                lineMapPosL++;
            }

            if (useR)
            {
                lineMapR[lineMapPosR].inputPos = i;
                lineMapR[lineMapPosR].type = typeR;
                lineMapPosR++;
            }

            // Balance
            if (typeL == HEADER || typeR == HEADER || i == inputLines->qty - 1)
            {
                int difference = lineMapPosL - lineMapPosR;
                if (difference > 0)
                {
                    int j;
                    for (j = 0; j < difference; j++)
                    {
                        lineMapR[lineMapPosR].inputPos = -1;
                        lineMapR[lineMapPosR].type = EMPTY;
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
                        lineMapPosL++;
                    }
                }
            }

        }

        if (lineMapPosL != lineMapPosR)
        {
            char * error = malloc(100 * sizeof(char));
            snprintf(error, 100, "Columns not equal in length. L:%i R:%i\n", lineMapPosL, lineMapPosR);
            return error;
        }

        for (i = 0; i < lineMapPosL; i++)
        {
            bconcat(html, inputLines->entry[lineMapL[i].inputPos]);
            bconchar(html, '\n');
        }

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
        "    }\n"
        "  </style>\n"
        "</head>\n";
}
