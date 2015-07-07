/***************************************************************************//**

  @file         lex.c

  @author       Stephen Brennan

  @date         Created Monday,  6 July 2015

  @brief        Lexer

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include "libstephen/al.h"
#include "regex.h"
#include "lex.h"
#include "str.h"

void lex_init(smb_lex *obj)
{
  // Initialization logic
  al_init(&obj->patterns);
  al_init(&obj->tokens);
}

smb_lex *lex_create(void)
{
  smb_lex *obj = smb_new(smb_lex, 1);
  lex_init(obj);
  return obj;
}

void lex_destroy(smb_lex *obj)
{
  // Cleanup logic
  smb_iter it;
  fsm *f;
  wchar_t *s;
  smb_status status = SMB_SUCCESS;

  it = al_get_iter(&obj->patterns);
  while (it.has_next(&it)) {
    f = it.next(&it, &status).data_ptr;
    assert(status == SMB_SUCCESS);
    fsm_delete(f, true);
  }
  al_destroy(&obj->patterns);

  it = al_get_iter(&obj->tokens);
  while (it.has_next(&it)) {
    s = it.next(&it, &status).data_ptr;
    assert(status == SMB_SUCCESS);
    smb_free(s); // assumes we can free the string, may change that.
  }
  al_destroy(&obj->tokens);
}

void lex_delete(smb_lex *obj) {
  lex_destroy(obj);
  smb_free(obj);
}

void lex_add_pattern(smb_lex *obj, wchar_t *regex, wchar_t *token)
{
  fsm *f = regex_parse(regex);
  wchar_t *s = smb_new(wchar_t, wcslen(token) + 1);
  al_append(&obj->patterns, (DATA){.data_ptr=f});
  wcscpy(s, token);
  al_append(&obj->tokens, (DATA){.data_ptr=s});
}

static void lex_load_line(smb_lex *obj, wchar_t *line, smb_status *status)
{
  wchar_t *token;
  int i = 0;
  while (line[i] != L'\0' && line[i] != L'\t') {
    i++;
  }

  if (line[i] != L'\t') {
    *status = SMB_INDEX_ERROR;  //TODO: more reasonable error message
    return;
  }

  // End the string so we can parse the regular expression.
  line[i] = L'\0';
  token = line + i + 1;
  lex_add_pattern(obj, line, token);
}

void lex_load(smb_lex *obj, const wchar_t *str, smb_status *status)
{
  smb_ll *lines;
  smb_iter iter;
  wchar_t *buf = smb_new(wchar_t, wcslen(str) + 1);
  wchar_t *line;
  wcscpy(buf, str);
  lines = split_lines(buf);
  iter = ll_get_iter(lines);

  while (iter.has_next(&iter)) {
    line = iter.next(&iter, status).data_ptr;
    assert(*status == SMB_SUCCESS);
    fflush(stdout);
    if (line[0] != L'#') {
      lex_load_line(obj, line, status);
      if (*status != SMB_SUCCESS) {
        goto cleanup;
      }
    }
  }
 cleanup:
  smb_free(buf);
  ll_delete(lines);
}
