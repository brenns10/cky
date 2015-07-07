/***************************************************************************//**

  @file         lex.h

  @author       Stephen Brennan

  @date         Created Monday,  6 July 2015

  @brief        Lexer

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#ifndef SMB_LEX_H
#define SMB_LEX_H

#include "libstephen/al.h"

typedef struct {

  smb_al patterns;
  smb_al tokens;

} smb_lex;

// Data structure functions.
void lex_init(smb_lex *obj);
smb_lex *lex_create();
void lex_destroy(smb_lex *obj);
void lex_delete(smb_lex *obj);

// Loading from a file.
void lex_load(smb_lex *obj, const wchar_t *str, smb_status *status);

// Doing the actual tokenizing.
void lex_yylex(smb_lex *obj, FILE *f, DATA *token, wchar_t *match,
               smb_status *st);

#endif//SMB_LEX_H
