/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Created Friday, 17 July 2015

  @brief        Test runner for CKY.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "tests.h"

int main(int argc, char **argv)
{
  fsm_test();
  fsm_io_test();
  regex_test();
  regex_search_test();
  lex_test();
}
