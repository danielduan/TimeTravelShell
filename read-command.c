// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//defines type of token
#define NONE 0
#define STRING 1
#define SEMICOLON 2
#define PIPE 3
#define AND 4
#define OR 5
#define OPEN_SUBSHELL 6
#define CLOSE_SUBSHELL 7
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

//Returns true if character is alph-numerical or one of ! % + , - . / : @ ^ _
bool validChar (char c)
{
  if(isalnum(c))
    return true;
  switch(c){
    case '!':
    case '%':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case '@':
    case '^':
    case '_':
      return true;
    default:
      return false;
  }
}

//Adds the character 
void append(char* string, char c)
{
  int len = strlen(string);
  string[len] = c;
  string[len+1] = '\0';
}

void remove_last_char(char* string)
{
  int len = strlen(string);
  string[len-1] = '\0';
}

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
  *(container->_token + container->_length) = new_token;
  container->_length++;
}

/* separates input string into tokens and returns size */
token_container* tokenizer(char* input) {
  // intialize return token container
  token_container* container = checked_malloc(sizeof(token_container));
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
      case ';': { //end of command string
        //if type is none, append current 
        if (new_token._type != NONE) {
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = SEMICOLON;
        break;
      }
      case '\n': { //should end of token
        //if type is none, append current 
        if (new_token._type != NONE) {
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = NEWLINE;
        break;
      }
      case '&': { //check for &&
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        if (input[i+1] == '&') { //if && and
          //save character
          char* new_string = checked_malloc(3 * sizeof(char));
          new_string[0] = current_char;
          new_string[1] = current_char; //obv its gonna be another '&'
          new_string[2] = '\0';
          new_token._string = new_string;
          new_token._type = AND;
        }
        break;
      }
      case '|': { //check to see if it is OR or pipe
        
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        if (input[i+1] != '|') { //if | pipe
          //save character
          char* new_string = checked_malloc(2 * sizeof(char));
          new_string[0] = current_char;
          new_string[1] = '\0';
          new_token._string = new_string;
          new_token._type = PIPE;
        } else if (input[i+1] == '|') { //if || or
          //save character
          char* new_string = checked_malloc(3 * sizeof(char));
          new_string[0] = current_char;
          new_string[1] = current_char; //obv its gonna be another '|'
          new_string[2] = '\0';
          new_token._string = new_string;
          new_token._type = OR;
          //skip the next character too
          i+=1;
        }
        break;
      }
      case '<': { //subshell input
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = L_REDIR;
        break;
      }
      case '>': { //subshell output
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = R_REDIR;
        break;
      }
      case '(': { //subshell start
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = OPEN_SUBSHELL;
        break;
      }
      case ')': { //subsheell end
        if (new_token._type != NONE) { //if not new token
          add_token(container, new_token);
        }
        //save character
        char* new_string = checked_malloc(2 * sizeof(char));
        new_string[0] = current_char;
        new_string[1] = '\0';
        new_token._string = new_string;
        new_token._type = CLOSE_SUBSHELL;
        break;
      }
      
      case ' ': { //spaces
        if (curr.type != STRING){ //ignore if space is inside STRING
          break;
        } //otherwise, let default STRING handling take care of it
      }
      default: {
        if (new_token._type != STRING) { //catch all, save token
          if (new_token._type != NONE) { //if not new token
            add_token(container, new_token);
          }
          //save character
          char* new_string = checked_malloc(sizeof(char));
          new_string[0] = '\0';
          new_token._string = new_string;
          new_token._type = STRING;
        }
        //add character to the end of string
        int current_length = strlen(new_token._string)
        new_token._string = checked_realloc(string, current_length+1);
        new_token._string[current_length] = current_char;
        new_token._string[current_length+1] = '\0';
        break;
      }
    }
  }

  //commit last token at the end
  if (new_token._type != NONE) { //if not new token
    add_token(container, new_token);
  }
  //return container after everything is processed
  return container;
}


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{


  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */


  char current;
  char buf[1024] = "";
  char last_char = ""; //Keeps track of the last char read
  char last_non_space_char = "";
  int count = 0; //Total number of char's added
  int count_line = 0; //Counts total number of lines or "commands" that we have
  bool line_begin = true; //Signifies the beginning of a line, used to remove whitespaces
  bool comment = false; //Comment becomes true if we're currently inside a comment
  bool paran = false; //Checks if we're currently in a ( )


  //Loops through the entire input
  while((current = get_next_byte(get_next_byte_argument)) != EOF)
  {

    //Next two if statements will get rid of any spaces that occur between a '<' or '>'
    if((current == '>' || current == '<') && last_char == ' ')
      remove_last_char(buf);
    if(current == ' ' && (last_char == '>' || last_char == '<'))
      continue;

    //If there is a comment right after a token, there is an error
    if(current == '#' && (last_non_space_char == '|' || last_non_space_char == '&' || last_non_space_char == '<' || last_non_space_char == '>'))
    {
      error(1,0,"Invalid syntax: Comment right after a token");
      exit(1);
    }

    //Get rid of spaces
    if(last_char == ' ' && current == ' '|| current == '\t' || line_begin == true && current == ' ')
    {
      continue;
    }

    //If we run into a comment (#), then we want to set the comment flag until we reach a newline
    if(current == '#')
      comment = true;
    if(current == '\n' && comment == true)
      comment = false;

    //If we run into a newline, then we need to check if the previous character is a special character
    //If it is, then we need to continue adding to the buffer
    if(current == '\n' && (last_non_space_char == '|' || last_non_space_char == '&' || last_non_space_char == '<' || last_non_space_char == '>'))
    { 
      count_line++;
      line_begin = true;
      continue;
    }

    //Gets rid of uneeded newlines
    if(current == '\n' && last_char == '\n')
    {
      line_begin = true;
      continue;
    }

    //We add the current character to our buffer as long as we're not in a comment
    else if(comment == false)
      {
        line_begin = false;
        append(buf,current); //Add character to our buffer
        last_char = current;
        if(current != ' ')
          last_non_space_char = current;
        count++;
      }
  }

  //DEBUGGING PURPOSES
  int i;
  for(i = 0; i < sizeof(buf); i++)
    {
    printf("%c",buf[i]);
    }

  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  return 0;
}
