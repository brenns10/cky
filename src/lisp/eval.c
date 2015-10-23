/***************************************************************************//**

  @file         eval.c

  @author       Stephen Brennan

  @date         Created Wednesday, 21 October 2015

  @brief        Lisp evaluation routines. maybe.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <assert.h>

#include "lex.h"
#include "lisp.h"

/**
   @brief "Pretty-print" a lisp value.

   Prints (complete with nesting) a lisp value.  This can be a piece of lisp
   code, or just a value.

   @param lv Value to print.
   @param indent_level Number of spaces to print at the beginning of the line.
 */
static void print_lisp_value(lisp_value *lv, int indent_level)
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

// forward-declaration
lisp_value *lisp_evaluate(lisp_value *expression, lisp_scope *scope);

/**
   @brief Return a list containing each item in a list, evaluated.
   @param list List of items to evaluate.
   @param scope Scope to evaluate each list item within.
   @returns A list of the evaluated items!
 */
static lisp_list *lisp_evaluate_list(lisp_list *list, lisp_scope *scope)
{
  if (list == NULL) return NULL;
  lisp_list *l = smb_new(lisp_list, 1);
  l->val = *lisp_evaluate(&list->val, scope);
  l->next = lisp_evaluate_list(list->next, scope);
  return l;
}

/**
   @brief Return the value of a piece of lisp code!
   @param expression The code to evaluate.
   @param scope The scope to evaluate the code within.
 */
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

lisp_value *lisp_run(wchar_t *str)
{
  // given string, return list of tokens (with strings if necessary)
  smb_ll *tokens = lisp_lex(str);
  // then, parse it to a (list of) lisp_value
  smb_iter it = ll_get_iter(tokens);
  lisp_value *code = lisp_parse(&it);
  // then, evaluate that
  lisp_scope *scope = lisp_create_globals();
  lisp_value *res = lisp_evaluate(code, scope);
  print_lisp_value(res, 0);
  return res;
}

/**
   @brief This is the demo for main.c.
 */
void lisp(void)
{
  wchar_t *file = read_filew(stdin);
  lisp_run(file);
}
