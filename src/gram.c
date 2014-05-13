/*******************************************************************************

  File:         gram.c

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

#include "libstephen.h"
#include "gram.h"

/**
   @brief Create a new CFG symbol (for either CNF or CFG).

   @param type The type of the symbol (terminal or nonterminal).
   @param name The name of the symbol.
   @param next Pointer to next symbol in the list.
 */
cfg_sym *cfg_sym_new(int type, char *name, cfg_sym *next)
{
  cfg_sym *pNew = (cfg_sym *) malloc(sizeof(cfg_sym));

  if (!pNew) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(cfg_sym));

  pNew->type = type;
  pNew->name = name;
  pNew->next = next;

  return pNew;
}

cfg_rhs *cfg_rhs_new(cfg_sym *sym, cfg_rhs *next)
{
  cfg_rhs *pNew = (cfg_rhs *) malloc(sizeof(cfg_rhs));

  if (!pNew) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(cfg_rhs));

  pNew->sym = sym;
  pNew->next = next;

  return pNew;
}

cfg_rule *cfg_rule_new(cfg_sym *lhs, cfg_rhs *rhs)
{
  cfg_rule *pNew = (cfg_rule *) malloc(sizeof(cfg_rule));
}
