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
#include <assert.h>

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

typedef struct lisp_scope {

  smb_ht table;
  struct lisp_scope *up;

} lisp_scope;

static lisp_value *lisp_add(lisp_list *params)
{
  if (params && params->val.type == TP_INT &&
      params->next && params->next->val.type == TP_INT) {
    lisp_value *rv = smb_new(lisp_value, 1);
    rv->type = TP_INT;
    rv->value = LLINT(params->val.value.data_llint + params->next->val.value.data_llint);
    return rv;
  } else {
    fprintf(stderr, "lisp_add: error\n");
    exit(EXIT_FAILURE);
  }
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

lisp_scope *lisp_create_globals(void)
{
  lisp_scope *scope = smb_new(lisp_scope, 1);
  scope->up = NULL;
  ht_init(&scope->table, &ht_string_hash, &data_compare_string);

  ht_insert(&scope->table, PTR("+"), PTR(&lv_add));

  return scope;
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

lisp_value *lisp_parse(smb_iter *, bool);

lisp_list *lisp_parse_list(smb_iter *it, bool within_list) {
  lisp_list *prev = NULL, *list = NULL, *orig = NULL;
  lisp_value *value;

  while ((value = lisp_parse(it, within_list)) != NULL) {
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

lisp_value *lisp_parse(smb_iter *it, bool within_list)
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

void print_lisp_value(lisp_value *lv, int indent_level)
{
  int i;
  lisp_list *l;

  // Indent however much necessary.
  for (i = 0; i < indent_level; i++) {
    putchar(' ');
  }

  // Print out this value.
  switch (lv->type) {
  case TP_INT:
    printf("%d\n", lv->value.data_llint);
    break;

  case TP_ATOM:
    printf("'%ls\n", lv->value.data_ptr);
    break;

  case TP_LIST:
    if (lv->value.data_ptr == NULL) {
      printf("'()\n");
    } else {
      printf("'(\n");
      l = lv->value.data_ptr;
      while (l != NULL) {
        print_lisp_value(&l->val, indent_level + 1);
        l = l->next;
      }
      // indent again
      for (i = 0; i < indent_level; i++) {
        putchar(' ');
      }
      printf(")\n");
    }
    break;

  case TP_FUNCTION:
    printf("a function?\n");
    break;

  case TP_FUNCCALL:
    l = lv->value.data_ptr;
    printf("(%ls\n", l->val.value.data_ptr);
    l = l->next;
    while (l != NULL) {
      print_lisp_value(&l->val, indent_level + 1);
      l = l->next;
    }
    for (i = 0; i < indent_level; i++) {
      putchar(' ');
    }
    printf(")\n");
    break;

  case TP_IDENTIFIER:
    printf("'%ls\n", lv->value.data_ptr);
    break;
  }
}

void data_printer_token(FILE *f, DATA d)
{
  lisp_token *lt = d.data_ptr;
  fprintf(f, "%d: %ls", lt->token.data_llint, lt->text);
}

lisp_value *lisp_evaluate(lisp_value *expression, lisp_scope *scope);

lisp_list *lisp_evaluate_list(lisp_list *list, lisp_scope *scope)
{
  if (list == NULL) return NULL;
  lisp_list *l = smb_new(lisp_list, 1);
  l->val = *lisp_evaluate(&list->val, scope);
  l->next = lisp_evaluate_list(list->next, scope);
  return l;
}

lisp_value *lisp_evaluate(lisp_value *expression, lisp_scope *scope)
{
  smb_status st = SMB_SUCCESS;
  lisp_value *rv, *f;
  lisp_list *l;

  switch (expression->type) {
  case TP_INT:
  case TP_ATOM:
  case TP_LIST:
  case TP_FUNCTION:
    return expression;

  case TP_FUNCCALL:
    l = lisp_evaluate_list(expression->value.data_ptr, scope);
    f = &l->val;
    l = l->next;
    if (f->type == TP_BUILTIN) {
      lisp_value* (*func)(lisp_list*);
      func = f->value.data_ptr;
      return func(l);
    } else {
      printf("error in evaluation\n");
    }
    break;

  case TP_IDENTIFIER:
    rv = ht_get(&scope->table, expression->value, &st).data_ptr;
    assert(st == SMB_SUCCESS);
    return rv;
  }
}

smb_ll *lisp_eval(wchar_t *str)
{
  // given string, return list of tokens (with strings if necessary)
  smb_ll *tokens = lisp_lex(str);
  iter_print(ll_get_iter(tokens), stdout, &data_printer_token);
  // then, parse it to a (list of) lisp_value
  smb_iter it = ll_get_iter(tokens);
  lisp_value *code = lisp_parse(&it, false);
  print_lisp_value(code, 0);
  // then, evaluate that
  lisp_scope *scope = lisp_create_globals();
  lisp_value *res = lisp_evaluate(code, scope);
  print_lisp_value(res, 0);
  return NULL;
}

void lisp(void)
{
  wchar_t *file = read_filew(stdin);
  lisp_eval(file);
}
