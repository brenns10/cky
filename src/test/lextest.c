/***************************************************************************//**

  @file         lextest.c

  @author       Stephen Brennan

  @date         Created Monday,  6 July 2015

  @brief        Test for lexer

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "libstephen/ut.h"
#include "lex.h"

static int test_load_single(void)
{
  smb_status status = SMB_SUCCESS;
  wchar_t *config = L"[a-zA-Z_]\\w*\tidentifier";
  smb_lex *lex = lex_create();
  lex_load(lex, config, &status);
  lex_delete(lex);
  return 0;
}

void lex_test(void)
{
  smb_ut_group *group = su_create_test_group("lex");

  smb_ut_test *load_single = su_create_test("load_single", test_load_single);
  su_add_test(group, load_single);

  su_run_group(group);
  su_delete_group(group);
}
