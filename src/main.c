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
#include "regex.h"

void simple_gram(void);
void simple_fsm(void);
void read_fsm(void);
void read_combine_fsm(void);

/**
   @brief Main entry point of the program.

   Currently, the program has no set interface or function.  I modify it to test
   out certain features I have just developed.  As time goes on, I'll add some
   sort of real test code.

   @param argc Number of command line arguments
   @param argv Array of command line arguments
   @return The program's exit code.
 */
int main(int argc, char **argv)
{
  printf("Initial bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);

  simple_gram();

  printf("After simple_gram(): %d\n", SMB_GET_MALLOC_COUNTER);

  //simple_fsm();

  printf("After simple_fsm(): %d\n", SMB_GET_MALLOC_COUNTER);

  //read_fsm();

  printf("After read_fsm(): %d\n", SMB_GET_MALLOC_COUNTER);

  //read_combine_fsm();

  printf("After read_combine_fsm() [final]: %d\n", SMB_GET_MALLOC_COUNTER);

  return 0;
}

/**
   @brief A function to test reading and combining FSMs.

   Reads in two FSMs: one to accept "Stephen" or "stephen".  Another to accept
   "Brennan" or "brennan".  Then it creates the union, concatenation, and the
   Kleene star of each.  Then it runs all the machines (nondeterministically) on
   some test strings, and prints out the resulting FSMs.
 */
void read_combine_fsm(void)
{
  const wchar_t *m1spec = 
    L"start:0\n"
    L"accept:7\n"
    L"0-1:+s-s S-S\n"
    L"1-2:+t-t\n"
    L"2-3:+e-e\n"
    L"3-4:+p-p\n"
    L"4-5:+h-h\n"
    L"5-6:+e-e\n"
    L"6-7:+n-n\n";

  const wchar_t *m2spec = 
    L"start:0\n"
    L"accept:7\n"
    L"0-1:+b-b B-B\n"
    L"1-2:+r-r\n"
    L"2-3:+e-e\n"
    L"3-4:+n-n\n"
    L"4-5:+n-n\n"
    L"5-6:+a-a\n"
    L"6-7:+n-n\n";

  const wchar_t *inputs[] = {
    L"stephen",
    L"Stephen",
    L"brennan",
    L"Brennan",
    L"stephenbrennan",
    L"stephenBrennan",
    L"Stephenbrennan",
    L"StephenBrennan",
    L"StephenstephenStephen",
    L"BrennanbrennanBrennan",
    L""
  };

  int i;
  fsm *m1 = fsm_read(m1spec), *m2 = fsm_read(m2spec);
  fsm *m1Um2 = fsm_copy(m1), *m1Cm2 = fsm_copy(m1);
  fsm *m1S = fsm_copy(m1), *m2S = fsm_copy(m2);
  fsm_union(m1Um2, m2);
  fsm_concat(m1Cm2, m2);
  fsm_kleene(m1S);
  fsm_kleene(m2S);

  for (i = 0; i < sizeof(inputs)/sizeof(wchar_t *); i++) {
    printf("BEGIN TESTING: \"%Ls\".\n", inputs[i]);
    
    printf("M1: %s\n", fsm_sim_nondet(m1, inputs[i]) ? "accept" : "reject");
    printf("M2: %s\n", fsm_sim_nondet(m2, inputs[i]) ? "accept" : "reject");
    printf("M1 U M2: %s\n", fsm_sim_nondet(m1Um2, inputs[i]) ? "accept" : "reject");
    printf("M1 + M2: %s\n", fsm_sim_nondet(m1Cm2, inputs[i]) ? "accept" : "reject");
    printf("M1*: %s\n", fsm_sim_nondet(m1S, inputs[i]) ? "accept" : "reject");
    printf("M2*: %s\n", fsm_sim_nondet(m2S, inputs[i]) ? "accept" : "reject");
  }

  fsm_print(m1, stdout);
  fsm_print(m2, stdout);
  fsm_print(m1Um2, stdout);
  fsm_print(m1Cm2, stdout);
  fsm_print(m1S, stdout);
  fsm_print(m2S, stdout);

  fsm_delete(m1, true);
  fsm_delete(m2, true);
  fsm_delete(m1Um2, true);
  fsm_delete(m1Cm2, true);
  fsm_delete(m1S, true);
  fsm_delete(m2S, true);
}

/**
   @brief A function that tests the basic construction of a simple FSM.

   Using the actual creation functions, creates a FSM to accept a string with an
   even number of a's and b's.  Currently, the function simulates it
   nondeterministically, even though the machine is deterministic.
 */
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
  if (fsm_sim_nondet(f, i1))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i2=\"%Ls\"\n", i2);
  if (fsm_sim_nondet(f, i2))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i3=\"%Ls\"\n", i3);
  if (fsm_sim_nondet(f, i3))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  fsm_delete(f, true);
}

/**
   @brief This function tests reading in a FSM and running it.

   It reads in the even a's and b's machine, and it simulates it on similar
   inputs.
 */
void read_fsm(void) {
  const wchar_t *input = 
    L"start:0\n"
    L"accept:3\n"
    L"0-0:+b-b\n"
    L"0-1:+a-a\n"
    L"1-2:+b-b\n"
    L"2-3:+a-a\n";
  const wchar_t *i1 = L"ababa";
  const wchar_t *i2 = L"aabaa";
  const wchar_t *i3 = L"aaaabbbba";

  fsm *f = fsm_read(input);
  if (f == NULL) {
    return;
  }
  
  printf("Running on i1=\"%Ls\"\n", i1);
  if (fsm_sim_nondet(f, i1))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i2=\"%Ls\"\n", i2);
  if (fsm_sim_nondet(f, i2))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i3=\"%Ls\"\n", i3);
  if (fsm_sim_nondet(f, i3))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  fsm_print(f, stdout);
  fsm_delete(f, true);
}

/**
   @brief Tests creating a grammar, and printing it out.
 */
void simple_gram(void)
{
  char *start = "start";
  char *plus = "+";
  char *minus = "-";
  char *number = "NUMBER";

  cfg *gram = cfg_create();
  int nStart = cfg_add_symbol(gram, start, false);
  int nPlus = cfg_add_symbol(gram, plus, true);
  int nMinus = cfg_add_symbol(gram, minus, true);
  int nNumber = cfg_add_symbol(gram, number, true);

  cfg_rule *rule_plus = cfg_rule_create(nStart, 3);
  cfg_rule *rule_minus = cfg_rule_create(nStart, 3);
  cfg_rule *rule_number = cfg_rule_create(nStart, 1);

  rule_plus->rhs[0] = nStart;
  rule_plus->rhs[1] = nPlus;
  rule_plus->rhs[2] = nStart;
  rule_minus->rhs[0] = nStart;
  rule_minus->rhs[1] = nMinus;
  rule_minus->rhs[2] = nStart;
  rule_number->rhs[0] = nNumber;

  cfg_add_rule(gram, rule_plus);
  cfg_add_rule(gram, rule_minus);
  cfg_add_rule(gram, rule_number);

  cfg_print(gram);
  cfg_delete(gram, false);
}
