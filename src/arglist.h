#ifndef ARGLIST_H
#define ARGLIST_H

typedef struct ArgList{
    int length;
    char** list;
} ArgList;

ArgList * createArgList();
void pushArg(ArgList * arglist, char * str);
void pushFArg(ArgList * arglist, char * fmt, ...);
void freeArgList(ArgList * arglist);
void endArgList(ArgList * arglist);
void prettyPrint(ArgList * arglist);

#endif
