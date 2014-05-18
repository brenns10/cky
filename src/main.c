/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Created Tuesday, 13 May 2014

  @brief        CKY parser project main program.

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

#include <stdio.h>
#include <wchar.h>

#include "gram.h"
#include "fsm.h"

void simple_gram(void);
void simple_fsm(void);
void read_fsm(void);

int main(int argc, char **argv)
{
  printf("Initial bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);

  simple_gram();

  printf("Intermediate bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);

  simple_fsm();

  printf("Intermediate bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);

  read_fsm();

  printf("Final bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);

  return 0;
}

void simple_fsm(void)
{
  fsm *f = fsm_create();

  fsm_trans *T01 = fsm_trans_create_single(L'a', L'a', FSM_TRANS_POSITIVE, 1);
  fsm_trans *T10 = fsm_trans_create_single(L'a', L'a', FSM_TRANS_POSITIVE, 0);
  fsm_trans *T13 = fsm_trans_create_single(L'b', L'b', FSM_TRANS_POSITIVE, 3);
  fsm_trans *T31 = fsm_trans_create_single(L'b', L'b', FSM_TRANS_POSITIVE, 1);
  fsm_trans *T32 = fsm_trans_create_single(L'a', L'a', FSM_TRANS_POSITIVE, 2);
  fsm_trans *T23 = fsm_trans_create_single(L'a', L'a', FSM_TRANS_POSITIVE, 3);
  fsm_trans *T20 = fsm_trans_create_single(L'b', L'b', FSM_TRANS_POSITIVE, 0);
  fsm_trans *T02 = fsm_trans_create_single(L'b', L'b', FSM_TRANS_POSITIVE, 2);

  wchar_t *i1 = L"abab";
  wchar_t *i2 = L"aab";
  wchar_t *i3 = L"aaaabbbba";

  f->start = 0;

  fsm_add_state(f, true);  // 0
  fsm_add_state(f, false); // 1
  fsm_add_state(f, false); // 2
  fsm_add_state(f, false); // 3

  fsm_add_trans(f, 0, T01);
  fsm_add_trans(f, 1, T10);
  fsm_add_trans(f, 1, T13);
  fsm_add_trans(f, 3, T31);
  fsm_add_trans(f, 3, T32);
  fsm_add_trans(f, 2, T23);
  fsm_add_trans(f, 2, T20);
  fsm_add_trans(f, 0, T02);

  printf("Running on i1=\"%Ls\"\n", i1);
  if (fsm_sim_det(f, i1))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i2=\"%Ls\"\n", i2);
  if (fsm_sim_det(f, i2))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i3=\"%Ls\"\n", i3);
  if (fsm_sim_det(f, i3))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  fsm_delete(f, true);
}

void read_fsm(void) {
  const wchar_t *input = 
    L"start: 0\n"
    L"accept:0\n"
    L"0-1:+a-a\n"
    L"1-0:+a-a\n"
    L"1-3:+b-b\n"
    L"3-1:+b-b\n"
    L"3-2:+a-a\n"
    L"2-3:+a-\\u0061\n"
    L"2-0:+b-b\n"
    L"0-2:+b-b";
  const wchar_t *i1 = L"abab";
  const wchar_t *i2 = L"aab";
  const wchar_t *i3 = L"aaaabbbba";

  fsm *f = fsm_read(input);
  if (f == NULL) {
    return;
  }
  
  printf("Running on i1=\"%Ls\"\n", i1);
  if (fsm_sim_det(f, i1))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i2=\"%Ls\"\n", i2);
  if (fsm_sim_det(f, i2))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i3=\"%Ls\"\n", i3);
  if (fsm_sim_det(f, i3))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  fsm_delete(f, true);
}

void simple_gram(void)
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
  cfg_delete(gram, false);
}
