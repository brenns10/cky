/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Created Tuesday, 13 May 2014

  @brief        CKY parser project main program.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See the LICENSE.txt file for details.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#include "libstephen/base.h"
#include "libstephen/ad.h"
#include "libstephen/util.h"
#include "gram.h"
#include "fsm.h"
#include "regex.h"
#include "test/tests.h"

void simple_gram(void);
void simple_fsm(void);
void regex(void);
void search(void);
void dot(void);
void test(void);

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
  puts("  -e, --regex             input regex and test strings");
  puts("  -s, --search            regex search file");
  puts("  -d, --dot               create graphviz dot from regex");
  puts("  -t, --test              run tests");
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
  if (check_flag(&data, 't') || check_long_flag(&data, "test")) {
    test();
    executed = true;
  }

  if (!executed) {
    help(argv[0]);
    arg_data_destroy(&data);
    exit(1);
  }

  arg_data_destroy(&data);
  return 0;
}

/**
   @brief Read a regex from stdin and print to stdout a dot representation.
 */
void dot(void)
{
  smb_status status = SMB_SUCCESS;
  wchar_t *str;
  fsm *compiled_fsm;

  str = smb_read_linew(stdin, &status);
  compiled_fsm = regex_parse(str);
  smb_free(str);
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
  int len;
  smb_al *results;
  regex_hit *hit;
  smb_status status = SMB_SUCCESS;
  int i;

  printf("Input Filename: ");
  filename = smb_read_line(stdin, &status);
  file = fopen(filename, "r");
  smb_free(filename);
  if (file == NULL) {
    perror("Error opening file: ");
    return;
  }

  printf("Input Regex: ");
  regex = smb_read_line(stdin, &status);
  len = strlen(regex) + 1;
  wregex = smb_new(wchar_t, len);
  if (utf8toucs4(wregex, regex, len) != 0) {
    PRINT_ERROR_LOC;
    fprintf(stderr, "Error converting to UCS 4.\n");
    smb_free(regex);
    smb_free(wregex);
    return;
  }
  smb_free(regex);
  regex_fsm = regex_parse(wregex);
  smb_free(wregex);

  input = read_file(file, &status);
  fclose(file);
  len = strlen(input) + 1;
  winput = smb_new(wchar_t, len);
  if (utf8toucs4(winput, input, len) != 0) {
    PRINT_ERROR_LOC;
    fprintf(stderr, "Error converting to UCS 4.\n");
    fsm_delete(regex_fsm, true);
    smb_free(input);
    smb_free(winput);
    return;
  }
  smb_free(input);

  results = fsm_search(regex_fsm, winput, false, false);
  for (i = 0; i < al_length(results); i++) {
    hit = (regex_hit *)al_get(results, i, &status).data_ptr;
    printf("=> Hit at index %d, length %d\n", hit->start, hit->length);
    printf("   \"%.*ls\"\n", hit->length, winput + hit->start);
    regex_hit_delete(hit);
  }

  smb_free(winput);
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
  smb_status status = SMB_SUCCESS;
  fsm * compiled_fsm;

  printf("Input Regex: ");
  str = smb_read_linew(stdin, &status);
  puts("Parsing...");
  compiled_fsm = regex_parse(str);
  smb_free(str);
  printf("Parsed!  Do you wish to see the FSM? [y/n]: ");

  str = smb_read_linew(stdin, &status);
  if (str[0] == L'y' || str[0] == L'Y') {
    puts("");
    fsm_print(compiled_fsm, stdout);
  } else { puts(""); }
  smb_free(str);

  puts("");

  while (true) {
    printf("Input Test String: ");
    str = smb_read_linew(stdin, &status);
    if (wcscmp(str, L"exit") == 0) {
      smb_free(str);
      break;
    }
    printf(fsm_sim_nondet(compiled_fsm, str) ? "Accepted.\n\n" : "Rejected.\n\n");
    smb_free(str);
  }
  fsm_delete(compiled_fsm, true);
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

  printf("Running on i1=\"%ls\"\n", i1);
  if (fsm_sim_nondet(f, i1))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i2=\"%ls\"\n", i2);
  if (fsm_sim_nondet(f, i2))
    printf("Accept.\n");
  else
    printf("Reject.\n");

  printf("Running on i3=\"%ls\"\n", i3);
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

void test(void)
{
  fsm_test();
  fsm_io_test();
  regex_test();
}
