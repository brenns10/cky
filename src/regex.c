/***************************************************************************//**

  @file         regex.c

  @author       Stephen Brennan

  @date         Created Sunday, 18 May 2014

  @brief        Definitions for regular expression routines.

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

#include <stdbool.h>
#include <stdio.h>

#include "regex.h"      // the functions we are defining
#include "fsm.h"        // tools to implement regex
#include "libstephen.h" // for smb_al

/**
   @brief Copy the transitions and states from src into dest.

   All the states are added, and the transitions are modified with the offset
   determined from the initial number of states in the destination.
 */
static void fsm_copy_trans(fsm *dest, const fsm *src)
{
  int offset = al_length(&dest->transitions);
  int i,j;
  // Copy the transitions from the source one, altering their start and
  // destination to fit with the new state numbering.
  for (i = 0; i < al_length(&src->transitions); i++) {
    // Add new state corresponding to this one
    fsm_add_state(dest, false);
    // For each transition out of this state, add a copy to dest.
    const smb_al *list = al_get(&src->transitions, i).data_ptr;
    for (j = 0; j < al_length(list); j++) {
      const fsm_trans *old = al_get(list, j).data_ptr;
      fsm_trans *new = fsm_trans_copy(old);
      new->dest = new->dest + offset;
      fsm_add_trans(dest, i + offset, new);
    }
  }
}

/**
   @brief Concatenate two FSMs into the first FSM object.

   This function concatenates two FSMs.  This means it constructs a machine that
   will accept a string where the first part is accepted by the first machine,
   and the second part is accepted by the second machine.  It is done by hooking
   up the accepting states of the first machine to the start state of the second
   (via epsilon transition).  Then, only the second machines's accepting states
   are used for the final product.

   Note that the concatenation is done in-place into the first FSM.  If you
   don't wish for that to happen, use fsm_copy() to copy the first one, and then
   use fsm_concat() to concatenate into the copy.

   @param first The first FSM.  Is modified to contain the concatenation.
   @param second The second FSM.  It is not modified.
 */
void fsm_concat(fsm *first, const fsm *second)
{
  int i, offset = al_length(&first->transitions);
  DATA d;
  // Create a transition from each accepting state into the second machine's
  // start state.
  for (i = 0; i < al_length(&first->accepting); i++) {
    int start = (int)al_get(&first->accepting, i).data_llint;
    // This could be outside the loop, but we need a new one for each instance
    fsm_trans *ft = fsm_trans_create_single(EPSILON, EPSILON,
                                            FSM_TRANS_POSITIVE,
                                            second->start + offset);
    fsm_add_trans(first, start, ft);
  }

  fsm_copy_trans(first, second);

  // Replace the accepting states of the first with the second.
  al_destroy(&first->accepting);
  al_init(&first->accepting);
  for (i = 0; i < al_length(&second->accepting); i++) {
    d = al_get(&second->accepting, i);
    d.data_llint += offset;
    al_append(&first->accepting, d);
  }
}

/**
   @brief Construct the union of the two FSMs into the first one.

   This function creates an FSM that will accept any string accepted by the
   first OR the second one.  The FSM is constructed in place of the first
   machine, modifying the parameter.  If this is not what you want, make a copy
   of the first one, and pass the copy instead.

   @param first The first FSM to union.  Will be modified with the result.
   @param second The second FSM to union
 */
void fsm_union(fsm *first, const fsm *second)
{
  int newStart, i, offset = al_length(&first->transitions);
  fsm_trans *fsTrans, *ssTrans;
  DATA d;

  // Add the transitions from the second into the first.
  fsm_copy_trans(first, second);

  // Create a new start state!
  newStart = fsm_add_state(first, false);

  // Add epsilon-trans from new start to first start
  fsTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE,
                                    first->start);
  fsm_add_trans(first, newStart, fsTrans);

  // Add epsilon-trans from new start to second start
  ssTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE,
                                    second->start + offset);
  fsm_add_trans(first, newStart, ssTrans);

  // Accept from either the first or the second
  for (i = 0; i < al_length(&second->accepting); i++) {
    d = al_get(&second->accepting, i);
    d.data_llint += offset;
    al_append(&first->accepting, d);
  }

  // Change the start state
  first->start = newStart;
}

/**
   @brief Modify an FSM to accept 0 or more occurrences of it.

   This modifies the existing FSM to accept L*, where L is the language accepted
   by the machine.

   @param f The FSM to perform Kleene star on
 */
void fsm_kleene(fsm *f)
{
  int i, newStart = fsm_add_state(f, false); // not accepting yet
  fsm_trans *newTrans;
  DATA d;

  // Add epsilon-trans from new start to first start
  newTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE,
                                     f->start);
  fsm_add_trans(f, newStart, newTrans);

  // For each accepting state, add a epsilon-trans to the new start
  for (i = 0; i < al_length(&f->accepting); i++) {
    newTrans = fsm_trans_create_single(EPSILON, EPSILON, FSM_TRANS_POSITIVE,
                                       newStart);
    fsm_add_trans(f, (int) al_get(&f->accepting, i).data_llint, newTrans);
  }

  // Make the new start accepting
  d.data_llint = newStart;
  al_append(&f->accepting, d);

  // Change the start state (officially)
  f->start = newStart;
}

/**
   @brief Adjust the FSM according to its modifier, if any.

   When a character, character class, or parenthesized regex is read in, it
   could be followed by the modifiers `*`, `+`, or `?`.  This function adjusts
   the FSM for those modifiers, and adjusts the location pointer if one was
   present.

   @param new The newly read in FSM.
   @param regex The pointer to the pointer to the location in the regex.
 */
void check_modifier(fsm *new, const wchar_t **regex)
{
  fsm *f;

  switch ((*regex)[1]) {

  case L'*':
    fsm_kleene(new);
    (*regex)++;
    break;

  case L'+':
    f = fsm_copy(new);
    fsm_kleene(f);
    fsm_concat(new, f);
    fsm_delete(f, true);
    (*regex)++;
    break;

  case L'?':
    // Create the machine that accepts the empty string
    f = fsm_create();
    f->start = fsm_add_state(f, true);
    fsm_union(new, f);
    fsm_delete(f, true);
    (*regex)++;
    break;
  }
}

/**
   @brief Return a FSM that accepts any whitespace.
   @param type Positive or negative.
 */
fsm *whitespace_fsm(int type)
{
  fsm *f = fsm_create();
  int src = fsm_add_state(f, false);
  int dest = fsm_add_state(f, true);
  fsm_trans *ft = fsm_trans_create(6, type, dest);
  ft->start[0] = L' ';
  ft->end[0] = L' ';
  ft->start[1] = L'\f';
  ft->end[1] = L'\f';
  ft->start[2] = L'\n';
  ft->end[2] = L'\n';
  ft->start[3] = L'\r';
  ft->end[3] = L'\r';
  ft->start[4] = L'\t';
  ft->end[4] = L'\t';
  ft->start[5] = L'\v';
  ft->end[5] = L'\v';
  fsm_add_trans(f, src, ft);
  f->start = src;
  return f;
}

/**
   @brief Return a FSM for word characters (letters, numbers, and underscore).
 */
fsm *word_fsm(int type)
{
  fsm *f = fsm_create();
  int src = fsm_add_state(f, false);
  int dest = fsm_add_state(f, true);
  fsm_trans *ft = fsm_trans_create(4, type, dest);
  ft->start[0] = L'a';
  ft->end[0] = L'z';
  ft->start[1] = L'A';
  ft->end[1] = L'Z';
  ft->start[2] = L'_';
  ft->end[2] = L'_';
  ft->start[3] = L'0';
  ft->end[3] = L'9';
  fsm_add_trans(f, src, ft);
  f->start = src;
  return f;
}

/**
   @brief Return a FSM for digits.
 */
fsm *digit_fsm(int type)
{
  fsm *f = fsm_create();
  int src = fsm_add_state(f, false);
  int dest = fsm_add_state(f, true);
  fsm_trans *ft = fsm_trans_create(1, type, dest);
  ft->start[0] = L'0';
  ft->end[0] = L'9';
  fsm_add_trans(f, src, ft);
  f->start = src;
  return f;
}

/**
   @brief Returns an FSM for an escape sequence, outside of a character class.

   Basically, adds the \W, \w, \D, \d, \S, \s character classes to the already
   existing character escape sequences covered by get_escape().

   Expects that `*regex` points to the backslash in the escape sequence.  Always
   returns such that `*regex` points to the LAST character in the escape
   sequence.

   @param regex Pointer to the pointer to the backslash escape.
   @return FSM to accept the backslash escape sequence.
 */
fsm *escaped_fsm(const wchar_t **regex)
{
  char c;

  (*regex)++; // advance to the specifier
  switch (**regex) {
  case L's':
    return whitespace_fsm(FSM_TRANS_POSITIVE);
  case L'S':
    return whitespace_fsm(FSM_TRANS_NEGATIVE);
  case L'w':
    return word_fsm(FSM_TRANS_POSITIVE);
  case L'W':
    return word_fsm(FSM_TRANS_NEGATIVE);
  case L'd':
    return digit_fsm(FSM_TRANS_POSITIVE);
  case L'D':
    return digit_fsm(FSM_TRANS_NEGATIVE);
  default:
    c = get_escape(regex, L'e');
    // get_escape() leaves the pointer AFTER the last character in the escape.
    // We want it ON the last one.
    (*regex)--;
    return fsm_create_single_char(c);
  }
}

/**
   @brief Create a FSM for a character class.

   Reads a character class (pointed by `*regex`), which it then converts to a
   single transition FSM.

   @param regex Pointer to pointer to location in string!
   @returns FSM for character class.
 */
fsm *create_char_class(const wchar_t **regex)
{
  #define NORMAL 0
  #define RANGE 1

  smb_ll start, end;
  DATA d;
  int type = FSM_TRANS_POSITIVE, state = NORMAL, index = 0;
  smb_ll_iter iter;
  fsm *f;
  int src, dest;
  fsm_trans *ft;

  ll_init(&start);
  ll_init(&end);

  // Detect whether the character class is positive or negative
  (*regex)++;
  if (**regex == L'^') {
    type = FSM_TRANS_NEGATIVE;
    (*regex)++;
  }

  // Loop through characters in the character class, recording each start-end
  // pair for the transition.
  for ( ; **regex != L']'; (*regex)++) {
    if (**regex == L'-') {
      state = RANGE;
    } else {
      // Get the correct character
      if (**regex == L'\\') {
        printf("Read escape.\n");
        (*regex)++;
        d.data_llint = get_escape(regex, L'e');
        (*regex)--;
      } else {
        d.data_llint = **regex;
      }
      // Put it in the correct place
      if (state == NORMAL) {
        ll_append(&start, d);
        ll_append(&end, d);
      } else {
        // Modify the last transition if this is a range
        ll_set(&end, ll_length(&end)-1, d);
        state = NORMAL;
      }
    }
  }

  if (state == RANGE) {
    // The last hyphen was meant to be literal.  Yay!
    d.data_llint = L'-';
    ll_append(&start, d);
    ll_append(&end, d);
  }

  // Now, create an fsm and fsm_trans, and write the recorded pairs into the
  // allocated start and end buffers.
  f = fsm_create();
  src = fsm_add_state(f, false);
  dest = fsm_add_state(f, true);
  f->start = src;
  ft = fsm_trans_create(ll_length(&start), type, dest);

  for (iter = ll_get_iter(&start); ll_iter_valid(&iter); ll_iter_next(&iter)) {
    d = ll_iter_curr(&iter);
    ft->start[index] = (wchar_t) d.data_llint;
    index++;
  }
  index = 0;
  for (iter = ll_get_iter(&end); ll_iter_valid(&iter); ll_iter_next(&iter)) {
    d = ll_iter_curr(&iter);
    ft->end[index] = (wchar_t) d.data_llint;
    index++;
  }

  fsm_add_trans(f, src, ft);

  ll_destroy(&start);
  ll_destroy(&end);
  return f;
}

/**
   @brief The workhorse recursive helper to create_regex_fsm().

   Crawls through a regex, holding a FSM of the expression so far.  Single
   characters are concatenated as simple 0 -> 1 machines.  Parenthesis initiate
   a recursive call.  Pipes union the current regex with everything after.

   Needs to be called with a non-NULL value for both, which is why there is a
   top level function.

   @param regex Local pointer to the location in the regular expression.
   @param final Pointer to the parent's location.
   @return An FSM for the regular expression.
 */
fsm *create_regex_fsm_recursive(const wchar_t *regex, const wchar_t **final)
{
  // Initial FSM is a machine that accepts the empty string.
  int i = 0;
  fsm *curr = fsm_create();
  fsm *new;
  curr->start = fsm_add_state(curr, true);

  // ASSUME THAT ALL PARENS, ETC ARE BALANCED!

  for ( ; *regex; regex++) {
    switch (*regex) {

    case L'(':
      new = create_regex_fsm_recursive(regex + 1, &regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

    case L')':
      *final = regex;
      return curr;
      break;

    case L'|':
      // Need to pass control back up without attempting to check modifiers.
      new = create_regex_fsm_recursive(regex + 1, &regex);
      fsm_union(curr, new);
      fsm_delete(new, true);
      *final = regex;
      return curr;
      break;

    case L'[':
      new = create_char_class(&regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

    case L'\\':
      new = escaped_fsm(&regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;

    default:
      // A regular letter
      new = fsm_create_single_char(*regex);
      check_modifier(new, &regex);
      fsm_concat(curr, new);
      fsm_delete(new, true);
      break;
    }
  }
  *final = regex;
  return curr;
}

/**
   @brief Construct a FSM to accept the given regex.
   @param regex Regular expression string.
   @return A FSM that can be used to decide the language of the regex.
 */
fsm *create_regex_fsm(const wchar_t *regex){
  return create_regex_fsm_recursive(regex, &regex);
}

void regex_hit_init(regex_hit *obj, int start, int length)
{
  // Initialization logic
  obj->start = start;
  obj->length = length;
}

regex_hit *regex_hit_create(int start, int length)
{
  // Allocate space
  regex_hit *obj = (regex_hit *) malloc(sizeof(regex_hit));
  CLEAR_ALL_ERRORS;

  // Check for allocation error
  if (!obj) {
    RAISE(ALLOCATION_ERROR);
    return NULL;
  }

  // Initialize
  regex_hit_init(obj, start, length);

  if (CHECK(ALLOCATION_ERROR)) {
    free(obj);
    return NULL;
  }

  SMB_INCREMENT_MALLOC_COUNTER(sizeof(regex_hit));
  return obj;
}

void regex_hit_destroy(regex_hit *obj)
{
  // Cleanup logic (none)
}

void regex_hit_delete(regex_hit *obj) {
  if (obj) {
    regex_hit_destroy(obj);
    free(obj);
    SMB_DECREMENT_MALLOC_COUNTER(sizeof(regex_hit));
  } else {
    fprintf(stderr, "regex_hit_delete: called with null pointer.\n");
  }
}

smb_al *fsm_search(fsm *regex_fsm, const wchar_t *srchText, bool greedy,
                   bool overlap)
{
  fsm_sim *curr_sim;
  int start = 0, length, last_length, res;
  smb_al *results = al_create();
  DATA d;

  SMB_DP("STARTING FSM SEARCH\n");

  while (srchText[start] != L'\0') {
    // Start an FSM simulation at the current character.
    curr_sim = fsm_sim_nondet_begin(regex_fsm, srchText + start);
    length = 0;
    last_length = -1;
    res = -1;

    SMB_DP("=> Begin simulation at index %d: '%lc'.\n", start, srchText[start]);

    while (srchText[start + length] != L'\0' && res != FSM_SIM_REJECTED) {
      // Step through the FSM simulation until accepting.
      fsm_sim_nondet_step(curr_sim);
      res = fsm_sim_nondet_state(curr_sim);
      length++;
      SMB_DP("   => On step (length %d), res=%d.\n", length, res);

      // When we encounter a possible match, mark it to record.  We only have
      // one match per search, and we want the largest match per search.
      if (res == FSM_SIM_ACCEPTING || res == FSM_SIM_ACCEPTED) {
        last_length = length;
        SMB_DP("      Currently accepting!\n");
      }
      // If the search reports input exhausted, we leave the loop.
      if (res == FSM_SIM_ACCEPTED || res == FSM_SIM_REJECTED) {
        break;
      }
    }

    fsm_sim_delete(curr_sim, true);
    if (last_length != -1) {
      // If we encounter a match, record it.
      SMB_DP("=> Found match of length %d.\n", last_length);
      d.data_ptr = (void *) regex_hit_create(start, last_length);
      al_append(results, d);
      if (greedy) {
        SMB_DP("=> Greedy return.\n");
        return results;
      }
      if (overlap) {
        start++;
      } else {
        start += last_length;
      }
    } else {
      start++;
    }
  }
  return results;
}

smb_al *regex_search(const wchar_t *regex, const wchar_t *srchText, bool greedy,
                     bool overlap)
{
  fsm *regex_fsm = create_regex_fsm(regex);
  return fsm_search(regex_fsm, srchText, greedy, overlap);
}
