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
 */
int fsm_test_memory(void)
{
  fsm_trans fsm_trans_stack;
  fsm_trans *fsm_trans_heap;
  fsm fsm_stack;
  fsm *fsm_heap;

  fsm_trans_init(&fsm_trans_stack, 3, FSM_TRANS_POSITIVE, 12);
  fsm_trans_heap = fsm_trans_create(3, FSM_TRANS_POSITIVE, 12);
  fsm_trans_destroy(&fsm_trans_stack);
  fsm_trans_delete(fsm_trans_heap);

  fsm_init(&fsm_stack);
  fsm_heap = fsm_create();
  fsm_destroy(&fsm_stack, true);
  fsm_delete(fsm_heap, true);
  return 0;
}

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

void fsm_test(void)
{
  smb_ut_group *group = su_create_test_group("fsm");

  smb_ut_test *test_memory = su_create_test("test_memory", fsm_test_memory);
  su_add_test(group, test_memory);

  smb_ut_test *test_check = su_create_test("test_check", fsm_test_check);
  su_add_test(group, test_check);

  su_run_group(group);
  su_delete_group(group);
}
