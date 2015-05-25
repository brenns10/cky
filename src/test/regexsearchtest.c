/***************************************************************************//**

  @file         regexsearchtest.c

  @author       Stephen Brennan

  @date         Created Sunday, 24 May 2015

  @brief        Test regex search functions.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/ut.h"
#include "regex.h"

int test_simple_search(void)
{
  smb_status status = SMB_SUCCESS;
  wchar_t *search_text = L"words words words";
  smb_al *results = regex_search(L"\\w+", search_text, false, false);
  regex_hit *hit;
  int starts[] = {0, 6, 12};
  int lengths[] = {5, 5, 5};
  int i;
  TEST_ASSERT(al_length(results) == sizeof(starts)/sizeof(int));
  for (i = 0; i < al_length(results); i++) {
    hit = al_get(results, i, &status).data_ptr;
    TEST_ASSERT(status == SMB_SUCCESS);
    TEST_ASSERT(hit->start == starts[i]);
    TEST_ASSERT(hit->length == lengths[i]);
    regex_hit_delete(hit);
  }
  al_delete(results);
  return 0;
}

void regex_search_test(void)
{
  smb_ut_group *group = su_create_test_group("regex_search");

  smb_ut_test *simple_search = su_create_test("simple_search", test_simple_search);
  su_add_test(group, simple_search);

  su_run_group(group);
  su_delete_group(group);
}
