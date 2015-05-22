/***************************************************************************//**

  @file         fsm_sim.c

  @author       Stephen Brennan

  @date         Friday, 22 May 2015

  @brief        Test fsm_sim functions.

*******************************************************************************/

#include "libstephen/ut.h"
#include "libstephen/al.h"
#include "fsm.h"

/**
   @brief Test fsm_sim for memory leaks (use Valgrind!).
 */
static int test_memory(void)
{
  fsm f;
  fsm_sim *pfs;
  smb_al *state;

  // Dummy resources for the FSM simulation.
  state = al_create();
  fsm_init(&f);

  // Test is to make sure that these work properly.
  pfs = fsm_sim_create(&f, state, L"abctest");
  fsm_sim_delete(pfs, true);

  // Only need to destroy the FSM, since the "curr" state was freed.
  fsm_destroy(&f, true);
  return 0;
}

void fsm_sim_test(void)
{
  smb_ut_group *group = su_create_test_group("fsm_sim");

  smb_ut_test *memory = su_create_test("memory", test_memory);
  su_add_test(group, memory);

  su_run_group(group);
  su_delete_group(group);
}
