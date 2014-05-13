/*******************************************************************************

  File:         gram.c

  Author:       Stephen Brennan

  Date Created: Monday, 12 May 2014

  Description:  Context-free grammar data structures.

  Copyright (c) 2014, Stephen Brennan
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Stephen Brennan nor the names of his contributors may
      be used to endorse or promote products derived from this software without
      specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL STEPHEN BRENNAN BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "gram.h" // also includes libstephen.h
#include <stdbool.h>

/**
   @brief Initialize a new CFG rule.

   Creates a CFG rule, and allocates the space for the right hand side.
   However, does not allow the caller to specify the actual right hand side
   values.  This must be done post-call.

   @param pNewRule The memory to initialize
   @param lhs The index of the lhs symbol
   @param rhs_len The number of symbols in the rhs
 */
void cfg_rule_init(cfg_rule *pNewRule, int lhs, int rhs_len)
{
  // Assign values
  pNewRule->lhs = lhs;
  pNewRule->rhs_len = rhs_len;
  pNewRule->rhs = (int *) malloc(rhs_len * sizeof(int));

  // Check for another allocation error
  if (!pNewRule->rhs) {
    RAISE(ALLOCATION_ERROR);
    free(pNewRule);
    SMB_DECREMENT_MALLOC_COUNTER(sizeof(cfg_rule));
    return NULL;
  }

  // Set all items in the right hand side to none.
  for (int i = 0; i < rhs_len; i++) {
    pNewRule->rhs[i] = CFG_SYMBOL_NONE;
  }
}

/**
   @brief Allocate and initialize a new CFG rule.

   Creates a CFG rule, and allocates the space for the right hand side.
   However, does not allow the caller to specify the actual right hand side
   values.  This must be done post-call.

   @param lhs The index of the lhs symbol
   @param rhs_len The number of symbols in the rhs
 */
cfg_rule *cfg_rule_create(int lhs, int rhs_len)
{
  // Allocate the new rule
  cfg_rule *pNewRule = (cfg_rule *) malloc(sizeof(cfg_rule));

  // Check for allocation error
  if (!pNewRule) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(cfg_rule));

  cfg_rule_init(pNewRule, lhs, rhs_len);

  return pNewRule;
}

/**
   @brief Clean up the fields of the CFG rule, but don't free.

   @param pRule The rule to clean up
 */
void cfg_rule_destroy(cfg_rule *pRule)
{
  SMB_DECREMENT_MALLOC_COUNTER(pRule->rhs_len * sizeof(int));
  free(pRule->rhs);
}

/**
   @brief Cleanup and free the CFG rule.

   @param pRule The rule to delete.
 */
void cfg_rule_delete(cfg_rule *pRule)
{
  cfg_rule_destroy(pRule);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(cfg_rule));
  free(pRule);
}

/**

/**
   @brief Initialize a CNF rule.

   Initializes the fields of a CNF rule.  You could just fill them yourself, but
   this conforms to the patterns set in libstephen.

   @param pNewRule The pre-allocated rule memory
   @param lhs The left hand side of the rule
   @param rhs_one The first item in the rhs
   @param rhs_two The second item in the rhs
 */
void cnf_rule_init(cnf_rule *pNewRule, int lhs, int rhs_one, int rhs_two)
{
  pNewRule->lhs = lhs;
  pNewRule->rhs_one = rhs_one;
  pNewRule->rhs_two = rhs_two;
}

/**
   @brief Allocate and initialize a CNF rule.

   Allocates a CNF rule and initializes the space.

   @param lhs The left hand side of the rule
   @param rhs_one The first item in the rhs
   @param rhs_two The second item in the rhs
 */
cnf_rule *cnf_rule_create(int lhs, int rhs_one, int rhs_two)
{
  // Allocate the new rule
  cnf_rule *pNewRule = (cnf_rule *) malloc(sizeof(cnf_rule));

  // Check for allocation error
  if (!pNewRule) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(cnf_rule));

  cnf_rule_init(pNewRule, lhs, rhs_one, rhs_two);
  
  return pNewRule;
}

/**
   @brief Clean up the fields of a CNF rule.  Nothing is actually done here.

   @param pRule The rule to clean up
 */
void cnf_rule_destroy(cnf_rule *pRule)
{
  // Do nothing...
}

/**
   @brief Free a CNF rule.

   @param pRule The rule to delete
 */
void cnf_rule_delete(cnf_rule *pRule)
{
  cnf_rule_destroy(pRule);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(cnf_rule));
  free(pRule);
}

/**
   @brief Initialize a CFG.

   @param pGram The grammar to initialize
   @param type The type of grammar (CNF or regular)
 */
void cfg_init(cfg *pGram, int type)
{
  pGram->type = type;
  al_init(&pGram->symbols);
  al_init(&pGram->rules);
}

/**
   @brief Allocate and initialize a CFG.

   @param type The type of grammar (CNF or regular)
 */
cfg *cfg_create(int type)
{
  // Allocate the new grammar
  cfg *pGram = (cfg *) malloc(sizeof(cfg));

  // Check for allocation error
  if (!pGram) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(cfg));

  cnf_rule_init(pGram, type);
  
  return pNewRule;
}

/**
   @brief Clean up the fields of a CFG.  Do not free.

   @param pGram The grammar to clean up
   @param free_symbols Do we free the symbols?  If so, the allocation counter is
   not decremented.
 */
void cfg_destroy(cfg *pGram, bool free_symbols)
{
  DATA *d;

  if (free_symbols) {
    for (int i = 0; i < al_length(&pGram->symbols); i++) {
      d = al_get(&pGram->symbols, i);
      free(d->data_ptr);
    }
  }

  for (int i = 0; i < al_length(&pGram->rules); i++) {
    d = al_get(&pGram->rules, i);
    if (pGram->type == CFG_TYPE_REG)
      cfg_rule_delete((cfg_rule *)d->data_ptr);
    else
      cnf_rule_delete((cnf_rule *)d->data_ptr);
  }

  al_destroy(&pGram->symbols);
  al_destroy(&pGram->rules);
}

/**
   @brief Clean up and free a CFG.

   @param pGram The grammar to free
   @param free_symbols Do we free the symbols? If so, the allocation counter is
   not decremented.
 */
void cfg_delete(cfg *pGram, bool free_symbols)
{
  cfg_destroy(pGram, free_symbols);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(cfg));
  free(pGram);
}
