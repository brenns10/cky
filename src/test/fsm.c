/***************************************************************************//**

  @file         fsm.c

  @author       Stephen Brennan

  @date         Sunday, 18 January 2015

  @brief        Test fsm functions.

*******************************************************************************/

#include "libstephen/ut.h"
#include "fsm.h"


/**
   @brief Test for memory leaks.

   This will always pass, but Valgrind may catch memory leaks if errors exist.
   This is why you should always run tests with Valgrind!!
 */
int fsm_test_memory(void)
{
  fsm_trans fsm_trans_stack;
  fsm_trans *fsm_trans_heap;
  fsm fsm_stack;
  fsm *fsm_heap;

  fsm_trans_init(&fsm_trans_stack, 3, FSM_TRANS_POSITIVE, 12);
  fsm_trans_heap = fsm_trans_create(3, FSM_TRANS_POSITIVE, 0);
  fsm_trans_destroy(&fsm_trans_stack);

  fsm_init(&fsm_stack);
  fsm_heap = fsm_create();
  fsm_destroy(&fsm_stack, true);

  fsm_add_state(fsm_heap, true);
  fsm_add_trans(fsm_heap, 0, fsm_trans_heap);
  fsm_delete(fsm_heap, true);
  return 0;
}

/**
   @brief Test fsm_trans checking.

   Makes sure that testing transitions works correctly.
 */
int fsm_test_check(void)
{
  fsm_trans *single_positive = fsm_trans_create(1, FSM_TRANS_POSITIVE, 10);
  fsm_trans *multiple_positive = fsm_trans_create(2, FSM_TRANS_POSITIVE, 10);
  fsm_trans *single_negative = fsm_trans_create(1, FSM_TRANS_NEGATIVE, 10);
  fsm_trans *multiple_negative = fsm_trans_create(2, FSM_TRANS_NEGATIVE, 10);

  single_positive->start[0] = L'b';
  multiple_positive->start[0] = L'b';
  single_negative->start[0] = L'b';
  multiple_negative->start[0] = L'b';

  single_positive->end[0] = L'b';
  multiple_positive->end[0] = L'b';
  single_negative->end[0] = L'b';
  multiple_negative->end[0] = L'b';

  multiple_positive->start[1] = L'c';
  multiple_negative->start[1] = L'c';

  multiple_positive->end[1] = L'c';
  multiple_negative->end[1] = L'c';

  TEST_ASSERT(!fsm_trans_check(single_positive, L'a'));
  TEST_ASSERT(!fsm_trans_check(multiple_positive, L'a'));
  TEST_ASSERT(fsm_trans_check(single_negative, L'a'));
  TEST_ASSERT(fsm_trans_check(multiple_negative, L'a'));

  TEST_ASSERT(fsm_trans_check(single_positive, L'b'));
  TEST_ASSERT(fsm_trans_check(multiple_positive, L'b'));
  TEST_ASSERT(!fsm_trans_check(single_negative, L'b'));
  TEST_ASSERT(!fsm_trans_check(multiple_negative, L'b'));

  TEST_ASSERT(!fsm_trans_check(single_positive, L'c'));
  TEST_ASSERT(fsm_trans_check(multiple_positive, L'c'));
  TEST_ASSERT(fsm_trans_check(single_negative, L'c'));
  TEST_ASSERT(!fsm_trans_check(multiple_negative, L'c'));

  TEST_ASSERT(!fsm_trans_check(single_positive, L'd'));
  TEST_ASSERT(!fsm_trans_check(multiple_positive, L'd'));
  TEST_ASSERT(fsm_trans_check(single_negative, L'd'));
  TEST_ASSERT(fsm_trans_check(multiple_negative, L'd'));

  fsm_trans_delete(single_positive);
  fsm_trans_delete(multiple_positive);
  fsm_trans_delete(single_negative);
  fsm_trans_delete(multiple_negative);
  return 0;
}

/**
   @brief Test the shortcut methods for creating "single" FSM transitions.
 */
int test_shortcut(void)
{
  fsm_trans *a = fsm_trans_create_single(L'a', L'b', FSM_TRANS_POSITIVE, 10);
  fsm_trans b;
  fsm_trans_init_single(&b, L'a', L'b', FSM_TRANS_NEGATIVE, 10);

  TEST_ASSERT(fsm_trans_check(a, L'a'));
  TEST_ASSERT(!fsm_trans_check(&b, L'a'));
  TEST_ASSERT(fsm_trans_check(a, L'b'));
  TEST_ASSERT(!fsm_trans_check(&b, L'b'));
  TEST_ASSERT(!fsm_trans_check(a, L'c'));
  TEST_ASSERT(fsm_trans_check(&b, L'c'));

  fsm_trans_delete(a);
  fsm_trans_destroy(&b);
  return 0;
}

/**
   @brief Test the copy method works.  Valgrind will help here.
 */
int test_copy(void)
{
  fsm_trans *a = fsm_trans_create_single(L'a', L'b', FSM_TRANS_POSITIVE, 10);
  fsm_trans *c = fsm_trans_copy(a);
  TEST_ASSERT(fsm_trans_check(c, L'a'));
  TEST_ASSERT(fsm_trans_check(c, L'b'));
  TEST_ASSERT(!fsm_trans_check(c, L'c'));
  fsm_trans_delete(a);
  fsm_trans_delete(c);
  return 0;
}

/**
   @brief Test that simple deterministic FSMs work.
 */
int test_simple_machine(void)
{
  fsm *f = fsm_create();
  int start = fsm_add_state(f, false);
  int end = fsm_add_state(f, true);
  fsm_trans *t = fsm_trans_create_single(L'a', L'a', FSM_TRANS_POSITIVE, end);

  f->start = start;
  fsm_add_trans(f, start, t);
  TEST_ASSERT(fsm_sim_det(f, L"a"));
  TEST_ASSERT(!fsm_sim_det(f, L"fail"));
  TEST_ASSERT(!fsm_sim_det(f, L""));
  fsm_delete(f, true);
  return 0;
}

void fsm_test(void)
{
  smb_ut_group *group = su_create_test_group("fsm");

  smb_ut_test *test_memory = su_create_test("test_memory", fsm_test_memory);
  su_add_test(group, test_memory);

  smb_ut_test *test_check = su_create_test("test_check", fsm_test_check);
  su_add_test(group, test_check);

  smb_ut_test *shortcut = su_create_test("shortcut", test_shortcut);
  su_add_test(group, shortcut);

  smb_ut_test *copy = su_create_test("copy", test_copy);
  su_add_test(group, copy);

  smb_ut_test *simple_machine = su_create_test("simple_machine", test_simple_machine);
  su_add_test(group, simple_machine);

  su_run_group(group);
  su_delete_group(group);
}
