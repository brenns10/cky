/***************************************************************************//**

  @file         functions.c

  @author       Stephen Brennan

  @date         Created Thursday, 22 October 2015

  @brief        Builtin functions of the lisp interpreter.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/ht.h"
#include "lisp.h"

/**
   @brief Add two values.
 */
static lisp_value *lisp_add(lisp_list *params)
{
  lisp_int *rv = (lisp_int*)tp_int.tp_alloc();
  while (params) {
    if (params->value->type != &tp_int) {
      fprintf(stderr, "lisp_add(): error\n");
    }
    rv->value += ((lisp_int*)params->value)->value;
    params = params->next;
  }
  return (lisp_value *)rv;
}


/**
   @brief Return a scope containing the top-level variables for our lisp.
 */
lisp_scope *lisp_create_globals(void)
{
  lisp_scope *scope = smb_new(lisp_scope, 1);
  lisp_builtin *bi;
  scope->up = NULL;
  ht_init(&scope->table, &ht_string_hash, &data_compare_string);

  bi = (lisp_builtin*)tp_builtin.tp_alloc();
  bi->function = &lisp_add;
  ht_insert(&scope->table, PTR("+"), PTR(bi));

  return scope;
}
