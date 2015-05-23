/***************************************************************************//**

  @file         fsm_io.c

  @author       Stephen Brennan

  @date         Saturday, 23 May 2015

  @brief        Tests for FSM I/O.

*******************************************************************************/

#include <stdbool.h>
#include <wchar.h>

#include "libstephen/ut.h"
#include "fsm.h"

/**
   @brief This function tests reading in a FSM and running it.

   It reads in the even a's and b's machine, and it simulates it on similar
   inputs.
 */
static int test_read_fsm(void) {
  int i;
  const wchar_t *machine =
    L"start:0\n"
    L"accept:0\n"
    L"0-1:+a-a\n"
    L"0-2:+b-b\n"
    L"1-0:+a-a\n"
    L"1-3:+b-b\n"
    L"2-3:+a-a\n"
    L"2-0:+b-b\n"
    L"3-1:+b-b\n"
    L"3-2:+a-a\n";
  const wchar_t *inputs[] = {
    L"ababa",
    L"aabaa",
    L"aaaabbbba",
    L"ab",
    L"abab",
    L"aabb"
  };
  const bool results[] = {false, false, false, false, true, true};

  fsm *f = fsm_read(machine);
  for (i = 0; i < sizeof(inputs)/sizeof(wchar_t*); i++) {
    TEST_ASSERT(fsm_sim_nondet(f, inputs[i]) == results[i]);
  }
  fsm_delete(f, true);
  return 0;
}

/**
   @brief This function tests reading an FSM with escape sequences.
 */
static int test_escape(void)
{
  int i;
  const wchar_t *machine =
    L"start:0\n"
    L"accept:12\n"
    L"0-1:+\\a-\\a\n"           // alarm
    L"1-2:+\\b-\\b\n"           // backspace
    L"2-3:+\\e-\\e\n"           // EPSILON
    L"3-4:+\\f-\\f\n"           // form feed
    L"4-5:+\\n-\\n\n"           // newline (lf)
    L"5-6:+\\r-\\r\n"           // carriage return
    L"6-7:+\\t-\\t\n"           // tab
    L"7-8:+\\v-\\v\n"           // vertical tab
    L"8-9:+\\\\-\\\\\n"         // backslash
    L"9-10:+\\+-\\+\n"          // other character
    L"10-11:+\\x3A-\\x3a\n"     // colon
    L"11-12:+\\u0051-\\u0051\n";// Q
  const wchar_t *inputs[] = {
    L"\a\b\f\n\r\t\v\\+:Q", // accept
    L"a\b\f\n\r\t\v\\+:Q",  // reject
    L"\ab\f\n\r\t\v\\+:Q",  // reject
    L"\a\bf\n\r\t\v\\+:Q",  // reject
    L"\a\b\fn\r\t\v\\+:Q",  // reject
    L"\a\b\f\nr\t\v\\+:Q",  // reject
    L"\a\b\f\n\rt\v\\+:Q",  // reject
    L"\a\b\f\n\r\tv\\+:Q",  // reject
    L"\a\b\f\n\r\t\v/+:Q",  // reject
    L"\a\b\f\n\r\t\v\\-:Q", // reject
    L"\a\b\f\n\r\t\v\\-;Q", // reject
    L"\a\b\f\n\r\t\v\\-:q", // reject
  };
  const bool results[] = {true, false, false, false, false, false, false, false,
                          false, false, false, false};

  fsm *f = fsm_read(machine);
  for (i = 0; i < sizeof(inputs)/sizeof(wchar_t*); i++) {
    TEST_ASSERT(fsm_sim_nondet(f, inputs[i]) == results[i]);
  }
  fsm_delete(f, true);
  return 0;
}

void fsm_io_test(void)
{
  smb_ut_group *group = su_create_test_group("fsm_io");

  smb_ut_test *read_fsm = su_create_test("read_fsm", test_read_fsm);
  su_add_test(group, read_fsm);

  smb_ut_test *escape = su_create_test("escape", test_escape);
  su_add_test(group, escape);

  su_run_group(group);
  su_delete_group(group);
}
