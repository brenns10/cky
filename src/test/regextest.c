/***************************************************************************//**

  @file         regextest.c

  @author       Stephen Brennan

  @date         Created Sunday, 24 May 2015

  @brief        Tests for stuff under regex/.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/ut.h"
#include "fsm.h"
#include "regex.h"

static int test_memory(void)
{
  regex_hit *hit = regex_hit_create(0, 5);
  TEST_ASSERT(hit->start == 0);
  TEST_ASSERT(hit->length == 5);
  regex_hit_delete(hit);
  return 0;
}

static int test_single(void)
{
  fsm *f = regex_parse(L"a");
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"b"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"c"));
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(!fsm_sim_nondet(f, L"abcd"));
  fsm_delete(f, true);
  return 0;
}

static int test_multiple(void)
{
  fsm *f = regex_parse(L"abcd");
  TEST_ASSERT(fsm_sim_nondet(f, L"abcd"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"abcde"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"abc"));
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(!fsm_sim_nondet(f, L"blah"));
  fsm_delete(f, true);
  return 0;
}

static int test_character_class(void)
{
  fsm *f = regex_parse(L"[abcd]");
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(fsm_sim_nondet(f, L"b"));
  TEST_ASSERT(fsm_sim_nondet(f, L"c"));
  TEST_ASSERT(fsm_sim_nondet(f, L"d"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"e"));
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(!fsm_sim_nondet(f, L"abcd"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"uuuu"));
  fsm_delete(f, true);
  return 0;
}

static int test_subexpression(void)
{
  fsm *f = regex_parse(L"(a|b|c|d)");
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(fsm_sim_nondet(f, L"b"));
  TEST_ASSERT(fsm_sim_nondet(f, L"c"));
  TEST_ASSERT(fsm_sim_nondet(f, L"d"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"e"));
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(!fsm_sim_nondet(f, L"abcd"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"uuuu"));
  fsm_delete(f, true);
  return 0;
}

static int test_plus(void)
{
  fsm *f = regex_parse(L"a+");
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(fsm_sim_nondet(f, L"aa"));
  TEST_ASSERT(fsm_sim_nondet(f, L"aaa"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ab"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ba"));
  fsm_delete(f, true);
  return 0;
}

static int test_kleene(void)
{
  fsm *f = regex_parse(L"a*");
  TEST_ASSERT(fsm_sim_nondet(f, L""));
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(fsm_sim_nondet(f, L"aa"));
  TEST_ASSERT(fsm_sim_nondet(f, L"aaa"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ab"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ba"));
  fsm_delete(f, true);
  return 0;
}

static int test_dot(void)
{
  fsm *f = regex_parse(L".");
  TEST_ASSERT(!fsm_sim_nondet(f, L""));
  TEST_ASSERT(fsm_sim_nondet(f, L"a"));
  TEST_ASSERT(fsm_sim_nondet(f, L"b"));
  TEST_ASSERT(fsm_sim_nondet(f, L"!"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ab"));
  TEST_ASSERT(!fsm_sim_nondet(f, L"ba"));
  fsm_delete(f, true);
  return 0;
}

void regex_test(void)
{
  smb_ut_group *group = su_create_test_group("regex");

  smb_ut_test *memory = su_create_test("memory", test_memory);
  su_add_test(group, memory);

  smb_ut_test *single = su_create_test("single", test_single);
  su_add_test(group, single);

  smb_ut_test *multiple = su_create_test("multiple", test_multiple);
  su_add_test(group, multiple);

  smb_ut_test *character_class = su_create_test("character_class", test_character_class);
  su_add_test(group, character_class);

  smb_ut_test *subexpression = su_create_test("subexpression", test_subexpression);
  su_add_test(group, subexpression);

  smb_ut_test *plus = su_create_test("plus", test_plus);
  su_add_test(group, plus);

  smb_ut_test *kleene = su_create_test("kleene", test_kleene);
  su_add_test(group, kleene);

  smb_ut_test *dot = su_create_test("dot", test_dot);
  su_add_test(group, dot);

  su_run_group(group);
  su_delete_group(group);
}
