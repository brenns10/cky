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
  TEST_ASSERT(al_length(&lex->patterns) == 1);
  lex_delete(lex);
  return 0;
}

static int test_comment(void)
{
  smb_status status = SMB_SUCCESS;
  wchar_t *config =
    L"[a-zA-Z_]\\w*\tidentifier\n"
    L"# this is a comment!\n";
  smb_lex *lex = lex_create();
  lex_load(lex, config, &status);
  TEST_ASSERT(al_length(&lex->patterns) == 1);
  lex_delete(lex);
  return 0;
}

static int test_multiple(void)
{
  smb_status status = SMB_SUCCESS;
  wchar_t *config =
    L"[a-zA-Z_]\\w*\tidentifier\n"
    L"\\d+\tinteger";
  smb_lex *lex = lex_create();
  lex_load(lex, config, &status);
  TEST_ASSERT(al_length(&lex->patterns) == 2);
  lex_delete(lex);
  return 0;
}

void lex_test(void)
{
  smb_ut_group *group = su_create_test_group("lex");

  smb_ut_test *load_single = su_create_test("load_single", test_load_single);
  su_add_test(group, load_single);

  smb_ut_test *comment = su_create_test("comment", test_comment);
  su_add_test(group, comment);

  smb_ut_test *multiple = su_create_test("multiple", test_multiple);
  su_add_test(group, multiple);

  su_run_group(group);
  su_delete_group(group);
}
