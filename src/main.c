/*******************************************************************************

  File:         main.c

  Author:       Stephen Brennan

  Date Created: Tuesday, 13 May 2014

  Description:  CKY parser project main program.

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

#include <stdio.h>

#include "libstephen.h"
#include "gram.h"

int main(int argc, char **argv)
{
  char *start = "start";
  char *plus = "+";
  char *minus = "-";

  cfg *gram = cfg_create();
  int nStart = cfg_add_symbol(gram, start);
  int nPlus = cfg_add_symbol(gram, plus);
  int nMinus = cfg_add_symbol(gram, minus);

  cfg_rule *rule_plus = cfg_rule_create(nStart, 3);
  cfg_rule *rule_minus = cfg_rule_create(nStart, 3);

  rule_plus->rhs[0] = nStart;
  rule_plus->rhs[1] = nPlus;
  rule_plus->rhs[2] = nStart;
  rule_minus->rhs[0] = nStart;
  rule_minus->rhs[1] = nMinus;
  rule_minus->rhs[2] = nStart;

  cfg_add_rule(gram, rule_plus);
  cfg_add_rule(gram, rule_minus);

  cfg_print(gram);

  return 0;
}
