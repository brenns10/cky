/***************************************************************************//**

  @file         fsm_io.c

  @author       Stephen Brennan

  @date         Saturday, 23 May 2015

  @brief        Tests for FSM I/O.

*******************************************************************************/

#include <stdbool.h>
#include <wchar.h>

#include "libstephen/ut.h"o
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

void fsm_io_test(void)
{
  smb_ut_group *group = su_create_test_group("fsm_io");

  smb_ut_test *read_fsm = su_create_test("read_fsm", test_read_fsm);
  su_add_test(group, read_fsm);

  su_run_group(group);
  su_delete_group(group);
}
