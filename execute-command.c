// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h> //For READ/WRITE 
#include <sys/wait.h> // For waitstatus

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        int *elements;
}Queue;
/* crateQueue function takes argument the maximum number of elements the Queue can hold, creates
   a Queue according to it and returns a pointer to the Queue. */
Queue* createQueue(int maxElements)
{
        /* Create a Queue */
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        /* Initialise its properties */
        Q->elements = (int *)malloc(sizeof(int)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;
        /* Return the pointer */
        return Q;
}
void Dequeue(Queue *Q)
{
        /* If Queue size is zero then it is empty. So we cannot pop */
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                return;
        }
        /* Removing an element is equivalent to incrementing index of front by one */
        else
        {
                Q->size--;
                Q->front++;
                /* As we fill elements in circular fashion */
                if(Q->front==Q->capacity)
                {
                        Q->front=0;
                }
        }
        return;
}
int front(Queue *Q)
{
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                exit(0);
        }
        /* Return the element which is at the front*/
        return Q->elements[Q->front];
}
void Enqueue(Queue *Q,int element)
{
        /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
        if(Q->size == Q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                Q->size++;
                Q->rear = Q->rear + 1;
                /* As we fill the queue in circular fashion */
                if(Q->rear == Q->capacity)
                {
                        Q->rear = 0;
                }
                /* Insert the element in its rear side */ 
                Q->elements[Q->rear] = element;
        }
        return;
}



char** str_split(char* str)
{
	int num = 0; //Keeps track of number of strings we've split
	char* i;
	char** return_string = (char**)checked_malloc(sizeof(char*));
	i = strtok (str," ");  //Tokenizes each string component
	while (i != NULL)
	{
		return_string = checked_realloc(return_string,sizeof(char*)*(num+2)); //Add two for the NULL char at the end
		return_string[num] = i;
		num++; //Increment number of strings we have
		return_string[num] = NULL; //Make the next character NULL
		i = strtok (NULL, " ");
	}

	return return_string;
}

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

     //WHEN TIME_TRAVEL IS FALSE:
     int fd[2]; // Used for PIPE
     int stdin_copy = dup(0);
	 int stdout_copy = dup(1);

     switch(c->type){
     	case SIMPLE_COMMAND:{
     		int status;
     		pid_t pid = fork(); //for lab 1, wants a child process to execute command
			//Since we tokenize strings differently in 1A, we need to reprocess/split the strings into different tokens
			if(pid == 1)
				exit(1);
			//Handle I/O redirect
			if(c->input){
				fd[0] = open(c->input,O_RDONLY | O_CREAT, 0644);
				if(fd[0]<0)
					exit(1);
				dup2(fd[0],0);
			}
			if(c->output){
				fd[1] = open(c->output,O_WRONLY | O_TRUNC| O_CREAT, 0644);
				if(fd[1]<0)
					exit(1);
				dup2(fd[1],1);
			}
			char** splitter = str_split(*c->u.word);
			if (pid == 0) {
				execvp(splitter[0],splitter);
			}
			else{
				waitpid(pid,&status,0);
			}
			c->status = status;
			break;
     	}
     	case AND_COMMAND:{
     		command_t left = c->u.command[0];
     		command_t right = c->u.command[1];
     		execute_command(left,time_travel);
			c->status = left->status;
			//If left side exits with 0, then we execute right side
			if(left->status == 0){
				execute_command(right,time_travel);
				c->status = right->status;
				}
			break;
     	}
	    case OR_COMMAND:{     		
	    	//Works same as AND, except we only execute the right side if left side exits false
	    	command_t left = c->u.command[0];
     		command_t right = c->u.command[1];
     		execute_command(left,time_travel);
			c->status = left->status;
			if(left->status != 0){
				execute_command(right,time_travel);
				c->status = right->status;
				}
			break;
		}
	    case PIPE_COMMAND:{
	    	command_t left = c->u.command[0];
	    	command_t right = c->u.command[1];
	    	int status;
	    	pipe(fd);
	    	pid_t pid_PIPE = fork();
	    	if(pid_PIPE == 0){
	    		close(fd[0]);
		    	dup2(fd[1],1);
		    	close(fd[1]);
		    	execute_command(left,time_travel);
		    	exit(0);
		    }
		    else{
		    	close(fd[1]);
		    	dup2(fd[0],0);
		    	close(fd[0]);
		    	execute_command(right,time_travel);
		    	waitpid(pid_PIPE,&status,0);
		    	c->status = status;
		    }
		    break;
	    }
	    case SUBSHELL_COMMAND:{
			//Handle I/O redirect
			if(c->input){
				fd[0] = open(c->input,O_RDONLY | O_CREAT, 0644);
				if(fd[0]<0)
					exit(1);
				dup2(fd[0],0);
			}
			if(c->output){
				fd[1] = open(c->output,O_WRONLY | O_TRUNC| O_CREAT, 064);
				if(fd[1]<0)
					exit(1);
				dup2(fd[1],1);
			}
	    	execute_command(c->u.subshell_command,time_travel);
	    	c->status = c->u.subshell_command->status;
	    	break;
	    }
	    case SEQUENCE_COMMAND:{
	    	command_t left = c->u.command[0];
     		command_t right = c->u.command[1];
	    	execute_command(left,time_travel);
	    	execute_command(right,time_travel);
	    	break;
	    }
     	default:
     	//Error, should never get to default case
     	//error(1,0,"Error with executing command");
     	return;
     }

     //Clear the file I/O for the next command
     dup2(stdin_copy, 0);
	 dup2(stdout_copy, 1);
	 close(stdin_copy);
	 close(stdout_copy);

}

<<<<<<< HEAD
command_t run_time_travel(command_stream_t s)
{
	int status;
	bool time_travel = false;
	command_t last_command = NULL;
	command_t command;
	while ((command = read_command_stream (s))){
		pid_t pid = fork();
		if(pid == 1)
			exit(1);
		else if(pid == 0){
			execute_command(command,time_travel);
			exit(1);
		}
		last_command = command;
	}
	pid_t finished = waitpid(-1,&status,0);

	return last_command;
}
=======
//nodes hold list of file dependencies
struct node_f {
  int _type;
  char* _string;
  struct node_f* _next;
  struct node_f* _prev;
};

typedef struct node_f token_f;

//holds all tokens from tokenizer
typedef struct {
  token_f* _token;
  token_f* _last_token;
  struct token_container* _next_tokens;
  size_t _totaltokens;
} token_container_f;

//adds token to token container
void add_token_f (token_container_f* container, token_f new_token) {
  //printf("adding %s, %i\n", new_token._string, container->_totaltokens+1);
  
  //allocate new token and copy over token content
  token_f* add_token = (token_f*)checked_malloc(sizeof(token_f));
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

void add_file_dep(token_container_f* container, char* word) {
	if (strcmp(container->_last_token->_string, word) == 0) {
		//if last added is the same, don't add again
		return;
	}
	token_f new_token;
	new_token._string = word;
	add_token_f(container, new_token);
}

struct dep_node {
	struct tt_node* _dependent;
	struct dep_note* next;
};

struct tt_node {
	command_t* _command;
	int _pid;
	token_container_f* _in;
	token_container_f* _out;
	int num_dep;
	struct dep_node* _dependents;
	struct tt_node* _next;
};

void add_cmd_dep(struct tt_node* node, command_t command) {
	//generate file dependencies from command in/out
	if (command->input != NULL) {
		if (node->_in != NULL) {
			add_file_dep(node->_in, command->input);
		} else {
			node->_in = (token_container_f*)checked_malloc(sizeof(token_container_f));
		}
	}

	if (command->output != NULL) {
		if (node->_out != NULL) {
			add_file_dep(node->_out, command->output);
		} else {
			node->_out = (token_container_f*)checked_malloc(sizeof(token_container_f));
		}
	}
}
>>>>>>> fd5c22530c1c0a1ab7d634de619ac1e2c60b7104
