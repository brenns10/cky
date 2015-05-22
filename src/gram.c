/***************************************************************************//**

  @file         gram.c

  @author       Stephen Brennan

  @date         Created Monday, 12 May 2014

  @brief        Context-free grammar data structures.

  @copyright    Copyright (c) 2014, Stephen Brennan.
  All rights reserved.

  @copyright
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

  @copyright
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

#include <stdbool.h>
#include <stdio.h>

#include "gram.h"
#include "libstephen/al.h"

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
  int i;

  // Assign values
  pNewRule->lhs = lhs;
  pNewRule->rhs_len = rhs_len;
  pNewRule->rhs = smb_new(int, rhs_len);

  // Set all items in the right hand side to none.
  for (i = 0; i < rhs_len; i++) {
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
  cfg_rule *pNewRule = smb_new(cfg_rule, 1);
  cfg_rule_init(pNewRule, lhs, rhs_len);
  return pNewRule;
}

/**
   @brief Clean up the fields of the CFG rule, but don't free.

   @param pRule The rule to clean up
 */
void cfg_rule_destroy(cfg_rule *pRule)
{
  smb_free(pRule->rhs);
}

/**
   @brief Cleanup and free the CFG rule.

   @param pRule The rule to delete.
 */
void cfg_rule_delete(cfg_rule *pRule)
{
  cfg_rule_destroy(pRule);
  smb_free(pRule);
}

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
  cnf_rule *pNewRule = smb_new(cnf_rule, 1);
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
  smb_free(pRule);
}

/**
   @brief Initialize a CFG.

   @param pGram The grammar to initialize
 */
void cfg_init(cfg *pGram)
{
  al_init(&pGram->symbols);
  al_init(&pGram->terminals);
  al_init(&pGram->rules);
  pGram->start = CFG_SYMBOL_NONE;
}

/**
   @brief Allocate and initialize a CFG.
 */
cfg *cfg_create(void)
{
  cfg *pGram = smb_new(cfg, 1);
  cfg_init(pGram);
  return pGram;
}

/**
   @brief Clean up the fields of a CFG.  Do not free.

   @param pGram The grammar to clean up
   @param free_symbols Do we free the symbols?  If so, the allocation counter is
   not decremented.
 */
void cfg_destroy(cfg *pGram, bool free_symbols)
{
  int i;
  DATA d;
  smb_status status;

  if (free_symbols) {
    for (i = 0; i < al_length(&pGram->symbols); i++) {
      d = al_get(&pGram->symbols, i, &status);
      smb_free(d.data_ptr);
    }
  }

  for (i = 0; i < al_length(&pGram->rules); i++) {
    d = al_get(&pGram->rules, i, &status);
    cfg_rule_delete((cfg_rule *)d.data_ptr);
  }

  al_destroy(&pGram->symbols);
  al_destroy(&pGram->terminals);
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
  smb_free(pGram);
}

/**
   @brief Add a string symbol to the grammar.

   This function can be called safely if the symbol already exists in the
   grammar.  In that case, it simply returns the symbol's index.  However, note
   that if the symbol exists in the grammar, this function will **not** update
   its terminal status.  That is, if it is in the terminal list and terminal was
   false, this function won't delete it from the list, and the same if it's not
   in the list and terminal was true.

   @param pGram The grammar to add to
   @param symbol The symbol to add
   @param terminal Whether or not to add the symbol to the list of terminals
   @return The index of the new symbol.
 */
int cfg_add_symbol(cfg *pGram, char *symbol, bool terminal)
{
  DATA d;
  int idx;
  d.data_ptr = symbol;
  idx = al_index_of(&pGram->symbols, d, &data_compare_string);
  if (idx != -1) {
    return idx;
  } else {
    al_append(&pGram->symbols, d);
    idx = al_length(&pGram->symbols) - 1;
    if (terminal) {
      d.data_llint = idx;
      al_append(&pGram->terminals, d);
    }
    return idx;
  }
}

/**
   @brief Add a rule to the grammar.

   @param pGram The grammar to add to
   @param newRule The rule to add
 */
void cfg_add_rule(cfg *pGram, cfg_rule *newRule)
{
  DATA d;
  d.data_ptr = newRule;
  al_append(&pGram->rules, d);
}

/**
   @brief Print the grammar to stdout.

   @param pGram The grammar to print to stdout.
 */
void cfg_print(cfg *pGram)
{
  int i, j;
  DATA d, e;
  cfg_rule *rule;
  smb_status status;
  for (i = 0; i < al_length(&pGram->rules); i++) {
    d = al_get(&pGram->rules, i, &status);
    rule = d.data_ptr;
    e = al_get(&pGram->symbols, rule->lhs, &status);
    printf("%s --> ", (char *) e.data_ptr);
    for (j = 0; j < rule->rhs_len; j++) {
      e = al_get(&pGram->symbols, rule->rhs[j], &status);
      printf("%s ", (char *)e.data_ptr);
    }
    printf("\n");
  }
  printf("Terminals: ");
  for (i = 0; i < al_length(&pGram->terminals); i++) {
    printf("%s ", (char *) al_get(&pGram->symbols,
                                  (int) al_get(&pGram->terminals, i,
                                               &status).data_llint,
                                  &status).data_ptr);
  }
  printf("\n");
}

/**
   @brief Initialize a CNF grammar.

   @param pGram The grammar to initialize
 */
void cnf_init(cnf *pGram)
{
  al_init(&pGram->terminals);
  al_init(&pGram->nonterminals);
  al_init(&pGram->rules_one);
  al_init(&pGram->rules_two);
  pGram->start = CFG_SYMBOL_NONE;
}

/**
   @brief Allocate and initialize a CNF grammar.
 */
cnf *cnf_create(void)
{
  cnf *pGram = smb_new(cnf, 1);
  cnf_init(pGram);
  return pGram;
}

/**
   @brief Clean up the fields of a CNF grammar.  Do not free.

   @param pGram The grammar to clean up
   @param free_symbols Do we free the symbols?  If so, the allocation counter is
   not decremented.
 */
void cnf_destroy(cnf *pGram, bool free_symbols)
{
  int i;
  DATA d;
  smb_status status;

  if (free_symbols) {
    for (i = 0; i < al_length(&pGram->terminals); i++) {
      d = al_get(&pGram->terminals, i, &status);
      smb_free(d.data_ptr);
    }
    for (i = 0; i < al_length(&pGram->nonterminals); i++) {
      d = al_get(&pGram->nonterminals, i, &status);
      smb_free(d.data_ptr);
    }
  }

  for (i = 0; i < al_length(&pGram->rules_one); i++) {
    d = al_get(&pGram->rules_one, i, &status);
    cnf_rule_delete((cnf_rule *)d.data_ptr);
  }
  for (i = 0; i < al_length(&pGram->rules_two); i++) {
    d = al_get(&pGram->rules_two, i, &status);
    cnf_rule_delete((cnf_rule *)d.data_ptr);
  }

  al_destroy(&pGram->terminals);
  al_destroy(&pGram->nonterminals);
  al_destroy(&pGram->rules_one);
  al_destroy(&pGram->rules_two);
}

/**
   @brief Clean up and free a CNF grammar.

   @param pGram The grammar to free
   @param free_symbols Do we free the symbols? If so, the allocation counter is
   not decremented.
 */
void cnf_delete(cnf *pGram, bool free_symbols)
{
  cnf_destroy(pGram, free_symbols);
  smb_free(pGram);
}
