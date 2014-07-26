/***************************************************************************//**

  @file         fsm/io.c

  @author       Stephen Brennan

  @date         Created Saturday, 26 July 2014

  @brief        IO Functions for FSMs.

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

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>

#include "fsm.h"
#include "str.h"
#include "libstephen.h"

/**
   @brief Reading the 'source' state of the transition.
   @see fsm_read_trans()
 */
#define SFIRSTSTATE 0
/**
   @brief Reading the 'destination' state of the transition.
   @see fsm_read_trans()
 */
#define SSECONDSTATE 1
/**
   @brief Reading the first character in a range.
   @see fsm_read_trans()
 */
#define SFIRSTCHAR 2
/**
   @brief Reading the second character in a range.
   @see fsm_read_trans()
 */
#define SSECONDCHAR 3
/**
   @brief Finished reading the transition.
   @see fsm_read_trans()
 */
#define SFINISHED 4
/**
   @brief Reading the type of the transition (positive or negative).
   @see fsm_read_trans()
*/
#define SREADTYPE 5

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
int fsm_read_get_int(const wchar_t **start, const wchar_t *prefix)
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
   @brief Reads a transition from a line.

   This function reads a transition from a line.  The transition can have an
   arbitrary number of ranges, each separated by a space.  The line should be of
   the form: `X-Y:[+|-]A-B[[A-B]...]`, where `X` and `Y` are state numbers, `A`
   and `B` are characters forming a range, and the `+` or `-` determine whether
   the transition is positive (only those in the range) or negative (everything
   not in that range).  Note that there is only one `+` or `-`; it applies for
   all ranges in the transition.

   @param source The source pointer.  It is advanced to the beginning of the
   next line.
   @param[out] start Where to put the start state of the transition.
   @return The transition.
 */
fsm_trans *fsm_read_trans(const wchar_t **source, int *start)
{
  // state: FSM state.  s1, s2: from and to states for the trans.
  // type: the type of transition.  i: iteration var
  int state = SFIRSTSTATE, s1 = 0, s2 = 0, type = -1, i;
  // first, second: array lists for first, second values in ranges
  smb_al first, second;
  // d: utility var for assigning to arraylists
  DATA d;

  al_init(&first);
  al_init(&second);

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
        fprintf(stderr, "Error: bad transition type specifier: '%Lc'.\n",
                **source);
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
        d.data_llint = get_escape(source, EPSILON); //advances after the escape
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
        d.data_llint = get_escape(source, EPSILON);
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

   @param source The string source
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
    fprintf(stderr, "Error: FSM spec didn't contain \"start:\" at expected "
            "location.\n");
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
    SMB_DP("   Max observed state %d, current # states %d.\n", max,
           al_length(&new->transitions));
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
   @brief Print a character, filtering if necessary.

   This subroutine of fsm_print() prints characters in transitions.  It exists
   to make non-printable characters printable by printing their escaped versions
   instead.

   @param dest The file to print to.
   @param input The character to filter.
 */
void fsm_print_char(FILE *dest, wchar_t input)
{
  switch (input) {

  case EPSILON:
    // Print epsilon as \e
    fprintf(dest, "\\e");
    break;

  default:
    // Print other characters as verbatim
    fprintf(dest, "%Lc", input);
    break;
  }
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

  // Print the start state
  fprintf(dest, "start:%d\n", f->start);

  // Print all the accepting states
  for (i = 0; i < al_length(&f->accepting); i++) {
    fprintf(dest, "accept:%Ld\n", al_get(&f->accepting, i).data_llint);
  }

  // For each state's transition list
  for (i = 0; i < al_length(&f->transitions); i++) {

    // For each transition in that list
    list = (smb_al *) al_get(&f->transitions, i).data_ptr;
    for (j = 0; j < al_length(list); j++) {

      // Print the transition and whether it's positive or negative.
      ft = (fsm_trans *) al_get(list, j).data_ptr;
      fprintf(dest, "%d-%d:%c", i, ft->dest,
              (ft->type == FSM_TRANS_POSITIVE ? '+' : '-'));

      // For every range in the transition, print it followed by a space.
      for (start = ft->start, end = ft->end; *start != L'\0'; start++, end++) {
        fsm_print_char(dest, *start);
        fprintf(dest, "-");
        fsm_print_char(dest, *end);

        // If this isn't the last range, print a space to separate
        if (*(start+1) != L'\0') {
          fprintf(stdout, " ");
        }
      }
      fprintf(dest, "\n");
    }
  }
}

/**
   @brief Prints a character in an FSM specification to dot format.

   This function allows for printing out some characters differently than
   others.  In dot format, quotes should be escaped.  Also, epsilon transitions
   should be labelled something readable.

   @brief dest Destination file.
   @brief c The character to print
 */
void fsm_dot_char(FILE * dest, wchar_t c)
{
  switch (c) {

  case EPSILON:
    // Print epsilon as a string, not a non-existant character!
    fprintf(dest, "eps");
    break;
  case L'\"':
    // Escape quotes.
    fprintf(dest, "\"");
    break;
  default:
    // Print other characters vebatim.
    fprintf(dest, "%Lc", c);
    break;
  }
}

/**
   @brief Print an FSM to graphviz dot format.

   Graphviz is a neat CLI tool to create diagrams of graphs.  The dot format is
   an input language for graphs.  This function takes a FSM and prints it to a
   file as a dot file, so that it can then be converted to a image by graphviz.

   @param f The machine to print.
   @param dest The destination file.
 */
void fsm_dot(fsm *f, FILE *dest)
{
  int i, j;
  smb_al *list;
  wchar_t *start, *end;
  fsm_trans *ft;

  // Declare a digraph, where the nodes are all boxes.
  fprintf(dest, "digraph regex {\n");
  fprintf(dest, "  node [shape=box];\n");

  // Declare start state as oval
  fprintf(dest, "  s%d [shape=oval];\n", f->start);

  // Declare accepting states as octagons
  for (i = 0; i < al_length(&f->accepting); i++) {
    fprintf(dest, "  s%d [shape=octagon];\n",
            al_get(&f->accepting, i).data_llint);
  }

  // For each state's transition list
  for (i = 0; i < al_length(&f->transitions); i++) {

    // For each transition in that list
    list = (smb_al *) al_get(&f->transitions, i).data_ptr;
    for (j = 0; j < al_length(list); j++) {

      // Declare an edge from source to destination.
      ft = (fsm_trans *) al_get(list, j).data_ptr;
      fprintf(dest, "  s%d -> s%d ", i, ft->dest);

      // Begin writing the label for the transition
      fprintf(dest, "[label=\"(%c) ",
              (ft->type == FSM_TRANS_POSITIVE ? '+' : '-'));

      // For every character range in the transition description.
      for (start = ft->start, end = ft->end; *start != L'\0'; start++, end++) {

        // Print the range into the label using the filter.
        fsm_dot_char(dest, *start);
        fprintf(dest, "-");
        fsm_dot_char(dest, *end);

        // If this isn't the last range, print a space to separate.
        if (*(start+1) != L'\0') {
          fprintf(stdout, " ");
        }
      }

      // Finish up the label and transition declaration.
      fprintf(dest, "\"];\n");
    }
  }

  // Finish up the entire digraph declaration.
  fprintf(dest, "}\n");
}
