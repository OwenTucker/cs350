//
//  tokens-demo.c
//  mysh
//
//  Created by Dorian Arnold on 10/22/20.
//  Copyright © 2020 Dorian Arnold. All rights reserved.

#include "tokens.h"
#include <stdio.h>

#define MAXLINELEN 256

int main(int argc, const char * argv[]) {
    char line[MAXLINELEN];
    char **tokens;
    
    printf("Enter a series of words/tokens:\n");
    
    if ( fgets( line, MAXLINELEN, stdin ) != NULL ) {
        tokens = get_tokens( line );
        
        for(int i=0; tokens[i] != NULL; i++ )
            printf("\ttoken[%d]: %s\n", i, tokens[i] );
        
        free_tokens ( tokens );
    }
    
    return 0;
}
