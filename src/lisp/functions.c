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

/**
   @brief Return a scope containing the top-level variables for our lisp.
 */
lisp_scope *lisp_create_globals(void)
{
  lisp_scope *scope = smb_new(lisp_scope, 1);
  scope->up = NULL;
  ht_init(&scope->table, &ht_string_hash, &data_compare_string);

  ht_insert(&scope->table, PTR("+"), PTR(&lv_add));

  return scope;
}
