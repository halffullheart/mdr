#ifndef READER_H
#define READER_H

#include "bstrlib.h"

enum side {
    RIGHT,
    LEFT
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
    MASK_SAME,
    MASK_DIFFERENT,
    MASK_GAP
};

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

char * getHTML();

bstring getContentFromLine(bstring line, int formatPaddingLen, int * leadingSpaces);

void createLine(int side, bstring base, bstring content, lineData lineMap, int * highlightMask);

bstring getWhitespace(int spaces);

void createEmptyLine(bstring base);

char * typeString(enum lineType type);

char * lineNumberString(int lineNo);

void syncLineNumbers(bstring, int * lineNoL, int * lineNoR);

void determineLineHighlighting(bstring a, bstring b, int ** maskPtrA, int ** maskPtrB);

int compareStringPositions(seq, seq, int);

void determineAlignment(seq s, seq t, int (*compare)(seq, seq, int, int), seq * posPtrS, seq * posPtrT);

void alignStrings(bstring s, bstring t);

int charSimilarity(char a, char b);

int compareChars(seq a, seq b, int ai, int bi);

int max2(int a, int b);

int max3(int a, int b, int c);

int editDistance(bstring a, bstring b);

seq initSeq(int);

void setSeq(seq *, int, int);

void unshiftSeq(seq *, int);

seq stringToSeq(bstring);

void freeSeq(seq *);

#endif
