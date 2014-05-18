/***************************************************************************//**

  @file         fsm.c

  @author       Stephen Brennan

  @date         Created Wednesday, 14 May 2014

  @brief        Finite state machine implementations.

  @copyright    Copyright (c) 2014, Stephen Brennan.
  All rights reserved.

  @copyright
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    - Neither the name of Stephen Brennan nor the names of his contributors may
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

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include "libstephen.h" // already in fsm.h, but used here too
#include "fsm.h"

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
 */
void fsm_trans_init(fsm_trans *ft, int n, int type, int dest)
{
  int i;
  ft->type = type;

  // Allocate space for the start range, plus null terminator
  ft->start = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->start) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation failed.\n");
    return;
  }

  // Allocate space for the end range, plus null terminator
  ft->end = (wchar_t *) malloc((n + 1) * sizeof(wchar_t));
  if (!ft->end) {
    RAISE(ALLOCATION_ERROR);
    fprintf(stderr, "libstephen: fsm_trans_init(): Memory allocation failed.\n");
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
    fprintf(stderr, "libstephen: fsm_trans_destroy() called with null pointers.\n");
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
    fprintf(stderr, "libstephen: fsm_trans_delete() called with null pointer.\n");
  }
}

/**
   @brief Initialize a fsm_trans object with a single range.

   @param ft The object to initialize
   @param start The beginning of the range
   @param end The end of the range
   @param type The type of the object
 */
void fsm_trans_init_single(fsm_trans *ft, wchar_t start, wchar_t end, int type, int dest)
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
 */
fsm_trans *fsm_trans_create_single(wchar_t start, wchar_t end, int type, int dest)
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

/**
   @brief Add a state to the FSM
   @param f The FSM to add to
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
fsm_trans *fsm_add_single(fsm *f, int from, int to, wchar_t start, wchar_t end, int type)
{
  fsm_trans *new = fsm_trans_create_single(start, end, type, to);
  fsm_add_trans(f, from, new);
  return new;
}

/**
   @brief Run the finite state machine as a deterministic one.

   Simulates a DFSM.  Any possible input at any given state should have exactly
   one transition.  Zero transitions are allowed (and interpreted as an
   immediate reject).

   @param f The FSM to simulate
   @param input The string of input
 */
bool fsm_sim_det(fsm *f, const wchar_t *input)
{
  DATA d;
  int i, next, state;
  smb_al *list;
  fsm_trans *trans;
  state = f->start;

  // While we have not exhausted our input
  while (*input != '\0') {
    SMB_DP("State: %d, Input %Lc\n", state, *input);
    // Get the list of transitions from this state
    list = (smb_al *) al_get(&f->transitions, state).data_ptr;
    next = -1;
    // Look for transitions that match this character
    for (i = 0; i < al_length(list); i++) {
      trans = (fsm_trans *) al_get(list, i).data_ptr;
      if (fsm_trans_check(trans, *input)) {
        if (next == -1) {
          next = trans->dest;
        } else {
          // If there are two matching transitions, error.
          fprintf(stderr, "Error: non-deterministic FSM simulated as deterministic.\n");
        }
      }
    }
    // If there are no transitions, fail.
    if (next == -1) {
      SMB_DP("No available transitions, reject.\n");
      return false;
    }
    state = next;
    input++;
  }

  SMB_DP("State: %d, Input finished.\n", state);

  // If the state is in the accepting states, we accept, else reject
  d.data_llint = state;
  return al_index_of(&f->accepting, d) != -1;
}

/**
   @brief Get an integer value from a key-value line.

   The line is in the form "key: value ", where whitespace is not allowed in the
   key section, but is allowed for the value section.  The value must be a
   positive decimal integer.

   @param start Pointer to the string pointer.  This function advances the
   pointer to the start of the next line.
   @param prefix The "key:" prefix the line should match.
   @returns The integer value
 */
static int fsm_read_get_int(const wchar_t **start, const wchar_t *prefix)
{
  int prefix_len = wcslen(prefix);
  int value = 0;

  if (wcsncmp(*start, prefix, prefix_len) != 0) {
    return -1;
  }
  SMB_DP("   Good prefix (\"%Ls\").\n", prefix);

  (*start) += prefix_len; // advance pointer to end of prefix
  // Advance through any whitespace
  while (**start != L'\n' && iswspace(**start)) {
    (*start)++;
  }

  if (!iswdigit(**start)) {
    fprintf(stderr, "Error: digit expected after prefix.\n");
  }

  // Note: this assumes only '0' - '9' digits.  There may be more in the entire
  // Unicode spec...
  while (iswdigit(**start)) {
    SMB_DP("   Read digit: '%Lc'\n", **start);
    value = 10 * value + **start - L'0';
    (*start)++;
  }
  SMB_DP("   Value: %d\n", value);

  // Advance to end of line
  while (**start != L'\n') {
    (*start)++;
  }

  // Leave pointer pointing at next line
  (*start)++;
  return value;
}

/**
   @brief Get the value of a hexadecimal digit.
   @param digit The digit
   @return The value of the digit in hexadecimal.
 */
static int hexit_val(wchar_t digit) {
  if (iswdigit(digit)) {
    return digit - L'0';
  } else if (digit == L'a' || digit == L'b' || digit == L'c' || digit == L'd'
             || digit == L'e' || digit == L'f' || digit == L'A' || digit == L'B'
             || digit == L'C' || digit == L'D' || digit == L'E' || digit == L'F'){
    return 10 + towupper(digit) - L'A';
  }
}

/**
   @brief Get an escaped character from the string source.

   This function will advance the source pointer to after the escape sequence.
   It can get escapes abfnrtv\0xu, which includes octal, hexadecimal, and
   unicode escapes.

   @brief source The source pointer
   @return The character that was escaped
 */
static wchar_t get_escape(const wchar_t **source) {
  wchar_t value = 0;
  wchar_t specifier = **source;
  SMB_DP("      Escape specifier '%Lc'\n", specifier);
  (*source)++;
  switch (specifier) {
  case L'a':
    return L'\a';
  case L'b':
    return L'\b';
  case L'f':
    return L'\f';
  case L'n':
    return L'\n';
  case L'r':
    return L'\r';
  case L't':
    return L'\t';
  case L'v':
    return L'\v';
  case L'\\':
    return L'\\';
  case L'0':
    value += 64 * (**source);
    (*source)++;
    value += 8 * (**source);
    (*source)++;
    value += **source;
    (*source)++;
    SMB_DP("      Escaped octal: '%Lc'\n", value);
    return value;
  case L'x':
    value += 16 * hexit_val(**source);
    (*source)++;
    value += hexit_val(**source);
    (*source)++;
    SMB_DP("      Escaped hex: '%Lc'\n", value);
    return value;
  case L'u':
    value += 16 * 16 * 16 * hexit_val(**source);
    (*source)++;
    value += 16 * 16 * hexit_val(**source);
    (*source)++;
    value += 16 * hexit_val(**source);
    (*source)++;
    value += hexit_val(**source);
    (*source)++;
    SMB_DP("      Escaped unicode: '%Lc'\n", value);
    return value;
  default:
    return specifier;
  }
}

/**
   @brief Reads a transition from a line.

   This function reads a transition from a line.  The transition can have an
   arbitrary number of ranges, each separated by a space.  The line should be of
   the form: "X-Y:[+|-]A-B[[A-B]...]", where X and Y are state numbers, A and B
   are characters forming a range, and the + or - determine whether the
   transition is positive (only those in the range) or negative (everything not
   in that range).  Note that there is only one + or -; it applies for all
   ranges in the transition.

   @param source The source pointer.  It is advanced to the beginning of the
   next line.
   @param [OUT] start Where to put the start state of the transition.
   @return The transition.
 */
static fsm_trans *fsm_read_trans(const wchar_t **source, int *start)
{
  // state: FSM state.  s1, s2: from and to states for the trans.
  // type: the type of transition.  i: iteration var
  int state = 0, s1 = 0, s2 = 0, type = -1, i;
  // first, second: array lists for first, second values in ranges
  smb_al first, second;
  // d: utility var for assigning to arraylists
  DATA d;

  al_init(&first);
  al_init(&second);

  // Input machine states
  #define SFIRSTSTATE 0
  #define SSECONDSTATE 1
  #define SFIRSTCHAR 2
  #define SSECONDCHAR 3
  #define SFINISHED 4
  #define SREADTYPE 5

  // A 'finite state machine' to read finite state machine tranistions!
  while (**source != L'\n' && **source != L'\0') {
    SMB_DP("   => Current char: '%Lc'.  Current state: ", **source);
    switch (state) {
    case SFIRSTSTATE:
      SMB_DP("SFIRSTSTATE\n");
      if (**source == L'-') {
        state = SSECONDSTATE;
        SMB_DP("      Read '-', go to SSECONDSTATE\n");
      } else if (!iswdigit(**source)) {
        fprintf(stderr, "Error: expected digit, found %Lc.\n", **source);
        return NULL;
      } else {
        s1 = s1 * 10 + **source - L'0';
        SMB_DP("      Read '%Lc', new s1 is %d.\n", **source, s1);
      }
      break;
    case SSECONDSTATE:
      SMB_DP("SSECONDSTATE\n");
      if (**source == L':') {
        SMB_DP("      Read ':', go to SREADTYPE.\n");
        state = SREADTYPE;
      } else if (!iswdigit(**source)) {
        fprintf(stderr, "Error: expected digit, found %Lc.\n", **source);
        al_destroy(&first);
        al_destroy(&second);
        return NULL;
      } else {
        s2 = s2 * 10 + **source - L'0';
        SMB_DP("      Read '%Lc', new s2 is %d.\n", **source, s2);
      }
      break;
    case SREADTYPE:
      SMB_DP("SREADTYPE\n");
      if (**source == L'-') {
        type = FSM_TRANS_NEGATIVE;
        SMB_DP("      Negative transition\n");
      } else if (**source == L'+') {
        type = FSM_TRANS_POSITIVE;
        SMB_DP("      Positive transition\n");
      } else {
        fprintf(stderr, "Error: bad transition type specifier: '%Lc'.\n", **source);
        al_destroy(&first);
        al_destroy(&second);
        return NULL;
      }
      state = SFIRSTCHAR;
      break;
    case SFIRSTCHAR:
      SMB_DP("SFIRSTCHAR\n");
      if (**source == L'\\') {
        (*source)++;
        d.data_llint = get_escape(source); //advances after the escape
        SMB_DP("      Got c1 from escape.\n");
      } else {
        d.data_llint = **source;
        SMB_DP("      Got c1: '%Lc'\n", (wchar_t) d.data_llint);
        (*source)++; // advance to hyphen, then again to next char
      }
      al_append(&first, d);
      SMB_DP("      Next char '%Lc'\n", **source);
      if (**source != L'-') {
        fprintf(stderr, "Error: bad char separator: '%Lc'.\n", **source);
        al_destroy(&first);
        al_destroy(&second);
        return NULL;
      }
      state = SSECONDCHAR;
      break;
    case SSECONDCHAR:
      SMB_DP("SSECONDCHAR\n");
      if (**source == L'\\') {
        (*source)++;
        d.data_llint = get_escape(source);
        (*source)--;
        SMB_DP("      Got c2 from escape.\n");
      } else {
        d.data_llint = **source;
        SMB_DP("      Got c2: '%Lc'\n", (wchar_t) d.data_llint);
      }
      al_append(&second, d);
      state = SFINISHED;
      break;
    case SFINISHED:
      SMB_DP("SFINISHED\n");
      if (**source == L' ') {
        SMB_DP("      Read a space.  Bracing for new range.\n");
        state = SFIRSTCHAR;
      } else {
        SMB_DP("      Finished.  Hang tight everyone.\n");
      }
      break;
    }
    (*source)++;
  }
  SMB_DP("   TRANSITION LOOP COMPLETE\n");

  // Filter out issues when the machine ends prematurely.  If we end on
  // SFIRSTCHAR, there must have been a trailing space, which is ok.
  if (state != SFINISHED && state != SFIRSTCHAR) {
    fprintf(stderr, "Error: premature line end.\n");
    al_destroy(&first);
    al_destroy(&second);
    return NULL;
  }

  // If there's a newline, advance.  If not, leave the pointer pointing at '\0'
  if (**source == '\n') {
    (*source)++;
  }

  // Create and write final output
  fsm_trans *t = fsm_trans_create(al_length(&first), type, s2);
  t->dest = s2;
  for (i = 0; i < al_length(&first); i++) {
    t->start[i] = (wchar_t) al_get(&first, i).data_llint;
    t->end[i] = (wchar_t) al_get(&second, i).data_llint;
  }
  *start = s1;

  // Clean up & return
  al_destroy(&first);
  al_destroy(&second);
  return t;
}

/**
   @brief Read a FSM from a string!  The format is as follows:

   @code
   START LINE (start: n)
   ACCEPTING LINES ... (>=1) (accept: n)
   TRANSITION LINES (X-Y:[+|-]A-B[[A-B]...])
   @endcode
   @see fsm_read_get_int() for info on START LINE and ACCEPTING LINES
   @see fsm_read_trans() for info on the TRANSITION LINES

   Note that this function produces heavy output on stdout if compiled with
   SMD_ENABLE_DIAGNOSTIC_PRINTING.

   @param The string source
   @return A FSM from the string if possible, NULL on error.
 */
fsm *fsm_read(const wchar_t *source)
{
  fsm *new = fsm_create();
  fsm_trans *ft;
  smb_al *list;
  int n, max;
  DATA d;

  SMB_DP("BEGIN READING A FSM\n");

  // Get start state
  SMB_DP("=> Read first line (start state spec).\n");
  n  = fsm_read_get_int(&source, L"start:");
  if (n == -1) {
    fprintf(stderr, "Error: FSM spec didn't contain \"start:\" at expected location.\n");
    fsm_destroy(new, true);
    return NULL;
  } else {
    SMB_DP("   *Start state is %d*\n\n", n);
    new->start = n;
  }

  // Get accepting states
  SMB_DP("=> Reading for accepting states.\n");
  while ((d.data_llint =fsm_read_get_int(&source, L"accept:")) != -1) {
    al_append(&new->accepting, d);
    SMB_DP("\n=> Reading for additional accepting states.\n");
  }
  SMB_DP("   No more accepting states.\n\n");

  // Get transitions
  SMB_DP("Reading transitions.\n");
  while (*source != L'\0') {
    SMB_DP("=> READING A TRANSITION\n");
    ft = fsm_read_trans(&source, &n);
    if (ft == NULL) {
      fsm_destroy(new, true);
      fprintf(stderr, "Error: got bad transition.\n");
      return NULL;
    }
    max = (n>ft->dest ? n : ft->dest) + 1;
    SMB_DP("   Max observed state %d, current # states %d.\n", max, al_length(&new->transitions));
    while (al_length(&new->transitions) < max) {
      fsm_add_state(new, false);
      SMB_DP("   Added a state (now have %d).\n", al_length(&new->transitions));
    }
    list = (smb_al *)al_get(&new->transitions, n).data_ptr;
    d.data_ptr = (void *) ft;
    al_append(list, d);
    SMB_DP("   *Added transition*\n\n");
  }

  SMB_DP("FSM CREATION COMPLETE!\n");
  return new;
}

/**
   @brief Print a string representation of the FSM to a file (or stdout).

   @param f The FSM to print
   @param dest The file to print to
 */
void fsm_print(fsm *f, FILE *dest)
{
  int i, j;
  smb_al *list;
  wchar_t *start, *end;
  fsm_trans *ft;

  fprintf(dest, "start:%d\n", f->start);
  for (i = 0; i < al_length(&f->accepting); i++) {
    fprintf(dest, "accepting:%Ld\n", al_get(&f->accepting, i).data_llint);
  }

  for (i = 0; i < al_length(&f->transitions); i++) {
    list = (smb_al *) al_get(&f->transitions, i).data_ptr;
    for (j = 0; j < al_length(list); j++) {
      ft = (fsm_trans *) al_get(list, j).data_ptr;
      fprintf(dest, "%d-%d:%c", i, ft->dest, (ft->type == FSM_TRANS_POSITIVE ? '+' : '-'));
      start = ft->start;
      end = ft->end;
      while (*(start+1) != L'\0') {
        fprintf(dest, "%Lc-%Lc ", *start, *end);
        start++; end++;
      }
      fprintf(dest, "%Lc-%Lc\n", *start, *end);
    }
  }
}

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

/**
   @brief Begin a non-deterministic simulation of this finite state machine.

   This simulation is designed to be run in steps.  First, call this function.
   Then, loop through many calls to fsm_sim_nondet_step(), until it returns a
   value that indicates that the simulation has finished (generally, that is
   FSM_SIM_REJECTED or FSM_SIM_ACCEPTED).

   @param f The FSM to simulate
   @param input The input to run on
 */
fsm_sim *fsm_sim_nondet_begin(fsm *f, const wchar_t *input)
{
  DATA d;
  smb_al *curr = al_create();
  d.data_llint = f->start;
  al_append(curr, d);
  fsm_sim *fs = fsm_sim_create(f, curr, input);
  return fs;
}

/**
   @brief Return the epsilon closure of the state on this FSM.

   The epsilon closure is the set of all states that can be reached from this
   state without consuming any input.

   @param f The FSM
   @param state The state to calculate from
   @return A list of states in the epsilon closure
 */
static smb_al *epsilon_closure(fsm *f, int state)
{
  smb_al *closure = al_create();
  smb_al *trans_list;
  smb_al *visit_queue = al_create();
  fsm_trans *ft;
  DATA d;
  int i;

  d.data_llint = state;
  al_push_back(visit_queue, d);

  // Breadth first search here: while queue is not empty.
  while (al_length(visit_queue) > 0) {
    // Get next state to expand, mark it as visited (in the closure)
    d = al_pop_front(visit_queue);
    al_append(closure, d);
    trans_list = (smb_al *)al_get(&f->transitions, d.data_llint).data_ptr;
    // For every transition out of it
    for (i = 0; i < al_length(trans_list); i++) {
      // If epsilon transitions out of it, and we haven't visited or put it in
      // the queue yet, add it to the visit queue.
      ft = (fsm_trans *) al_get(trans_list, i).data_ptr;
      d.data_llint = ft->dest;
      if (fsm_trans_check(ft, EPSILON) && 
          al_index_of(visit_queue, d) == -1 &&
          al_index_of(closure, d) == -1){
        d.data_llint = ft->dest;
        al_push_back(visit_queue, d);
      }
    }
  }

  al_delete(visit_queue);
  return closure;
}

/**
   @brief Combine the two lists (with no duplicates), and free the second.

   @param first The target list (which will be added to)
   @param second The list which will be combined into first and freed.
 */
static void union_and_delete(smb_al *first, smb_al *second)
{
  int i;
  DATA d;
  for (i = 0; i < al_length(second); i++) {
    d = al_get(second, i);
    if (al_index_of(first, d) == -1) {
      al_append(first, d);
    }
  }
  al_delete(second);
}

/**
   @brief Test if the intersection between the two lists is non-empty.
 */
static bool non_empty_intersection(smb_al *first, smb_al *second)
{
  int i;
  DATA d;
  for (i = 0; i < al_length(first); i++) {
    d = al_get(first, i);
    if (al_index_of(second, d) != -1)
      return true;
  }
  return false;
}

/**
   @brief Perform a single step in the non-deterministic simulation.

   This function performs a single iteration of the simulation algorithm.  It
   takes the current states, finds next states with the input character, and
   then adds in the epsilon closures of all the states.  Finally, it returns a
   value that indicates the state of the simulation.

   @param s The simulation state struct
   @retval FSM_SIM_ACCEPTING when the simulation has not ended, but is accepting
   @retval FSM_SIM_NOT_ACCEPTING when the simulation has not ended, and is not
   accepting
   @retval FSM_SIM_REJECTED when the simulation has ended and rejected
   @retval FSM_SIM_ACCEPTED when the simulation has ended and accepted
 */
int fsm_sim_nondet_step(fsm_sim *s)
{
  int i, j, state, original;
  DATA d;
  fsm_trans *t;
  smb_al *next = al_create();
  smb_al *trans;

  // For diagnostics, print out the entire current state set
  SMB_DP("Current state: ");
  for (i = 0; i < al_length(s->curr); i++) {
    SMB_DP("%Ld ", al_get(s->curr, i).data_llint);
  }
  SMB_DP("\n");

  // For each current state:
  for (i = 0; i < al_length(s->curr); i++) {
    state = (int) al_get(s->curr, i).data_llint;
    trans = (smb_al *) al_get(&s->f->transitions, state).data_ptr;

    // For each transition out:
    for (j = 0; j < al_length(trans); j++) {
      t = (fsm_trans *)al_get(trans, j).data_ptr;
      d.data_llint = t->dest;

      // If the transition contains the current input, and it's not already in
      // the next state list, add it to the next state list.
      if (fsm_trans_check(t, *s->input) &&
          al_index_of(next, d) == -1) {
        al_append(next, d);
      }
    }
  }

  // For each state in the original next state list, union in all their epsilon
  // closures.
  original = al_length(next);
  for (i = 0; i < original; i++) {
    state = (int) al_get(next, i).data_llint;
    union_and_delete(next, epsilon_closure(s->f, state));
  }

  // Delete the old state, set the new one, and advance the input
  al_delete(s->curr);
  s->curr = next;
  s->input++;

  // For diagnostics, print the new state
  SMB_DP("New state: ");
  for (i = 0; i < al_length(s->curr); i++) {
    SMB_DP("%Ld ", al_get(s->curr, i).data_llint);
  }
  SMB_DP("\n");

  // If the current state is empty, REJECT
  if (al_length(s->curr) == 0) {
    return FSM_SIM_REJECTED;
  }
  if (non_empty_intersection(&s->f->accepting, s->curr)) {
    // If one of our current states is accepting...
    if (*s->input == L'\0') {
      // ... and input is exhausted, ACCEPT
      return FSM_SIM_ACCEPTED;
    } else {
      // ... and input remains, we are accepting, but not done
      return FSM_SIM_ACCEPTING;
    }
  } else {
    // If no current state is accepting ...
    if (*s->input == L'\0') {
      // ... and the input is exhausted, REJECT
      return FSM_SIM_REJECTED;
    } else {
      // ... and input remains, we are not accepting, but not done
      return FSM_SIM_NOT_ACCEPTING;
    }
  }
}

/**
   @brief Simulate the FSM as a non-deterministic state machine.

   This function is a convenience function that executes the NDFSM to
   completion.  The other functions, begin, step, and delete, can be used to
   perform a more customizable simulation.

   @param f The FSM to simulate
   @param input The input string to run with
   @retval true if the machine accepts
   @retval false if the machine rejects
 */
bool fsm_sim_nondet(fsm *f, const wchar_t *input)
{
  int res= FSM_SIM_NOT_ACCEPTING;
  fsm_sim *sim = fsm_sim_nondet_begin(f, input);

  while (res != FSM_SIM_REJECTED && res != FSM_SIM_ACCEPTED) {
    res = fsm_sim_nondet_step(sim);
    SMB_DP("Current result: %d\n", res);
  }

  fsm_sim_delete(sim, true);
  if (res == FSM_SIM_REJECTED)
    return false;
  else
    return true;
}
