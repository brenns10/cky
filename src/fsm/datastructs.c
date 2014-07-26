/***************************************************************************//**

  @file         datastructs.c

  @author       Stephen Brennan

  @date         Created Saturday, 26 July 2014

  @brief        Data structure function declarations for FSMs.

  @copyright    Copyright (c) 2014, Stephen Brennan.
  All rights reserved.

  @copyright
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Stephen Brennan nor the names of his contributors may
      be used to endorse or promote products derived from this software without
      specific prior written permission.

  @copyright
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL STEPHEN BRENNAN BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <stdlib.h>     // malloc, free
#include <stdio.h>      // fprintf, stderr, etc...

#include "fsm.h"
#include "libstephen.h"

////////////////////////////////////////////////////////////////////////////////
// fsm_trans Fundamental Functions
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Initialize an already-allocated FSM transition unit.

   An FSM transition unit defines what characters can and can't be accepted for
   a transition, similar to a regular expression's character class.  It can
   either be a positive transition (any character between start and end), or a
   negative transition (any character not between start and end).  It can have
   more than one transition criteria (i.e character range), but they all have to
   be either positive or negative.

   @param ft The pre-allocated transition
   @param n The number of transition criteria
   @param type The type of transition (positive or negative)
   @param dest The destination of the transition
 */
void fsm_trans_init(fsm_trans *ft, int n, int type, int dest)
{
  int i;
  ft->type = type;

  // Allocate space for the start range, plus null terminator
  ft->start = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->start) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation "
            "failed.\n");
    return;
  }

  // Allocate space for the end range, plus null terminator
  ft->end = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->end) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation "
            "failed.\n");
    free(ft->start);
    return;
  }

  for (i = 0; i < n; i++) {
    ft->start[i] = (wchar_t) EOF;
    ft->end[i] = (wchar_t) EOF;
  }
  ft->start[n] = L'\0';
  ft->end[n] = L'\0';

  ft->dest = dest;

  // Count the allocated space.
  SMB_INCREMENT_MALLOC_COUNTER(2 * (n + 1) * sizeof(wchar_t));
}

/**
   @brief Allocate and initialize a fsm_trans object.

   @param n The number of ranges
   @param type The type of transition
   @param dest The destination of the transition
 */
fsm_trans *fsm_trans_create(int n, int type, int dest)
{
  // Allocate the space
  fsm_trans *ft = (fsm_trans *) malloc(sizeof(fsm_trans));
  CLEAR_ALL_ERRORS;

  // Check for allocation error
  if (!ft) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }

  // Run initialization
  fsm_trans_init(ft, n, type, dest);

  // If the initialization failed, free and return
  if (CHECK(ALLOCATION_ERROR)) {
    free(ft);
    return NULL;
  }

  // Otherwise, count the memory and return it.
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(fsm_trans));
  return ft;
}

/**
   @brief Clean up the fsm_trans object.  Do not free it.

   @param ft The object to clean up
 */
void fsm_trans_destroy(fsm_trans *ft)
{
  if (ft && ft->start && ft->end) {
    int len = wcslen(ft->start); // assume len(start) == len(end)
    free(ft->start);
    free(ft->end);
    SMB_DECREMENT_MALLOC_COUNTER(2 * (len+1) * sizeof(wchar_t));
  } else {
    fprintf(stderr, "libstephen: fsm_trans_destroy() called with null "
            "pointers.\n");
  }
}

/**
   @brief Free the fsm_trans object.

   @param ft The object to free
 */
void fsm_trans_delete(fsm_trans *ft)
{
  if (ft) {
    fsm_trans_destroy(ft);
    free(ft);
    SMB_DECREMENT_MALLOC_COUNTER(sizeof(fsm_trans));
  } else {
    fprintf(stderr, "libstephen: fsm_trans_delete() called with null "
            "pointer.\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
// fsm_trans Utilities
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Initialize a fsm_trans object with a single range.

   @param ft The object to initialize
   @param start The beginning of the range
   @param end The end of the range
   @param type The type of the object
   @param dest The destination of the transition
 */
void fsm_trans_init_single(fsm_trans *ft, wchar_t start, wchar_t end, int type,
                           int dest)
{
  fsm_trans_init(ft, 1, type, dest);
  if (end < start)
    fprintf(stderr, "Error: initialization of invalid range in FSM.\n");
  ft->start[0] = start;
  ft->end[0] = end;
}

/**
   @brief Allocate and initialize a fsm_trans object with a single range.

   @param start The beginning of the range
   @param end The end of the range
   @param type The type of the object
   @param dest The destination of the transition
 */
fsm_trans *fsm_trans_create_single(wchar_t start, wchar_t end, int type, 
                                   int dest)
{
  fsm_trans *ft = fsm_trans_create(1, type, dest);
  if (end < start)
    fprintf(stderr, "Error: initialization of invalid range in FSM.\n");
  ft->start[0] = start;
  ft->end[0] = end;
  return ft;
}

/**
   Check whether the character is accepted by the transition.

   @param ft The transition to check
   @param c The character to check
 */
bool fsm_trans_check(const fsm_trans *ft, wchar_t c)
{
  wchar_t *start = ft->start, *end = ft->end;

  while (start && end && *start != '\0' && *end != '\0') {
    if (c >= *start && c <= *end) {
      if (ft->type == FSM_TRANS_POSITIVE)
        return true;
      else
        return false;
    }
    start++; end++;
  }

  if (ft->type == FSM_TRANS_POSITIVE)
    return false;
  else
    return true;
}

/**
   @brief Allocate an entirely new copy of an existing FSM transition.
   @param ft The transition to copy
   @return The copy of ft
 */
fsm_trans *fsm_trans_copy(const fsm_trans *ft)
{
  int i, rangeSize = wcslen(ft->start);
  fsm_trans *new = fsm_trans_create(rangeSize, ft->type, ft->dest);
  for (i = 0; i < rangeSize; i++) {
    new->start[i] = ft->start[i];
    new->end[i] = ft->end[i];
  }
  return new;
}

////////////////////////////////////////////////////////////////////////////////
// fsm Fundamental Functions
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Initialize a FSM.

   @param f The FSM to Initialize
 */
void fsm_init(fsm *f)
{
  CLEAR_ALL_ERRORS;
  f->start = -1;
  al_init(&f->transitions);
  if (CHECK(ALLOCATION_ERROR))
    return;
  al_init(&f->accepting);
  if (CHECK(ALLOCATION_ERROR)) {
    al_destroy(&f->transitions);
    return;
  }
}

/**
   @brief Allocate and initialize an FSM.
 */
fsm *fsm_create(void)
{
  fsm *f = (fsm *) malloc(sizeof(fsm));
  CLEAR_ALL_ERRORS;

  if (!f) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }

  fsm_init(f);
  if (CHECK(ALLOCATION_ERROR)) {
    free(f);
  }

  SMB_INCREMENT_MALLOC_COUNTER(sizeof(fsm));
  return f;
}

/**
   @brief Clean up a FSM, but do not free it.

   @param f The FSM to clean up
   @param free_transitions Do we free the transitions too?
 */
void fsm_destroy(fsm *f, bool free_transitions)
{
  int i, j;
  smb_al *list;

  for (i = 0; i < al_length(&f->transitions); i++) {
    list = (smb_al *) al_get(&f->transitions, i).data_ptr;
    if (free_transitions) {
      for (j = 0; j < al_length(list); j++) {
        fsm_trans_delete((fsm_trans *) al_get(list, j).data_ptr);
      }
    }
    al_delete(list);
  }

  al_destroy(&f->transitions);
  al_destroy(&f->accepting);
}

/**
   @brief Clean up and free an FSM.

   @param f The FSM to free
   @param free_transitions Do we free the transitions too?
 */
void fsm_delete(fsm *f, bool free_transitions)
{
  fsm_destroy(f, free_transitions);
  free(f);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(fsm));
}

////////////////////////////////////////////////////////////////////////////////
// fsm Utilities
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Creates a FSM for a single character.
   @param character The character to make a transition for.
   @returns FSM for the character.
 */
fsm *fsm_create_single_char(wchar_t character)
{
  fsm *f = fsm_create();
  int s0 = fsm_add_state(f, false);
  int s1 = fsm_add_state(f, true);
  fsm_add_single(f, s0, s1, character, character, FSM_TRANS_POSITIVE);
  f->start= s0;
  return f;
}

/**
   @brief Add a state to the FSM
   @param f The FSM to add to
   @param accepting Whether to add the state to the accepting list
   @return The index of the state
 */
int fsm_add_state(fsm *f, bool accepting)
{
  DATA d;
  int index;
  d.data_ptr = (void *) al_create();
  al_append(&f->transitions, d);
  index = al_length(&f->transitions) - 1;
  if (accepting) {
    d.data_llint = index;
    al_append(&f->accepting, d);
  }
  return index;
}

/**
   @brief Add a transition to the FSM.

   @param f The FSM to add the transition to
   @param state The state the transition is from
   @param ft The transition to add
 */
void fsm_add_trans(fsm *f, int state, const fsm_trans *ft)
{
  smb_al *list = (smb_al *) al_get(&f->transitions, state).data_ptr;
  DATA d;
  d.data_ptr = (void *) ft;
  al_append(list, d);
}

/**
   @brief Add a single range transition to the FSM.

   This function simplifies the allocation and addition of the more common
   single range transitions.  It returns the fsm_trans pointer if the programmer
   wishes to do manual memory management.  However, the return value can be
   ignored, and false passed to fsm_delete() in order to have the transition
   freed automatically.

   @param f The FSM to add to
   @param from The state the transition is from
   @param to The state the transition is to
   @param start The character at the start of the range
   @param end The character at the end of the range
   @param type The type of range (positive or negative)
   @return Pointer to the fsm_trans created by the function.
 */
fsm_trans *fsm_add_single(fsm *f, int from, int to, wchar_t start, wchar_t end,
                          int type)
{
  fsm_trans *new = fsm_trans_create_single(start, end, type, to);
  fsm_add_trans(f, from, new);
  return new;
}

////////////////////////////////////////////////////////////////////////////////
// fsm_sim Fundamental Functions
////////////////////////////////////////////////////////////////////////////////

/**
   @brief Initialize an fsm_sim struct
   @param fs The struct to init
   @param f The FSM to simulate 
   @param curr The current state of the simulation
   @param input The input string
 */
void fsm_sim_init(fsm_sim *fs, fsm *f, smb_al *curr, const wchar_t *input)
{
  fs->f = f;
  fs->curr = curr;
  fs->input = input;
}

/**
   @brief Allocate and initialize an fsm_sim struct
   @param f The FSM to simulate 
   @param curr The current state of the simulation
   @param input The input string
   @return The allocated simulation
 */
fsm_sim *fsm_sim_create(fsm *f, smb_al *curr, const wchar_t *input)
{
  fsm_sim *fs = (fsm_sim *) malloc(sizeof(fsm_sim));

  if (!fs) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }
  SMB_INCREMENT_MALLOC_COUNTER(sizeof(fsm_sim));

  fsm_sim_init(fs, f, curr, input);
  return fs;
}

/**
   @brief Clean up any resources held by the fsm_sim
   @param fs The simulation
   @param free_curr Do we free the state list (generally, true)
 */
void fsm_sim_destroy(fsm_sim *fs, bool free_curr)
{
  if (free_curr) {
    al_delete(fs->curr);
  }
}

/**
   @brief Clean up and any resources held by the fsm_sim, and free it.
   @param fs The simulation
   @param free_curr Do we free the state list (generally, true)
 */
void fsm_sim_delete(fsm_sim *fs, bool free_curr)
{
  fsm_sim_destroy(fs, free_curr);
  free(fs);
  SMB_DECREMENT_MALLOC_COUNTER(sizeof(fsm_sim));
}
