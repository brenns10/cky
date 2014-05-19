/***************************************************************************//**

  @file         gram.h

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

#ifndef SMB_GRAM_H
#define SMB_GRAM_H

#include <stdbool.h>
#include "libstephen.h"

/**
   @brief A constant to indicate that there is no symbol in a grammar slot.
 */
#define CFG_SYMBOL_NONE -1

/**
   @brief A struct to represent a rule in a context-free grammar.

   The structs and functions beginning with `cfg` are for context-free grammars
   that are not restricted by a normal form.  That is, they can have arbitrary
   left-hand sides.

   @see cfg
   @see cfg_rule_init
   @see cfg_rule_create
 */
typedef struct {

  /**
     @brief The symbol on the left hand side of the rule.
   */
  int lhs;

  /**
     @brief An array of symbols for the right hand side of the rule.
   */
  int *rhs;

  /**
     @brief The number of symbols in the right hand side.
   */
  int rhs_len;

} cfg_rule;

/**
   @brief A struct to represent a rule in a CNF CFG.

   CNF grammars are context-free grammars that are restricted in their form.
   The rules may have the form `A -> b`, where `b` is a terminal symbol, or they
   may be of the form `A -> B C`, where `B` and `C` are non-terminal symbols.

   @see cnf 
   @see cnf_rule_init
   @see cnf_rule_create
 */
typedef struct {

  /**
     @brief The symbol on the left-hand side of the rule.
   */
  int lhs;

  /**
     @brief The first symbol on the right hand side of the rule.
   */
  int rhs_one;

  /**
     @brief The second (optional) symbol on the right hand side of the rule.
   */
  int rhs_two;

} cnf_rule;

/**
   @brief A struct to represent a context-free grammar.
   @see cfg_rule
   @see cfg_init
   @see cfg_create
 */
typedef struct {

  /**
     @brief A list of symbols in the grammar.

     The symbols are identified by an index in the list.  The list itself
     contains the string representation of the symbol.
   */
  smb_al symbols;

  /**
     @brief A list of rules in the grammar.
   */
  smb_al rules;

  /**
     @brief The start symbol of the grammar.
   */
  int start;

} cfg;

/**
   @brief A structure to store a CNF context-free grammar.
   @see cnf_rule
   @see cnf_init
   @see cnf_create
 */
typedef struct {

  /**
     @brief The symbols of the grammar.  Symbols are identified by index.
   */
  smb_al symbols;

  /**
     @brief The rules that have one symbol in the RHS.
   */
  smb_al rules_one;

  /**
     @brief The rules that have two symbols in the RHS.
   */
  smb_al rules_two;

} cnf;

void cfg_rule_init(cfg_rule *pNewRule, int lhs, int rhs_len);
cfg_rule *cfg_rule_create(int lhs, int rhs_len);
void cfg_rule_destroy(cfg_rule *pRule);
void cfg_rule_delete(cfg_rule *pRule);

void cnf_rule_init(cnf_rule *pNewRule, int lhs, int rhs_one, int rhs_two);
cnf_rule *cnf_rule_create(int lhs, int rhs_one, int rhs_two);
void cnf_rule_destroy(cnf_rule *pRule);
void cnf_rule_delete(cnf_rule *pRule);

void cfg_init(cfg *pGram);
cfg *cfg_create(void);
void cfg_destroy(cfg *pGram, bool free_symbols);
void cfg_delete(cfg *pGram, bool free_symbols);

void cnf_init(cnf *pGram);
cnf *cnf_create(void);
void cnf_destroy(cnf *pGram, bool free_symbols);
void cnf_delete(cnf *pGram, bool free_symbols);

int cfg_add_symbol(cfg *pGram, char *symbol);
void cfg_add_rule(cfg *pGram, cfg_rule *newRule);
void cfg_print(cfg *pGram);

#endif
