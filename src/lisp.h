/***************************************************************************//**

  @file         lisp.h

  @author       Stephen Brennan

  @date         Created Thursday, 22 October 2015

  @brief        Public declarations for the lisp interpreter.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#ifndef CKY_LISP_H
#define CKY_LISP_H

#include "libstephen/ht.h"
#include "libstephen/ll.h"

/*
  Basic types in lisp.
 */
#define TP_INT  0
#define TP_ATOM 1
#define TP_LIST 2
#define TP_BUILTIN 3
#define TP_FUNCTION 4
#define TP_FUNCCALL 5
#define TP_IDENTIFIER 6

/**
  @brief The base lisp value type.
 */
typedef struct {

  /**
     @brief The type of the value.
   */
  int type;
  /**
     @brief The value contained.
   */
  DATA value;

} lisp_value;


/**
   @brief The lisp list type.
 */
typedef struct lisp_list {

  /**
     @brief The value contained in this part of the singly linked list.
   */
  lisp_value val;
  /**
     @brief Pointer to the next node in the list.
   */
  struct lisp_list *next;

} lisp_list;

/**
   @brief A struct to represent one level of scope.
 */
typedef struct lisp_scope {

  /**
     @brief Hash table containing variables!
   */
  smb_ht table;

  /**
     @brief Pointer to the previous level of scope.
   */
  struct lisp_scope *up;

} lisp_scope;

/**
   @brief Tokenize a string.

   The input string is not modified.  All returned tokens are newly allocated,
   so the input string does not need to be "kept around" while the tokens are
   used.  On the flip side, you must free all the tokens when you are done with
   them.

   @param str The string to tokenize.
   @returns A `smb_ll` containing `lisp_token` structs.
 */
smb_ll *lisp_lex(wchar_t *str);

/**
   @brief Parse a token stream and return the first expression.
   @param it Pointer to the iterator over the stream.
   @returns A lisp value!
 */
lisp_value *lisp_parse(smb_iter *it);

/**
   @brief Evaluate an expression within a scope.
   @param expr Reference to expression.
   @param scope The scope to run in.
   @returns The value of the code.
 */
lisp_value *lisp_evaluate(lisp_value *expr, lisp_scope *scope);

/**
   @brief Run a piece of lisp code.
   @param str Code to run.
   @returns The value of the last expression.
 */
lisp_value *lisp_run(wchar_t *str);


#endif // CKY_LISP_H
