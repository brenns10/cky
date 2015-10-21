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
#include "libstephen/cb.h"
#include "libstephen/str.h"
#include "libstephen/regex.h"
#include "lex.h"

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

void lex_destroy(smb_lex *obj, bool free_strings)
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
    if (free_strings) {
      smb_free(s);
    }
  }
  al_destroy(&obj->tokens);
}

void lex_delete(smb_lex *obj, bool free_strings) {
  lex_destroy(obj, free_strings);
  smb_free(obj);
}

void lex_add_token(smb_lex *obj, wchar_t *regex, DATA token)
{
  fsm *f = regex_parse(regex);
  al_append(&obj->patterns, (DATA){.data_ptr=f});
  al_append(&obj->tokens, token);
}

void lex_add_pattern(smb_lex *obj, wchar_t *regex, wchar_t *token)
{
  wchar_t *s = smb_new(wchar_t, wcslen(token) + 1);
  wcscpy(s, token);
  lex_add_token(obj, regex, PTR(token));
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
  lines = split_linesw(buf);
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

smb_lex_sim *lex_start(smb_lex *obj)
{
  int i;
  fsm_sim *fs;
  smb_status status;
  smb_lex_sim *sim = smb_new(smb_lex_sim, 1);
  al_init(&sim->simulations);

  // Create simulations for each pattern.
  for (i = 0; i < al_length(&obj->patterns); i++) {
    fs = fsm_sim_nondet_begin(al_get(&obj->patterns, i, &status).data_ptr);
    assert(status == SMB_SUCCESS);
    al_append(&sim->simulations, (DATA){.data_ptr=fs});
  }

  sim->last_pattern = -1;
  sim->last_index = -1;
  sim->finished = false;
  return sim;
}

bool lex_step(smb_lex *obj, smb_lex_sim *sim, wchar_t input)
{
  smb_status status = SMB_SUCCESS;
  fsm_sim *fs;
  int i, state;
  int curr_idx = sim->last_index + 1;
  // For each simulation...
  for (i = 0; i < al_length(&obj->patterns); i++) {
    // Drive forward the simulation, and get its state.
    fs = al_get(&sim->simulations, i, &status).data_ptr;
    assert(status == SMB_SUCCESS);
    fsm_sim_nondet_step(fs, input);
    // Use null byte as next input, since we don't know it yet.
    state = fsm_sim_nondet_state(fs, L'\0');

    // If the simulation is in an accepting state...
    if (state == FSM_SIM_ACCEPTING || state == FSM_SIM_ACCEPTED) {
      // And no other simulation has been accepting at this index...
      if (sim->last_index < curr_idx) {
        // Record this pattern and simulation.
        sim->last_pattern = i;
        sim->last_index = curr_idx;
      }
    }
  }

  // If none of the simulations were accepting after this character...
  if (sim->last_index < curr_idx) {
    // Then we don't need any more input.
    sim->finished = true;
  } else {
    // Otherwise, we still do need more input.
    sim->finished = false;
  }
  return sim->finished;
}

DATA lex_get_token(smb_lex *obj, smb_lex_sim *sim)
{
  smb_status status;
  DATA token;
  if (!sim->finished || sim->last_pattern < 0) {
    return (DATA){.data_ptr=NULL};
  } else {
    token = al_get(&obj->tokens, sim->last_pattern, &status);
    assert(status == SMB_SUCCESS);
    return token;
  }
}

int lex_get_length(smb_lex *obj, smb_lex_sim *sim)
{
  if (!sim->finished) {
    return -1;
  } else {
    return sim->last_index + 1;
  }
}

void lex_sim_delete(smb_lex_sim *sim)
{
  smb_status status = SMB_SUCCESS;
  int i;
  // Finally, destroy the simulations for each pattern.
  for (i = 0; i < al_length(&sim->simulations); i++) {
    fsm_sim_delete(al_get(&sim->simulations, i, &status).data_ptr, true);
    assert(status == SMB_SUCCESS);
  }
  al_destroy(&sim->simulations);
  smb_free(sim);
}

void lex_yylex(smb_lex *obj, wchar_t *input, DATA *token, int *length,
               smb_status *status)
{
  smb_lex_sim *sim = lex_start(obj);

  while (!sim->finished) {
    lex_step(obj, sim, *input++);
  }

  *token = lex_get_token(obj, sim);
  *length = lex_get_length(obj, sim);
  lex_sim_delete(sim);
}

wchar_t *lex_fyylex(smb_lex *obj, FILE *input, DATA *token, int *length,
                    smb_status *status)
{
  smb_lex_sim *sim = lex_start(obj);
  wcbuf wcb;
  wchar_t curr;
  wcb_init(&wcb, 128);

  while (!sim->finished) {
    curr = fgetwc(input);
    wcb_append(&wcb, curr);
    lex_step(obj, sim, curr);
  }

  // The last character is never part of the token, so get rid of it.
  wcb.buf[wcb.length-1] = L'\0';
  ungetwc(curr, input);

  *token = lex_get_token(obj, sim);
  *length = lex_get_length(obj, sim);
  lex_sim_delete(sim);
  return wcb.buf;
}
