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
  int _size; //Current number of commands being held
  int _iterator;
  command_stream* next;
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

//removes last character
void remove_last_char(char* string)
{
  int len = strlen(string);
  string[len-1] = '\0';
}

//holds individual tokens, linked list structure

struct node {
  int _type;
  char* _string;
  struct node* _next;
  struct node* _prev;
};

typedef struct node token;

//holds all tokens from tokenizer
typedef struct {
  token* _token;
  token* _last_token;
  size_t _totaltokens;
} token_container;

//adds token to token container
void add_token (token_container* container, token new_token) {
  //printf("adding %s, %i\n", new_token._string, container->_totaltokens+1);
  
  //allocate new token and copy over token content
  token* add_token = (token*)checked_malloc(sizeof(token));
  add_token->_type = new_token._type;
  add_token->_string = new_token._string;
  add_token->_next = NULL;

  //add first token and set head/tail
  if (container->_token == NULL) {
    add_token->_prev = NULL;
    container->_token = add_token;
    container->_last_token = add_token;
  } else { //add to end of linked list
    add_token->_prev = container->_last_token;
    container->_last_token->_next = add_token;
    container->_last_token = add_token;
  }
  
  container->_totaltokens++; //Counts total number of tokens added
  
}

///////////////////////////////////////////////////
//Stack, used for building our commands
typedef struct stack
{
    token* _command;
    //struct stack* _next;
    struct stack* _prev;
}* mystack;

void push(mystack* stack, token* command)
{
  mystack temp = (mystack)checked_malloc(sizeof(struct stack));
  temp->_command = command;
  temp->_prev = *stack;
  *stack = temp;
}

void pop(mystack* stack)
{
    mystack temp = *stack;
    *stack = (*stack)->_prev;
    free(temp);//}
}

token* peek(mystack* stack)
{
  return (*stack)->_command;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

/* separates input string into tokens and returns size */
token_container* tokenizer(char* input) {
  // intialize return token container
  token_container* container = checked_malloc(sizeof(token_container));
  container->_token = NULL;
  //container->_length = 0;
  container->_totaltokens = 0;

  // create new token
  token new_token;
  new_token._string = NULL;
  new_token._type = NONE;

  //iterator
  unsigned int i;
  for (i = 0; i < strlen(input) - 1; i++) {
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
          //skip the next character too
          i+=1;
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
        if (new_token._type != STRING){ //ignore if space is inside STRING
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

        int current_length = strlen(new_token._string);
        new_token._string = checked_realloc(new_token._string, current_length+1);
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

command_t stack_to_stream (mystack* operands, mystack* operators)
{
  command_stream_t command_list = checked_malloc(sizeof(struct command_stream));
  command_list->_size = 0; //Command_list is currently empty
  command_t new_command = (command_t) checked_malloc(sizeof(struct command));
  char** dblptr = (char**)checked_malloc(sizeof(char*));
  token* peeker;

  while((*operators)!=NULL)
  {
    peeker = peek(operators);
    if(!strcmp(peeker->_string,"<"))
    {
      printf("%s\n", "REACHED A < OPERATOR:");
      token* peektop = peek(operands);
      pop(operands);
      token* peeknext = peek(operands);
      new_command->type = SIMPLE_COMMAND;
      new_command->status = 0;
      new_command->input = peektop->_string;
      new_command->output = NULL;
      *dblptr = (char*)(peeknext->_string);
      printf("%s\n", *dblptr);
      new_command->u.word = dblptr;
    }
    else
      printf("%s\n", "WRONG OPERATOR");
    //(*operators) = (*operators)->_prev;
    pop(operators);
  }
    printf("%s\n","OPERATOR STACK IS EMPTY" );

  return new_command;
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
  char last_char; //Keeps track of the last char read
  char last_non_space_char;
  int count = 0; //Total number of char's added
  int count_line = 0; //Counts total number of lines or "commands" that we have
  bool line_begin = true; //Signifies the beginning of a line, used to remove whitespaces
  bool comment = false; //Comment becomes true if we're currently inside a comment
  //bool paran = false; //Checks if we're currently in a ( ) NEED TO IMPLEMENT


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
    if((last_char == ' ' && current == ' ')|| current == '\t' || (line_begin == true && current == ' '))
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
  token_container* tokens = tokenizer(buf);

  //DEBUGGING PURPOSES
  printf("%s\n","NOW DOING REAL WORK");
  printf("%i\n",tokens->_totaltokens);
  //int i;
  
  //DECLARING OPERATORS AND OPERANDS STACK
  mystack operators = NULL;
  mystack operands = NULL;

  token* token_iter = tokens->_token;
  //PUSHES EVERYTHING TO STACK
  /*while (token_iter != NULL) {
    push(&operands,token_iter);
    token_iter = token_iter->_next;
  }*/

  //CHECK STACKS
  printf("%s\n","NOW CHECKING STACKS");
  //Initializing a stack and setting it to NULL
  /*mystack temp = NULL;
  token* peeker;
  push(&temp,token_iter);
  push(&temp,token_iter->_next);
  push(&temp,token_iter->_next->_next);

  peeker = peek(&temp);
  printf("%s\n", peeker->_string);
  pop(&temp);
  peeker = peek(&temp);
  printf("%s\n", peeker->_string);
  pop(&temp);
  peeker = peek(&temp);
  printf("%s\n", peeker->_string);
  //Popping off again, but this time should give error since stack is empty
  pop(&temp); */

  //Testing stack_to_stream

  //peeker = peek(&operands);
  //printf("%s\n", peeker->_string);
  push(&operators,tokens->_token->_next);

  push(&operands,tokens->_token);
  push(&operands,tokens->_token->_next->_next);
  command_stream_t command_list = checked_malloc(sizeof(struct command_stream));
  command_list->_size = 0; //Command_list is currently empty
  command_list->commands[command_list->_size] = stack_to_stream(&operands,&operators);
  return command_list;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  return s->commands;
}
