#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "arglist.h"

ArgList * createArgList(){
    ArgList * arglist = (ArgList*)malloc(sizeof(ArgList));
    arglist->length = 0;
    arglist->list = NULL;
    return arglist;
}

#define MAX_ARG_LENGTH 200

static void lengthenList(ArgList * arglist){
    if(arglist->list == NULL){
        arglist->list = (char**)malloc(sizeof(char*));
    } else {
        size_t newSize = sizeof(char*) * (arglist->length + 1);
        arglist->list = (char**)realloc(arglist->list, newSize);
    }
}

void pushFArg(ArgList * arglist, char * fmt, ...){
    char str[MAX_ARG_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, MAX_ARG_LENGTH, fmt, args);
    va_end(args);
    pushArg(arglist, str);
}

void pushArg(ArgList * arglist, char * str){
    arglist->length++;
    lengthenList(arglist);
    char * newStr = malloc(strlen(str));
    strcpy(newStr, str);
    arglist->list[arglist->length-1] = newStr;
}

void freeArgList(ArgList * arglist){
    for(int i = 0; i < arglist->length; i++){
        free(arglist->list[i]);
    }
}

void endArgList(ArgList * arglist){
    arglist->length++;
    lengthenList(arglist);
    arglist->list[arglist->length-1] = (char*)0;
}

void prettyPrint(ArgList * arglist){
    printf("length: %i\n", arglist->length);
    for(int i = 0; i < arglist->length; i++){
        printf("[%i] %s\n", i, arglist->list[i]);
    }
}

