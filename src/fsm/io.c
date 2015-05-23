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
#include "libstephen/al.h"
#include "libstephen/ll.h"

/**
   @brief Return a list of lines from the given string.

   The string is modified!  You should make a copy before doing this.
   @param source The string to split into lines.
   @return A linked list containing wchar_t* to each line.
 */
static smb_ll *split_lines(wchar_t *source)
{
  wchar_t *start;
  smb_ll *list;
  DATA d;

  /*
    We go through `source` looking for every newline, replace it with NUL, and
    add the beginnig of the line to the list.
   */
  start = source;
  list = ll_create();
  while (*source != L'\0') {
    if (*source == L'\n') {
      // Add line to list.
      d.data_ptr = start;
      ll_append(list, d);
      // Null-terminate the line.
      *source = L'\0';
      // Next string starts at the next character.
      start = source + 1;
    }
    source++;
  }
  if (start != source) {
    d.data_ptr = start;
    ll_append(list, d);
  }
  return list;
}

/**
   @brief Expand f to have as many states as referenced in the transition.

   If either dst or src is greater than the number of states in f, add more
   states to f to match this transition.
 */
static void fsm_extend_states(fsm *f, int src, int dst)
{
  int max = src > dst ? src : dst;
  while (al_length(&f->transitions) < max + 1) {
    fsm_add_state(f, false);
  }
}

/**
   @brief Parse the "accept: %d" lines in a fsm spec.

   @param lines List of lines in the spec.
   @param f FSM we are currently building.
 */
static void fsm_parseacceptlines(smb_ll *lines, fsm *f)
{
  wchar_t *line;
  DATA d;
  int state;
  smb_status status = SMB_SUCCESS;

  // Iterate through lines
  d = ll_pop_front(lines, &status);
  while (status == SMB_SUCCESS) {
    // Get the next line.
    line = d.data_ptr;
    if (swscanf(line, L"accept:%d", &state) == 1) {
      // If we can successfully read an accept state, add it.
      d.data_llint = state;
      al_append(&f->accepting, d);
    } else {
      // Otherwise, we must be done with accept states, so put the line back
      // into the list and break out of the loop.
      ll_push_front(lines, d);
      break;
    }
    // Pop the next line off, if it exists.
    d = ll_pop_front(lines, &status);
  }
}

/**
   @brief Parse a transition from a single line.
   @param line The line to parse.
   @param f The FSM to put it in when we are done.
 */
static void fsm_parsetrans(wchar_t *line, fsm *f)
{
  int src_state, dst_state, type, numread, i;
  wchar_t src_char, dst_char, type_char;
  wchar_t *subline;
  smb_ll sources, destinations;
  DATA d;
  smb_iter src_it, dst_it;
  fsm_trans *t;
  smb_status status;

  // First, get the source state, destination state, and type.
  if (swscanf(line, L"%d-%d:%lc%n", &src_state, &dst_state,
              &type_char, &numread) != 3) {
    fprintf(stderr, "error: malformed transition line \"%ls\"\n", line);
    return; // TODO: smb_status!
  }
  type = type_char == L'+' ? FSM_TRANS_POSITIVE : FSM_TRANS_NEGATIVE;

  // Now, we initialize some lists to contain the range sources and
  // destinations, and begin reading these ranges.
  ll_init(&sources);
  ll_init(&destinations);
  subline = line + numread;
  while (swscanf(subline, L"%lc-%lc%n", &src_char, &dst_char, &numread) == 2) {
    d.data_llint = src_char;
    ll_append(&sources, d);
    d.data_llint = dst_char;
    ll_append(&destinations, d);
    subline += numread;
  }

  // Now that we've read them all, we can copy them into a new fsm_trans.
  src_it = ll_get_iter(&sources);
  dst_it = ll_get_iter(&destinations);
  t = fsm_trans_create(ll_length(&sources), type, dst_state);
  i = 0;
  while (src_it.has_next(&src_it)) {
    d = src_it.next(&src_it, &status);
    t->start[i] = d.data_llint;
    d = dst_it.next(&dst_it, &status);
    t->end[i] = d.data_llint;
    i++;
  }

  // Clean up our lists.
  ll_destroy(&sources);
  ll_destroy(&destinations);

  // Ensure that the FSM already has the states, and add the transition.
  fsm_extend_states(f, src_state, dst_state);
  fsm_add_trans(f, src_state, t);
}

/**
   @brief Parse the remaining lines from the line list as transitions.
   @param lines The line list.
   @param f The FSM we're building.
 */
static void fsm_parsetranslines(smb_ll *lines, fsm *f)
{
  wchar_t *line;
  DATA d;
  smb_status status = SMB_SUCCESS;

  d = ll_pop_front(lines, &status);
  while (status == SMB_SUCCESS) {
    line = d.data_ptr;
    fsm_parsetrans(line, f);
    d = ll_pop_front(lines, &status);
  }
}

/**
   @brief Return a FSM parsed from a list of lines.
   @param lines The list of lines.
 */
static fsm *fsm_parselines(smb_ll *lines)
{
  wchar_t *line;
  fsm *f;
  int state;
  smb_status status = SMB_SUCCESS;
  DATA d;

  // TODO: probably use an smb_status for the error handling.
  if (ll_length(lines) < 1) {
    fprintf(stderr, "error: need at least one line in FSM.\n");
    return NULL;
  }

  // Allocate FSM and get the first line.
  f = fsm_create();
  d = ll_pop_front(lines, &status);
  line = d.data_ptr;

  // The first line may be a "start:" line, or else we assume start:0.
  if (swscanf(line, L"start:%d", &state) == 1) {
    f->start = state;
  } else {
    // This must not have been a start state line. We'll assume the start state
    // is 0 and continue.
    f->start = 0;
    ll_push_front(lines, d);
  }

  fsm_parseacceptlines(lines, f);
  fsm_parsetranslines(lines, f);
  return f;
}

fsm *fsm_read(const wchar_t *source)
{
  smb_ll *lines;
  fsm *f;

  // Duplicate the string (wcsdup is pretty nonstandard...)
  wchar_t *copy = smb_new(wchar_t, wcslen(source) + 1);
  wcscpy(copy, source);

  // Split the string.
  lines = split_lines(copy);

  // could do some further removal of empty lines, etc. here

  // Parse the fsm and return it.
  f = fsm_parselines(lines);
  smb_free(copy);
  return f;
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
    fprintf(dest, "%lc", input);
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
  smb_status status;
  int i, j;
  smb_al *list;
  wchar_t *start, *end;
  fsm_trans *ft;

  // Print the start state
  fprintf(dest, "start:%d\n", f->start);

  // Print all the accepting states
  for (i = 0; i < al_length(&f->accepting); i++) {
    fprintf(dest, "accept:%Ld\n", al_get(&f->accepting, i, &status).data_llint);
  }

  // For each state's transition list
  for (i = 0; i < al_length(&f->transitions); i++) {

    // For each transition in that list
    list = (smb_al *) al_get(&f->transitions, i, &status).data_ptr;
    for (j = 0; j < al_length(list); j++) {

      // Print the transition and whether it's positive or negative.
      ft = (fsm_trans *) al_get(list, j, &status).data_ptr;
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
    fprintf(dest, "%lc", c);
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
  smb_status status;
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
            (int)al_get(&f->accepting, i, &status).data_llint);
  }

  // For each state's transition list
  for (i = 0; i < al_length(&f->transitions); i++) {

    // For each transition in that list
    list = (smb_al *) al_get(&f->transitions, i, &status).data_ptr;
    for (j = 0; j < al_length(list); j++) {

      // Declare an edge from source to destination.
      ft = (fsm_trans *) al_get(list, j, &status).data_ptr;
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
