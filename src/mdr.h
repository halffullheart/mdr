#ifndef MDR_H
#define MDR_H

#include "bstrlib.h"

#define ALIGN_GAP -1

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum side {
    RIGHT,
    LEFT
} side;

typedef enum lineType {
    SHARED,
    OLD,
    NEW,
    CHANGE,
    EMPTY,
    HEADER,
    INFO,
    OLD_FILE,
    NEW_FILE
} lineType;

typedef enum highlightMask {
    MASK_SAME,
    MASK_DIFFERENT,
    MASK_GAP
} highlightMask;

typedef struct {
    enum lineType type;
    int inputPos;
    int padding;
    int lineNo;
    int leadingSpaces;
} lineData;

typedef struct {
    int * val;
    int mlen; // Memory size
    int alen; // Array size
} seq;

char * getVersion();

char * getHelp();

char * getHtmlFromStdIn();

bstring getStdInContents();

char * getHtmlFromDiff(bstring diffContents);

bstring getContentFromLine(bstring, int, int *);

void createLine(int, bstring, bstring, lineData, highlightMask *);

bstring getWhitespace(int);

void createEmptyLine(bstring);

char * typeString(enum lineType);

char * lineNumberString(int);

void syncLineNumbers(bstring, int *, int *);

void determineLineHighlighting(bstring, bstring, highlightMask **, highlightMask **);

highlightMask compareStringPositions(seq, seq, int);

void determineAlignment(seq, seq, int (*compare)(seq, seq, int, int), seq *, seq *);

void alignStrings(bstring, bstring);

int charSimilarity(char, char);

int compareChars(seq, seq, int, int);

int max2(int, int);

int max3(int, int, int);

int editDistance(bstring, bstring);

seq initSeq(int);

void setSeq(seq *, int, int);

void unshiftSeq(seq *, int);

seq stringToSeq(bstring);

void freeSeq(seq *);

int stringStartsWith(bstring, char *);

#endif
