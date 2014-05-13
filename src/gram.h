/*******************************************************************************

  File:         gram.h

  Author:       Stephen Brennan

  Date Created: Monday, 12 May 2014

  Description:  Context-free grammar data structure.

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

#ifndef SMB_GRAM_H
#define SMB_GRAM_H

typedef struct cfg_sym {

  int type;
  char *name;
  struct cfg_sym *next;

} cfg_sym;

typedef struct cfg_rhs {

  cfg_sym *sym;
  struct cfg_rhs *next;

} cfg_rhs;

typedef struct {

  cfg_sym *lhs;
  cfg_rhs *rhs;

} cfg_rule;

typedef struct {

  int type;
  cfg_sym *lhs;
  cfg_sym *one;
  cfg_sym *two;

} cnf_rule;

typedef struct {

  cfg_sym *symbols;
  cfg_rule *rules;

} cfg_grammar;

typedef struct {

  cfg_sym *symbols;
  cnf_rule *rules;

} cnf_grammar;

cfg_sym *cfg_sym_new(int type, char *name, cfg_sym *next);
cfg_rhs *cfg_rhs_new(cfg_sym *sym, cfg_rhs *next);
cfg_rule *cfg_rule_new(cfg_sym *lhs, cfg_rhs *rhs);
cnf_rule *cnf_rule_new(int type, cfg_sym *lhs, cfg_sym *one, cfg_sym *two);
cfg_grammar *cfg_grammar_new(cfg_sym *symbols, cfg_rule *rules);
cnf_grammar *cnf_grammar_new(cfg_sym *symbols, cnf_rule *rules);

#endif //SMB_GRAM_H
