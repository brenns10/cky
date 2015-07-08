/***************************************************************************//**

  @file         lextest.c

  @author       Stephen Brennan

  @date         Created Monday,  6 July 2015

  @brief        Test for lexer

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <wchar.h>

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

static int test_simple_lex(void)
{
  smb_status status = SMB_SUCCESS;
  int length, idx = 0;
  DATA token;
  wchar_t *config =
    L"[a-zA-Z_]\\w*\tidentifier\n"
    L"\\d+\tinteger\n"
    L"\\+\tADD\n"
    L"\\-\tSUBTRACT\n"
    L"\\s+\twhitespace\n";
  wchar_t *test = L"var-12+ id3";
  smb_lex *lex = lex_create();
  lex_load(lex, config, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(al_length(&lex->patterns) == 5);

  lex_yylex(lex, test, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 3);
  TEST_ASSERT(wcscmp(token.data_ptr, L"identifier") == 0);
  idx += length;

  lex_yylex(lex, test+idx, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 1);
  TEST_ASSERT(wcscmp(token.data_ptr, L"SUBTRACT") == 0);
  idx += length;

  lex_yylex(lex, test+idx, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 2);
  TEST_ASSERT(wcscmp(token.data_ptr, L"integer") == 0);
  idx += length;

  lex_yylex(lex, test+idx, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 1);
  TEST_ASSERT(wcscmp(token.data_ptr, L"ADD") == 0);
  idx += length;

  lex_yylex(lex, test+idx, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 1);
  TEST_ASSERT(wcscmp(token.data_ptr, L"whitespace") == 0);
  idx += length;

  lex_yylex(lex, test+idx, &token, &length, &status);
  TEST_ASSERT(status == SMB_SUCCESS);
  TEST_ASSERT(length == 3);
  TEST_ASSERT(wcscmp(token.data_ptr, L"identifier") == 0);

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

  smb_ut_test *simple_lex = su_create_test("simple_lex", test_simple_lex);
  su_add_test(group, simple_lex);

  su_run_group(group);
  su_delete_group(group);
}
