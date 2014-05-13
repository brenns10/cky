/*******************************************************************************

  File:         gram.h

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

#ifndef SMB_GRAM_H
#define SMB_GRAM_H

#include "libstephen.h"

#define CFG_SYMBOL_NONE -1
#define CFG_TYPE_REG 0
#define CFG_TYPE_CNF 1

typedef struct {

  int lhs;
  int *rhs;
  int rhs_len;

} cfg_rule;

typedef struct {

  int lhs;
  int rhs_one;
  int rhs_two;

} cnf_rule;

typedef struct {

  int type;
  smb_al symbols;
  smb_al rules;

} cfg;

void cfg_rule_init(cfg_rule *pNewRule, int lhs, int rhs_len);
cfg_rule *cfg_rule_create(int lhs, int rhs_len);
void cfg_rule_destroy(cfg_rule *pRule);
void cfg_rule_delete(cfg_rule *pRule);

void cnf_rule_init(cnf_rule *pNewRule, int lhs, int rhs_one, int rhs_two);
cnf_rule *cnf_rule_create(int lhs, int rhs_one, int rhs_two);
void cnf_rule_destroy(cnf_rule *pRule);
void cnf_rule_delete(cnf_rule *pRule);

void cfg_init(cfg *pGram, int type);
cfg *cfg_create(int type);
void cfg_destroy(cfg *pGram, bool free_symbols);
void cfg_delete(cfg *pGram, bool free_symbols);

#endif
