/***************************************************************************//**

  @file         lisp.c

  @author       Stephen Brennan

  @date         Created Wednesday, 21 October 2015

  @brief        Lisp implementation maybe.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "libstephen/ll.h"
#include "libstephen/ht.h"
#include "libstephen/str.h"
#include "libstephen/log.h"
#include "lex.h"

#define WHITESPACE  0
#define OPEN_PAREN  1
#define CLOSE_PAREN 2
#define IDENTIFIER  3
#define ATOM        4
#define INTEGER     5
#define OPEN_LIST   6

#define TP_INT  0
#define TP_ATOM 1
#define TP_LIST 2
#define TP_BUILTIN 3
#define TP_FUNCTION 4
#define TP_FUNCCALL 5
#define TP_IDENTIFIER 6

smb_logger lisp_log = {
  .format = SMB_DEFAULT_LOGFORMAT,
  .num = 0,
};

typedef struct {

  int type;
  DATA value;

} lisp_value;

typedef struct lisp_list {

  lisp_value val;
  struct lisp_list *next;

} lisp_list;

typedef struct {

  DATA token;
  wchar_t *text;

} lisp_token;

static lisp_value lisp_add(lisp_list *params)
{
  lisp_value rv;
  if (params && params->val.type == TP_INT &&
      params->next && params->next->val.type == TP_INT) {
    rv.type = TP_INT;
    rv.value = LLINT(params->val.value.data_llint + params->next->val.value.data_llint);
  } else {
    fprintf(stderr, "lisp_add: error\n");
    exit(EXIT_FAILURE);
  }
  return rv;
}

lisp_value lv_add = {TP_BUILTIN, PTR(&lisp_add)};

smb_lex *lisp_create_lexer(void)
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

smb_ht *lisp_create_globals(void)
{
  smb_ht *globals = ht_create(&ht_string_hash, &data_compare_string);

  ht_insert(globals, PTR("+"), PTR(&lv_add));

  return globals;
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

lisp_value *lisp_parse(smb_iter *);

lisp_list *lisp_parse_list(smb_iter *it) {
  lisp_list *prev = NULL, *list = NULL, *orig = NULL;
  lisp_value *value;

  while ((value = lisp_parse(it)) != NULL) {
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

lisp_value *lisp_parse(smb_iter *it)
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
    lv->type = TP_IDENTIFIER;
    lv->value = PTR(lt->text);
    break;
  case INTEGER:
    lv->type = TP_INT;
    swscanf(lt->text, L"%ld", &lv->value.data_llint);
    break;
  case OPEN_PAREN:
    lv->type = TP_FUNCCALL;
    lv->value = PTR(lisp_parse_list(it));
    break;
  case OPEN_LIST:
    lv->type = TP_LIST;
    lv->value = PTR(lisp_parse_list(it));
    break;
  case CLOSE_PAREN:
    smb_free(lv);
    lv = NULL;
    break;
  }

  return lv;
}

void data_printer_token(FILE *f, DATA d)
{
  lisp_token *lt = d.data_ptr;
  fprintf(f, "%d: %ls", lt->token.data_llint, lt->text);
}

smb_ll *lisp_eval(wchar_t *str)
{
  // given string, return list of tokens (with strings if necessary)
  smb_ll *tokens = lisp_lex(str);
  iter_print(ll_get_iter(tokens), stdout, &data_printer_token);
  // then, parse it to a (list of) lisp_value
  smb_iter it = ll_get_iter(tokens);
  lisp_value *code = lisp_parse(&it);
  // then, evaluate that
  return NULL;
}

void lisp(void)
{
  wchar_t *file = read_filew(stdin);
  lisp_eval(file);
}
