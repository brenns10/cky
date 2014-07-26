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
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#include "libstephen.h"
#include "gram.h"
#include "fsm.h"
#include "regex.h"

void simple_gram(void);
void simple_fsm(void);
void read_fsm(void);
void read_combine_fsm(void);
void regex(void);
void search(void);
void dot(void);

/**
   @brief Print the help message for the main program.
 */
void help(char *name)
{
  printf("Usage: %s [TESTS]\n", name);
  puts("Runs tests on CKY parser and regex engine.");
  puts("");
  puts("Tests:");
  puts("  -g, --simple-gram       create and print a grammar");
  puts("  -f, --simple-fsm        programmatically create and run a certain FSM");
  puts("  -r, --read-fsm          create an FSM by reading a string, and run it");
  puts("  -c, --read-combine-fsm  read FSMs and combine them using various operators");
  puts("  -e, --regex             input regex and test strings");
  puts("  -s, --search            regex search file");
  puts("  -d, --dot               create graphviz dot from regex");
  puts("");
  puts("Misc:");
  puts("  -h, --help              display this help message and exit");
}

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
  smb_ad data;
  bool executed = false;

  arg_data_init(&data);
  process_args(&data, argc - 1, argv + 1);

  if (check_flag(&data, 'h') || check_long_flag(&data, "help")) {
    help(argv[0]);
    arg_data_destroy(&data);
    exit(0);
  }

  if (check_flag(&data, 'g') || check_long_flag(&data, "simple-gram")) {
    simple_gram();
    executed = true;
  }
  if (check_flag(&data, 'f') || check_long_flag(&data, "simple-fsm")) {
    simple_fsm();
    executed = true;
  }
  if (check_flag(&data, 'r') || check_long_flag(&data, "read-fsm")) {
    read_fsm();
    executed = true;
  }
  if (check_flag(&data, 'c') || check_long_flag(&data, "read-combine-fsm")) {
    read_combine_fsm();
    executed = true;
  }
  if (check_flag(&data, 'e') || check_long_flag(&data, "regex")) {
    regex();
    executed = true;
  }
  if (check_flag(&data, 's') || check_long_flag(&data, "search")) {
    search();
    executed = true;
  }
  if (check_flag(&data, 'd') || check_long_flag(&data, "dot")) {
    dot();
    arg_data_destroy(&data);
    return 0; // exit silently
  }

  if (!executed) {
    help(argv[0]);
    arg_data_destroy(&data);
    exit(1);
  }

  arg_data_destroy(&data);
  printf("\nFinal bytes allocated: %d\n", SMB_GET_MALLOC_COUNTER);
  return 0;
}

/**
   @brief Read a regex from stdin and print to stdout a dot representation.
 */
void dot(void)
{
  int alloc;
  wchar_t *str;
  fsm *compiled_fsm;

  str = smb_read_linew(stdin, &alloc);
  compiled_fsm = regex_parse(str);
  smb_free(wchar_t, str, alloc);
  fsm_dot(compiled_fsm, stdout);
  fsm_delete(compiled_fsm, true);
}

/**
   @brief Read a file completely into memory.

   This function allocates space for a file's contents and reads it into memory.
   The function also requires a parameter in which to store the number of chars
   it has allocated in the buffer, so the memory can be accounted for when
   freed.  This function is independent of encoding.

   @param file The file to read into memory.
   @param[out] bytes Pointer to variable to store number of chars allocated.
   @return Pointer to the file contents in memory.
 */
char *read_file(FILE *file, int *bytes)
{
  char *buffer;
  // assume file is open
  fseek(file, 0L, SEEK_END);
  *bytes = (int) ftell(file) + 1; // extra for null terminator
  fseek(file, 0L, SEEK_SET);
  buffer = smb_new(char, *bytes);
  if (buffer == NULL) {
    PRINT_ERROR_LOC;
    fprintf(stderr, "Error allocating memory for file.\n");
    return NULL;
  }
  fread(buffer, sizeof(char), *bytes, file);
  return buffer;
}

/**
   @brief Interactively search a file for a pattern.
 */
void search(void)
{
  char *filename;
  FILE *file;
  char *regex;
  wchar_t *wregex;
  fsm *regex_fsm;
  char *input;
  wchar_t *winput;
  int alloc, len;
  smb_al *results;
  regex_hit *hit;
  int i;

  printf("Input Filename: ");
  filename = smb_read_line(stdin, &alloc);
  file = fopen(filename, "r");
  smb_free(char, filename, alloc);
  if (file == NULL) {
    perror("Error opening file: ");
    return;
  }

  printf("Input Regex: ");
  regex = smb_read_line(stdin, &alloc);
  len = strlen(regex) + 1;
  wregex = smb_new(wchar_t, len);
  if (utf8toucs4(wregex, regex, len) != 0) {
    PRINT_ERROR_LOC;
    fprintf(stderr, "Error converting to UCS 4.\n");
    smb_free(char, regex, alloc);
    smb_free(wchar_t, wregex, len);
    return;
  }
  smb_free(char, regex, alloc);
  regex_fsm = regex_parse(wregex);
  smb_free(wchar_t, wregex, len);

  input = read_file(file, &alloc);
  fclose(file);
  len = strlen(input) + 1;
  winput = smb_new(wchar_t, len);
  if (utf8toucs4(winput, input, len) != 0) {
    PRINT_ERROR_LOC;
    fprintf(stderr, "Error converting to UCS 4.\n");
    fsm_delete(regex_fsm, true);
    smb_free(char, input, alloc);
    smb_free(wchar_t, winput, len);
    return;
  }
  smb_free(char, input, alloc);

  results = fsm_search(regex_fsm, winput, false, false);
  for (i = 0; i < al_length(results); i++) {
    hit = (regex_hit *)al_get(results, i).data_ptr;
    printf("=> Hit at index %d, length %d\n", hit->start, hit->length);
    printf("   \"%.*ls\"\n", hit->length, winput + hit->start);
    regex_hit_delete(hit);
  }

  smb_free(wchar_t, winput, len);
  fsm_delete(regex_fsm, true);
  al_delete(results);
}

/**
   @brief A light test of my regular expression parser.

   Prompts for a regex, and then loops prompting for test strings.  Runs the
   regex on each test string and reports to stdout whether they are accepted or
   rejected.
 */
void regex(void)
{
  wchar_t *str;
  int alloc;
  fsm * compiled_fsm;

  printf("Input Regex: ");
  str = smb_read_linew(stdin, &alloc);
  puts("Parsing...");
  compiled_fsm = regex_parse(str);
  smb_free(wchar_t, str, alloc);
  printf("Parsed!  Do you wish to see the FSM? [y/n]: ");

  str = smb_read_linew(stdin, &alloc);
  if (str[0] == L'y' || str[0] == L'Y') {
    puts("");
    fsm_print(compiled_fsm, stdout);
  } else { puts(""); }
  smb_free(wchar_t, str, alloc);

  puts("");

  while (true) {
    printf("Input Test String: ");
    str = smb_read_linew(stdin, &alloc);
    if (wcscmp(str, L"exit") == 0) {
      smb_free(wchar_t, str, alloc);
      break;
    }
    printf(fsm_sim_nondet(compiled_fsm, str) ? "Accepted.\n\n" : "Rejected.\n\n");
    smb_free(wchar_t, str, alloc);
  }
  fsm_delete(compiled_fsm, true);
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
