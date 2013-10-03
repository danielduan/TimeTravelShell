// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NONE 0
#define STRING 1
#define SEMICOLON 2
#define PIPE 3
#define AND 4
#define OR 5
#define OPEN_P 6
#define CLOSE_P 7
#define L_REDIR 8
#define R_REDIR 9
#define NEWLINE 10
#define OTHER 11

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

/* holds commands */
struct command_stream
{
  int size;
  int iterator;
  command_t *commands;
};

typedef struct {
  int _type;
  char* _string;
} token;

typedef struct {
  token* _token;
  int _length;
} token_container;

void add_token (token_container* container, token new_token) {
  container->_token = checked_realloc(container->_token, ((container->_length+1) * sizeof(token)));
  *(container->_token + _length) = new;
  container->_length++;
}

/* separates input string into tokens and returns size */
token_container* tokenizer(char* input) {
  // intialize return token container
  token_container container = checked_malloc(sizeof(token_container));
  container->_token = NULL;
  container->_length = 0;

  // create new token
  token new_token;
  new_token._string = NULL;
  new_token._type = NONE;

  //iterator
  int i;
  for (i = 0; i < strlen(input); i++) {
    //iterate through char array and check each character
    //for possible tokens
    char current_char = input[i];
    switch(current_char) {
      case '#': {
        //these should be removed already
        break;
      }
      case ';': {
        //break;
      }
      case '\n': {
        //signifies end of command string, should end of token too
        //if type is none, append current 
        if (new_token._type != NONE) {
          token_container = add_token(token_container, new_token);
        }

        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;

        if (c == ';') {
          new_token._type = SEMICOLON;
        } else {
          new_token._type = NEWLINE;
        }

        break;
      }
      case '|': {
        break;
      }
      case '&': {
        break;
      }
      case '(': {
        break;
      }
      case ')': {
        break;
      }
      case '<': {
        break;
      }
      case '>': {
        break;
      }
      case '\t': {
        break;
      }
      case ' ': {
        break;
      }
      default: {
        break;
      }
    }
  }



}


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{


  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command reading not yet implemented");




  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
