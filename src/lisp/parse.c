/***************************************************************************//**

  @file         parse.c

  @author       Stephen Brennan

  @date         Created Thursday, 22 October 2015

  @brief        Tokenizing and parsing for the lisp implementation.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/log.h"
#include "lex.h"
#include "lisp.h"

/*
  A logger for when I want to see debug output.
 */
smb_logger lisp_log = {
  .format = SMB_DEFAULT_LOGFORMAT,
  .num = 0,
};

/**
   @brief Token for whitespace.

   Whitespace is completely ignored in the parser, but it's necessary to include
   it as a token so that it will be matched and ignored.
 */
#define WHITESPACE  0
/**
   @brief Token for open paren.
 */
#define OPEN_PAREN  1
/**
   @brief Token for close paren.
 */
#define CLOSE_PAREN 2
/**
   @brief Token for an identifier (parameter or function name).
 */
#define IDENTIFIER  3
/**
   @brief Token for an atom.
 */
#define ATOM        4
/**
   @brief Token for an integer.
 */
#define INTEGER     5
/**
   @brief Token for the beginning of a list literal, '(
 */
#define OPEN_LIST   6

/**
   @brief A struct to represent the tokens of a lisp program.
 */
typedef struct {

  /**
     @brief The token type.

     Token types are defined as integers in macros above.
   */
  DATA token;

  /**
     @brief Text associated with a token (if necessary).
   */
  wchar_t *text;

} lisp_token;




/**
   @brief Return an instance of the lexer for lisp.

   Must be freed by the caller!
 */
static smb_lex *lisp_create_lexer(void)
{
  smb_lex *lexer = lex_create();
  lex_add_token(lexer, L"\\s+", LLINT(WHITESPACE));
  lex_add_token(lexer, L"\\(", LLINT(OPEN_PAREN));
  lex_add_token(lexer, L"\\)", LLINT(CLOSE_PAREN));
  lex_add_token(lexer, L"[a-zA-Z_+/*?%$=-][0-9a-zA-Z_+/*?%$=-]*", LLINT(IDENTIFIER));
  lex_add_token(lexer, L"'[0-9a-zA-Z_+/*?%$=-]+", LLINT(ATOM));
  lex_add_token(lexer, L"\\d+", LLINT(INTEGER));
  lex_add_token(lexer, L"'\\(", LLINT(OPEN_LIST));
  return lexer;
}

smb_ll *lisp_lex(wchar_t *str)
{
  smb_ll *tokens = ll_create();
  smb_lex *lex = lisp_create_lexer();
  smb_status status = SMB_SUCCESS;

  while (*str != L'\0') {
    LDEBUG(&lisp_log, "lisp_lex(): remaining text: \"%ls\"\n", str);
    lisp_token *lt = smb_new(lisp_token, 1);
    int length;
    lex_yylex(lex, str, &lt->token, &length, &status);
    printf(&lisp_log, "lisp_lex(): match length %d\n", length);
    switch (lt->token.data_llint) {
    case WHITESPACE:
      smb_free(lt);
      str += length;
      continue;
    case ATOM:
    case IDENTIFIER:
    case INTEGER:
      lt->text = smb_new(wchar_t, length + 1);
      wcsncpy(lt->text, str, length);
      lt->text[length] = L'\0';
      break;
    default:
      lt->text = NULL;
      break;
    }

    ll_append(tokens, PTR(lt));
    str += length;
  }

  lex_delete(lex, false);
  return tokens;
}

/*
  Forward declaration breaks dependency cycle between lisp_parse_rec and
  lisp_parse_list.
 */
static lisp_value *lisp_parse_rec(smb_iter *, bool);

/**
   @brief Parse a lisp list.

   This returns a lisp_list corresponding to a token stream.  The token stream
   may be a list literal, or a list of arguments.  This difference is specified
   by the within_list parameter.

   @param it Pointer to the token iterator.
   @param within_list True if this literal is within a list literal.
   @returns A lisp list containing parsed code.
 */
static lisp_list *lisp_parse_list(smb_iter *it, bool within_list) {
  lisp_list *prev = NULL, *list = NULL, *orig = NULL;
  lisp_value *value;

  // lisp_parse_rec() will return NULL when the matching closing paren is reached
  while ((value = lisp_parse_rec(it, within_list)) != NULL) {
    list = smb_new(lisp_list, 1);
    list->val = *value;
    smb_free(value);
    list->next = NULL;
    if (prev) {
      prev->next = list;
    } else {
      orig = list;
    }
    prev = list;
  }

  return orig;
}

/**
   @brief Parse a single piece of lisp code.

   If we are within a list literal, than open-parens are other lists, and things
   that look like identifiers are just atoms.  This is the only difference in
   parsing.

   @param it Pointer to an iterator of tokens.
   @param within_list Are we within a list literal?
   @return Parsed code as a lisp_value*.
 */
lisp_value *lisp_parse_rec(smb_iter *it, bool within_list)
{
  smb_status st = SMB_SUCCESS;
  lisp_value *lv = smb_new(lisp_value, 1);
  lisp_token *lt = it->next(it, &st).data_ptr;

  switch (lt->token.data_llint) {
  case ATOM:
    lv->type = TP_ATOM;
    lv->value = PTR(lt->text);
    break;
  case IDENTIFIER:
    if (within_list) {
      lv->type = TP_ATOM;
    } else {
      lv->type = TP_IDENTIFIER;
    }
    lv->value = PTR(lt->text);
    break;
  case INTEGER:
    lv->type = TP_INT;
    swscanf(lt->text, L"%ld", &lv->value.data_llint);
    break;
  case OPEN_PAREN:
    if (within_list) {
      lv->type = TP_LIST;
    } else {
      lv->type = TP_FUNCCALL;
    }
    lv->value = PTR(lisp_parse_list(it, within_list));
    break;
  case OPEN_LIST:
    lv->type = TP_LIST;
    lv->value = PTR(lisp_parse_list(it, true));
    break;
  case CLOSE_PAREN:
    smb_free(lv);
    lv = NULL;
    break;
  }

  return lv;
}

lisp_value *lisp_parse(smb_iter *it)
{
  return lisp_parse_rec(it, false);
}
