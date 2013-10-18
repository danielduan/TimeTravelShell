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

     int fd[2]; // Used for PIPE
     int stdin_copy = dup(0);
	 int stdout_copy = dup(1);

     //printf("%s\n","THE TYPE IS");
     //printf("%i\n", c->type);
     switch(c->type){
     	case SIMPLE_COMMAND:{
     		int status;
     		pid_t pid = fork(); //for lab 1, wants a child process to execute command
			//Since we tokenize strings differently in 1A, we need to reprocess/split the strings into different tokens
			if(pid == 1)
				exit(1);
			//Handle I/O redirect
			if(c->input){
				fd[0] = open(c->input,O_RDONLY | O_CREAT, 0666);
				if(fd[0]<0)
					exit(1);
				dup2(fd[0],0);
			}
			if(c->output){
				fd[1] = open(c->output,O_WRONLY | O_TRUNC| O_CREAT, 0666);
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
			break;
     	}
     	case AND_COMMAND:{
     		command_t left = c->u.command[0];
     		command_t right = c->u.command[1];
     		//pid_t pid_AND = fork();
     		execute_command(left,time_travel);
     			//if (left -> type == SIMPLE_COMMAND) {
				//	waitpid(pid_AND, &status, 0);
				//	left -> status = WEXITSTATUS(status);
				//}
			c->status = left->status;
			if(left->status == 0){
				execute_command(right,time_travel);
				///if (right -> type == SIMPLE_COMMAND) {
				//	waitpid(pid_AND, &status, 0);
				//	right -> status = WEXITSTATUS(status);
				}
			c->status = right->status;
			break;
			//}
     		/*
     		if(pid_AND == 0){
     			execute_command(left,time_travel);
     			exit(0);
     		}
     		else{
     			status = 0;
     			if(wait(&status)==NULL)
     				break;
     			else{
     			if(left->status != 0)
     				break;
     			else
     				execute_command(right,time_travel);
     			}
     		}*/
			/////

     		//execute_command(left,time_travel);
     		//if(command_status(left) == 0)
     		//	execute_command(right,time_travel);
     		//printf("%s\n","DONE WITH AND");
     	}
	    case OR_COMMAND:{     		
	    	command_t left = c->u.command[0];
     		command_t right = c->u.command[1];
     		execute_command(left,time_travel);
			c->status = left->status;
			if(left->status != 0){
				execute_command(right,time_travel);
				}
			c->status = right->status;
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
		    }
		    else{
		    	close(fd[1]);
		    	dup2(fd[0],0);
		    	close(fd[0]);
		    	execute_command(right,time_travel);
		    	waitpid(pid_PIPE,&status,0);
		    	c->status = WEXITSTATUS(status);
		    }
		    break;
	    }
	    case SUBSHELL_COMMAND:{
	    	execute_command(c->u.subshell_command,time_travel);
	    	c->status = c->u.subshell_command->status;
	    	break;
	    }
     	default:
     	printf("%s\n","NOT IMPLEMENTED");
     	return;
     }

     //Clear the file I/O for the next command
     dup2(stdin_copy, 0);
	 dup2(stdout_copy, 1);
	 close(stdin_copy);
	 close(stdout_copy);

}
