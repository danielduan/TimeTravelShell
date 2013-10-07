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
  struct command_stream* _next;
  command_t *commands;
};

//Returns true if character is alph-numerical or one of ! % + , - . / : @ ^ _
bool validChar (char c)
{
  if(isalnum(c) || isspace(c))
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
    case ';':
    case '|':
    case '&':
    case '<':
    case '>':
    case '(':
    case ')':
    case '\n':
    case '\t':
    case '#':
    case '\r':
      return true;
      break;
    default:
      return false;
      break;
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
  struct token_container* _next_tokens;
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
    command_t _command;
    //struct stack* _next;
    struct stack* _prev;
}* mystack;

void push(mystack* stack, command_t command)
{
  mystack temp = (mystack)checked_malloc(sizeof(struct stack));
  temp->_command = command;
  temp->_prev = *stack;
  *stack = temp;
}

void pop(mystack* stack)
{
  if(stack != NULL && (*stack) != NULL)
  {
    //mystack temp = *stack;
    *stack = (*stack)->_prev;
    //free(temp);
  }
}

command_t peek(mystack* stack)
{
  if(stack == NULL || *stack == NULL)
    return NULL;
  return (*stack)->_command;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

/* separates input string into tokens and returns size */
token_container* tokenizer(char* input) {
  // intialize return token container
  token_container* container = checked_malloc(sizeof(token_container));
  container->_token = NULL;
  container->_next_tokens = NULL;
  container->_totaltokens = 0;

  //Initialize a token head, which we will return
  token_container* head = checked_malloc(sizeof(token_container));
  head = container;

  // create new token
  token new_token;
  new_token._string = NULL;
  new_token._type = NONE;

  //iterator
  unsigned int i;
  for (i = 0; i < strlen(input); i++) {
    //iterate through char array and check each character
    //for possible tokens
    char current_char = input[i];
    switch(current_char) {
      case '#': {
        //these should be removed already
        break;
      }
      /*case ';': { //end of command string
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
      }*/
      case '\n': 
      case ';': { //should end of token
        //if type is none, append current 
        if (new_token._type != NONE) {
          add_token(container, new_token);
        }
        //save character
        container->_next_tokens = checked_malloc(sizeof(token_container));
        container = (token_container*)(container->_next_tokens);
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
      case ')': { //subshell end
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
  return head;
}

//use this function to convert list of tokens into command_t
command_t make_command(token_container* list) {
  //catch null list
  if (list == NULL) {
    return NULL;
  }

  //Use postfix and stacks to process the commands
  token* token_iter = list->_token;

  //two stacks, one to hold commands, one to hold operands
  mystack commands = NULL;
  mystack operators = NULL;

  //loop through list of tokens to process
  while (token_iter != NULL) {
    //printf("pushing: %s\n",token_iter->_string);

    switch (token_iter->_type) {
      case NONE: {
        //printf("NONE: %s\n",token_iter->_string);
        break;
      }
      case STRING: {
        //printf("STRING: %s\n",token_iter->_string);
        //check of redirect operators are present in operator stack
        if (peek(&operators) != NULL && peek(&operators)->type == SIMPLE_COMMAND) {
          //printf("STRING OP: %s\n",token_iter->_string);
          //create command_t from current stuff on stack
          command_t words = peek(&operators);
          pop(&operators);

          //check if input or output and set params
          if (words->input != NULL) {
            words->input = token_iter->_string;
          } 

          if (words->output != NULL) {
            words->output = token_iter->_string;
          }

          //printf("STRING WORD: %s\n",token_iter->_string);
          words->type = SIMPLE_COMMAND;

          //words->u.word = peek(&commands)->u.word;
          command_t prev = peek(&commands);

          if (prev->input != NULL) {
            words->u.word = prev->u.word;
            printf("STRING WORD 1: %s\n",token_iter->_string);
            size_t origlen = strlen(prev->input);
            char* catword = *(words->u.word);
            printf("STRING WORD 2: %s\n",token_iter->_string);
            catword = checked_realloc(catword, strlen(catword)+2+strlen(prev->input));
            size_t x;

            catword[origlen] = '<';
            printf("STRING WORD 3: %s\n",token_iter->_string);
            for (x = 0; x < strlen(prev->input); x++) {
              printf("STRING WORD copy: %c\n",prev->input[x]);
              catword[origlen + 1 + x] = prev->input[x];
            }
            printf("STRING WORD 4: %s\n",catword);
          } else if (prev->output != NULL){
            words->u.word = prev->u.word;
            size_t origlen = strlen(prev->output);
            char* catword = *(words->u.word);
            catword = checked_realloc(catword, strlen(catword)+1+strlen(prev->output));
            size_t x;
            catword[origlen] = '>';
            for (x = 0; x < strlen(prev->output) - 1; x++) {
              catword[origlen + 1 + x] = prev->output[x];
              printf("STRING WORD copy2: %c\n",prev->output[x]);
            }
            printf("STRING WORD 42: %s\n",catword);
          }
          else {
            words->u.word = prev->u.word;
          }
          pop(&commands);
          
          //printf("STRING PUSH: %s\n",token_iter->_string);
          push(&commands, words);
        } // check if && or || or | is present
        else if (peek(&operators) != NULL && (peek(&operators)->type == AND_COMMAND || peek(&operators)->type == OR_COMMAND || peek(&operators)->type == PIPE_COMMAND)) {
          command_t words = peek(&operators);
          words->u.word = NULL;
          pop(&operators);

          words->input = NULL;
          words->output = NULL;

          //printf("STRING OP WORD: %s\n",token_iter->_string);

          words->u.command[0] = peek(&commands);
          pop(&commands);

          //create operators
          command_t nwords = (command_t) checked_malloc(sizeof(struct command));
          nwords->type = SIMPLE_COMMAND;
          char** dblptr = (char**)checked_malloc(sizeof(char*));
          *dblptr = (char*)(token_iter->_string);
          nwords->u.word = dblptr;
          //needs rework
          nwords->input = NULL;
          nwords->output = NULL;

          words->u.command[1] = nwords;
          
          //printf("STRING OP PUSH: %s\n",token_iter->_string);
          push(&commands, words);
        
        }
         else {
          //printf("STRING ELSE: %s\n",token_iter->_string);

          //create command_t
          command_t words = (command_t) checked_malloc(sizeof(struct command));
          words->type = SIMPLE_COMMAND;
          char** dblptr = (char**)checked_malloc(sizeof(char*));
          *dblptr = (char*)(token_iter->_string);
          words->u.word = dblptr;
          //needs rework
          words->input = NULL;
          words->output = NULL;

          push(&commands, words);
        }

        break;
      }
      /*
      case SEMICOLON:
      case PIPE:
      case AND:
      case OR: {

        printf("SEMI: %s\n",token_iter->_string);
        if (commands!=NULL && (peek(&operators)->type == PIPE_COMMAND || ((peek(&operators)->type == AND_COMMAND || peek(&operators)->type == OR_COMMAND) && token_iter->_type != PIPE))) {
          //create command_t from current stuff on stack
          command_t words = peek(&operators);
          pop(&operators);
          words->u.command[1] = peek(&commands);
          pop(&commands);
          words->u.command[0] = peek(&commands);
          pop(&commands);
          if (words->u.command[0] == NULL || words->u.command[1] == NULL) {
            printf("%s\n", "NULL operator");
            return NULL;
          }

          push(&commands, words);

          command_t op = (command_t)checked_malloc(sizeof(struct command));
          if (token_iter->_type == AND) {
            op->type = AND_COMMAND;
          } else if (token_iter->_type == OR) {
            op->type = OR_COMMAND;
          } else if (token_iter->_type == PIPE) {
            op->type = PIPE_COMMAND;
          } else if (token_iter->_type == SEMICOLON) {
            op->type = SEQUENCE_COMMAND;
          }
          printf("%s\n", "SEMI Pushing operator");
          push(&operators, op);
        
        }
        break;
      }
      */
      case OPEN_SUBSHELL: {

        //printf("O SUB: %s\n",token_iter->_string);

        //create new list of inner token
        token_container* paran_list = checked_malloc(sizeof(token_container));
        paran_list->_token = NULL;
        paran_list->_totaltokens = 0;
        token* last = NULL;

        //error checking needs to be implemented
        token_iter = token_iter->_next;
        last = token_iter;
        while(token_iter != NULL)
        {
          if (token_iter->_type == CLOSE_SUBSHELL) {
            break;
          } else {
            add_token(paran_list, *token_iter);
          }
          token_iter = token_iter->_next;
          last = token_iter;
        }

        //recursively call this function again
        command_t sub = make_command(paran_list);
        //printf("O SUB MAKE: %s\n",token_iter->_string);

        if (sub != NULL) {
          command_t words = (command_t)checked_malloc(sizeof(struct command));
          words->type = SUBSHELL_COMMAND;
          //printf("O SUB PUSH: %s\n",token_iter->_string);
          words->u.subshell_command = sub;
          push(&commands, words);
        }

        //move iterator to next token
        //printf("O SUB NEXT: %s\n",token_iter->_string);
        token_iter = last;
        //printf("O SUB BREAK: %s\n",token_iter->_string);
        break;
      }
      case CLOSE_SUBSHELL: {

        //printf("CSUB: %s\n",token_iter->_string);
        //error because this should never happen
        //printf("%s\n", "Extra close paranthesis");
        break;
      }
      case L_REDIR: {

        //printf("LRE: %s\n",token_iter->_string);
        command_t words = (command_t)checked_malloc(sizeof(struct command));
        words->type = SIMPLE_COMMAND;
        //needs rework
        words->input = checked_malloc(sizeof(char));
        words->input[0] = '\0';
        push(&operators, words);
        break;
      }
      case R_REDIR: {

        //printf("RRE: %s\n",token_iter->_string);
        command_t words = (command_t)checked_malloc(sizeof(struct command));
        words->type = SIMPLE_COMMAND;
        //needs rework
        words->output = checked_malloc(sizeof(char));
        words->output[0] = '\0';
        push(&operators, words);
        break;
      }
      case NEWLINE: {
        //printf("%s\n", "New line, should be end of command");
        break;
      }
      default: {

        //printf("DEF: %s\n",token_iter->_string);
        command_t op = (command_t) checked_malloc(sizeof(struct command));
        if (token_iter->_type == AND) {
          op->type = AND_COMMAND;
        } else if (token_iter->_type == OR) {
          op->type = OR_COMMAND;
        } else if (token_iter->_type == PIPE) {
          op->type = PIPE_COMMAND;
        } else if (token_iter->_type == SEMICOLON) {
          op->type = SEQUENCE_COMMAND;
        }
        push(&operators, op);
        break;
      }
    }

    //go to next token
    token_iter = token_iter->_next;
  }

  //iterate through list of operators and pop them
  while(peek(&operators) != NULL) {
    command_t words = peek(&operators);
    pop(&operators);
    words->u.command[1] = peek(&commands);
    pop(&commands);
    words->u.command[0] = peek(&commands);
    pop(&commands);
    if (words->u.command[0] == NULL || words->u.command[1] == NULL) {
      //printf("%s\n", "operator stack is NULL");
      return NULL;
    }
    push(&commands, words);
  }

  command_t output = (command_t)checked_malloc(sizeof(struct command));
  output = peek(&commands);
  //printf("THE PUSHED COMMAND INPUT: %s\n", output->input);
  pop(&commands);
  //if for whatever reason its empty, catch it.
  if (output != NULL) {
    return output;
  } else {
    //something wrong happened
    //printf("%s\n", "command stack empty");
    return NULL;
  }


}

/*command_t stack_to_stream (mystack* operands, mystack* operators)
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
      //Peektop is the top of the operands
      token* peektop = peek(operands);
      pop(operands);
      //Peek next holds the next operand
      token* peeknext = peek(operands);

      //Initialize our type and format the '<' command
      new_command->type = SIMPLE_COMMAND;
      new_command->status = 0;
      new_command->input = peektop->_string;
      new_command->output = NULL;
      *dblptr = (char*)(peeknext->_string);
      new_command->u.word = dblptr;
    }
    else
      printf("%s\n", "WRONG OPERATOR");
    //Pop off operator
    pop(operators);
  }
  return new_command;
}*/

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{


  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */


  char current;
  char buf[1024] = "";
  char last_char = '\0'; //Keeps track of the last char read
  char last_non_space_char = '\0';
  int count = 0; //Total number of char's added
  int count_line = 0; //Counts total number of lines or "commands" that we have
  bool line_begin = true; //Signifies the beginning of a line, used to remove whitespaces
  bool comment = false; //Comment becomes true if we're currently inside a comment
  bool dble_AND = false; //Is true when we see a &&
  bool dble_OR = false; //Is true when we see a ||
  int paran_count = 0; //Number of parans we're in, and checks if we're currently in a paran


  //Loops through the entire input
  while((current = get_next_byte(get_next_byte_argument)) != EOF)
  {
    if(!validChar(current))
    {
      error(1,0,"Not a valid character");
    }
    if(current == '(')
      paran_count++;
    if(current == ')')
      paran_count--;

    if((current == ' ' || current == '\n' || current == '\r') && (last_non_space_char == '\0'))
        continue;
    //if((current == '|' || current == '&' || current == '<' || current == '>' || current == ';') && last_char == '\0')
    //    error(1,0,"Bad characters");
    if(current == ';' && (last_non_space_char == '\n' || last_char == '\0'))
      error(1,0,"Invalid ; error");
    //Handles redirects
    if((current == '<' && last_non_space_char == '<') ||(current == '>' && last_non_space_char == '>'))
        error(1,0,"Can't redirect twice");
    //Handles semicolons the same way we handle newlines
    if(current == ';')
      current = '\n';

    //Checks if we have ||| or &&& in a row
    if(dble_AND == true && current == '&')
      error(1,0,"Can't have &&&");
    if(dble_OR == true && current == '|')
      error(1,0,"Can't have |||");

    //Sets dble to be true if we see a && or ||
    if(current == '&' && last_char == '&' && dble_AND == false)
      dble_AND = true;
    else
      dble_AND = false;

    if(current == '|' && last_char == '|' && dble_OR == false)
      dble_OR = true;
    else
      dble_OR = false;

    //Next two if statements will get rid of any spaces that occur between a '<' or '>'
    if((current == '>' || current == '<' || current == '|' || current == '&') && last_char == ' ')
      remove_last_char(buf);

    if(current == ' ' && (last_char == '>' || last_char == '<'|| last_char == '|'|| last_char == '&'))
      continue;

    //If there is a comment right after a token, there is an error
    if(current == '#' && (last_non_space_char == '|' || last_non_space_char == '&' || last_non_space_char == '<' || last_non_space_char == '>'))
    {
      error(1,0,"Invalid syntax: Comment right after a token");
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
      {comment = false;
        continue;}

    //If we run into a newline, then we need to check if the previous character is a special character
    //If it is, then we need to continue adding to the buffer
    if(current == '\n' && (last_non_space_char == '|' || last_non_space_char == '&' ))
    { 
      count_line++;
      line_begin = true;
      continue;
    }
    if(current == '\n' && ( last_non_space_char == '<' || last_non_space_char == '>'))
      error(1,0,"Redirection syntax error");

    //Gets rid of uneeded newlines
    if(current == '\n' && last_char == '\n')
    {
      line_begin = true;
      continue;
    }

    //We add the current character to our buffer as long as we're not in a comment
    if(comment == false)
      {
        if(current != '\n')
          line_begin = false;
        else
          line_begin = true;
        append(buf,current); //Add character to our buffer
        last_char = current;
        if(current != ' ')
          last_non_space_char = current;
        count++;
      }
  }

  if(last_char == '>' || last_char == '|' || last_char == '&' || last_char == '<' || last_char == '(')
    error(1,0,"Invalid syntax");
  if(last_char == ')' && paran_count != 0)
    error(1,0,"Mismatched semicolons");
  if(last_char == '\n')
    remove_last_char(buf);
  if(paran_count != 0)
      error(1,0,"Mismatched semicolons");

  //tokenize the input
  token_container* tokens = tokenizer(buf);

  //DEBUGGING PURPOSES
  //printf("%s\n","NOW DOING REAL WORK");
  //printf("%i\n",tokens->_totaltokens);
  //int i;
  
  /*while (token_iter != NULL) {
    push(&operands,token_iter);
    token_iter = token_iter->_next;
  }*/
    /*
  //Creating test stacks to work with. 
  printf("%s\n","NOW CHECKING STACKS");
  //Operands stack holds [cat|a], operator stack hold [<]
  push(&operators,tokens->_token->_next);
  push(&operands,tokens->_token);
  push(&operands,tokens->_token->_next->_next);

  //Create and initialize a command_stream for us to use
  command_stream_t command_list = checked_malloc(sizeof(struct command_stream));
  command_list->_size = 0;
  command_list->_next = NULL;
  command_list->commands = checked_malloc(16*sizeof(struct command));

  //Adds a new command to command stream, command should be 'cat<a' and is accessed with commands[0]
  command_list->commands[command_list->_size] = stack_to_stream(&operands,&operators);
  command_list->_size++;

  //Operands stack holds [cat|a], operator stack hold [<]
  push(&operators,tokens->_token->_next);
  push(&operands,tokens->_token->_next->_next);
  push(&operands,tokens->_token);

  //Adds a new command to command stream, command should be 'a<cat' and is accessed with commands[1]
  command_list->commands[command_list->_size] = stack_to_stream(&operands,&operators);*/

  command_stream_t command_list = checked_malloc(sizeof(struct command_stream));
  command_list->_size = -1;
  command_list->_iterator = 0;
  command_list->_next = NULL;
  command_list->commands = checked_malloc(sizeof(struct command));

  while(tokens != NULL){
    //printf("%s\n","NOT NULL");
    command_list->_size++;
    command_list->commands[command_list->_size] = make_command(tokens);
    tokens = (token_container*)tokens->_next_tokens;
  }
  //printf("THE INPUT IS: %s\n", command_list->commands[command_list->_size]->input);
  //printf("THE OUTPUT IS: %s\n", command_list->commands[command_list->_size]->output);
  return command_list;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
    if(s->_iterator > s->_size)
    return NULL;
  return s->commands[(s->_iterator++)];
}
