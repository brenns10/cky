/***************************************************************************//**

  @file         gram.h

  @author       Stephen Brennan

  @date         Created Monday, 12 May 2014

  @brief        Context-free grammar data structures.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See the LICENSE.txt file for details.

*******************************************************************************/

#ifndef SMB_GRAM_H
#define SMB_GRAM_H

#include <stdbool.h>
#include "libstephen/al.h"

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

   The grammar is represented as a list of symbols, a list of terminal symbols,
   a list of rules, and a start symbol.  When referred to within the structure,
   symbols are indexes within the variable cfg.symbols.

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
     @brief A list of symbols that are terminal ones.

     The list contains integers, which refer to the index within cfg.symbols.
   */
  smb_al terminals;

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

   CNF CFG's are stored differently than your average CFG.  The cfg.symbols
   variable is replaced by two variables, cnf.terminals and cnf.nonterminals.
   The indexes overlap, but that's ok, because you can always tell whether a
   symbol in a rule is terminal or nonterminal, based on the type of rule.

   The second list of symbols removes the need for the sparse list of terminals
   that cfg has.

   For efficiency, rules are stored in two lists.  One contains the A->a rules,
   and the other stores the A->BC rules.

   @see cnf_rule
   @see cnf_init
   @see cnf_create
 */
typedef struct {

  /**
     @brief The terminal symbols of the grammar.  

     Terminal symbols are stored as a list of char* strings, and referred to by
     their index.
   */
  smb_al terminals;

  /**
     @brief The nonterminal symbols of the grammar.

     Nonterminal symbols are stored as a list of char* strings, and referred to
     by their index.
   */
  smb_al nonterminals;

  /**
     @brief The rules that have one symbol in the RHS.
   */
  smb_al rules_one;

  /**
     @brief The rules that have two symbols in the RHS.
   */
  smb_al rules_two;

  /**
     @brief The start symbol.
   */
  int start;

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

int cfg_add_symbol(cfg *pGram, char *symbol, bool terminal);
void cfg_add_rule(cfg *pGram, cfg_rule *newRule);
void cfg_print(cfg *pGram);

#endif
