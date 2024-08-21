#include "tokens.c"
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#define MAXLINELEN 1024

int off = 0;

typedef struct {
	char** inputLine;
	char* output;
	char* input;
	int append;
} cmd;

typedef struct {
	int background;
	cmd* commands;
} cmdlist;

int isOperand(char* arg) {
	return ((strcmp(arg, "|") == 0) || (strcmp(arg, ">") == 0) || (strcmp(arg, ">>") == 0) || (strcmp(arg, "<") == 0));
}

int getCommands(char** tokens) {
	int p = 0;
	for (int i = 0; tokens[i] != NULL; i++) {
		if (strcmp(tokens[i], "|") == 0) p++;
	}
	return p + 1; //numCommands
}

cmdlist* getLine(char** tokens) {
	cmdlist* cmds = (cmdlist*) calloc(1, sizeof(cmdlist));
	cmds->background = 0;
	int loop, p = 0;
	cmds->commands = (cmd*)calloc(getCommands(tokens), sizeof(cmd) * getCommands(tokens));
	for (int i = 0; i < getCommands(tokens); i++) {
		cmds->commands[i].inputLine = (char**)calloc(MAXLINELEN, sizeof(char*));
	}
	while(tokens[off] != NULL) {
		if (strcmp(tokens[off], "&") == 0) {
			if (tokens[off + 1] == NULL) {
				cmds->background = 1;
				return cmds;
			}
			else {
				printf("Error: \"&\" must be last token on command line\n");
				return NULL;
			}
		}
		else if (strcmp(tokens[off], ">") == 0) {
			if (off == 0) {
				printf("Error: Invalid null command.\n");
				return NULL;
			}
			else if (tokens[off + 1] == NULL) {
				printf("Error: Missing filename for output redirection.\n");
				return NULL;
			}
			else if (cmds->commands[loop].output != NULL) {
				printf("Error: Ambiguous output redirection.\n");
				return NULL;
			}
			else { //signify redirection
				cmds->commands[loop].output = tokens[off + 1];
				off++; //so it doesnt process argument after >
			}
		}
		else if (strcmp(tokens[off], ">>") == 0) {
			if (off == 0) {
				printf("Error: Invalid null command.\n");
				return NULL;
			}
			else if (tokens[off + 1] == NULL) {
				printf("Error: Missing filename for output redirection.\n");
				return NULL;
			}
			else if (cmds->commands[loop].output != NULL) {
				printf("Error: Ambiguous output redirection.\n");
				return NULL;
			}
			else { //do redirect from tokens[i-1] to tokens[i+1], and get rid of tokens[i]?, put in cmds
				cmds->commands[loop].output = tokens[off + 1];
				cmds->commands[loop].append = 1;
			}
		}
		else if (strcmp(tokens[off], "<") == 0) {
			if (off == 0) {
				printf("Error: Invalid null command.\n");
				return NULL;
			}
			else if (tokens[off + 1] == NULL) {
				printf("Error: Missing filename for input redirection.\n");
				return NULL;
			}
			else if (cmds->commands[loop].input != NULL) {
				printf("Error: Ambiguous input redirection.\n");
				return NULL;
			}
			else { 
				cmds->commands[loop].input = tokens[off + 1];
			}
		}
		else if (strcmp(tokens[off], "|") == 0) {
			if (off == 0) {
				printf("Error: Invalid null command.\n");
				return NULL;
			}
			else if (tokens[off + 1] == NULL) {
				printf("Error: Missing command after pipe.\n");
				return NULL;
			}
			else if (cmds->commands[loop].output != NULL) {
				printf("Error: Ambiguous output redirection.\n");
				return NULL;
			}
			else {
				cmds->commands[loop].output = "pipe";
				cmds->commands[loop].inputLine[p] = NULL;
				loop++;
				p = 0;
				cmds->commands[loop].input = "pipe";
			}
		}
		else {
			cmds->commands[loop].inputLine[p] = tokens[off];
			p++;
		}
		off++;
	}
	return cmds;
}

int main(int argc, char* argv[]) {

	int pid, wpid, status;
	
	while (1) {
		
		printf("mysh: ");
		char line[MAXLINELEN];
		char** tokens;
		cmdlist* cmds;
		
		if (fgets(line, MAXLINELEN, stdin) != NULL) { //CHANGE EVERYTHING HERE TO CMDS ONCE FINISHED
			off = 0;
			
			tokens = get_tokens(line);
			cmds = getLine(tokens);
			if (cmds == NULL) continue; //maybe have to free or som
			if (tokens[0] == NULL) continue;
			int numCommands = getCommands(tokens);
			int pipefd[numCommands-1][2];
			
			if ((strcmp(cmds->commands[0].inputLine[0], "exit") == 0)) {
				exit(0);
			}
			
			for (int i = 0; i < numCommands; i++) {

				if (i < numCommands - 1) {
					if (pipe(pipefd[i]) < 0) {
						perror("pipe");
						exit(-1);
					}
				}
				for (int j = 0; cmds->commands[i].inputLine[j] != NULL; j++) {
					//printf("Command line %d: %s, output: %s, input: %s\n", i, cmds->commands[i].inputLine[j], cmds->commands[i].output, cmds->commands[i].input);
				}
				pid = fork();
				if (pid == 0) {
					// Set up input redirection
					if (cmds->commands[i].input != NULL && strcmp(cmds->commands[i].input, "pipe") == 0) {
						if(dup2(pipefd[i - 1][0], STDIN_FILENO) < 0) {// Read from the previous pipe
							perror("dup2 failed for pipe input");
							printf("array index: %d\n", i);
							int fd = fcntl(pipefd[i - 1][0], F_GETFD);
							printf("fd: %d\n", fd);
							exit(1);
							}
						if (i > 0) close(pipefd[i - 1][0]); // Close read end of the previous pipe

					}
					else if (cmds->commands[i].input != NULL) {
						int fd = open(cmds->commands[i].input, O_RDONLY, 0777);
						if (fd < 0) {
							perror("open");
							exit(-1);
						}
						if (dup2(fd, STDIN_FILENO) < 0) {
							perror("dup2 failed for input");
							exit(1);
						}
						
						close(fd);
					}

					// Set up output redirection
					if (cmds->commands[i].output != NULL && strcmp(cmds->commands[i].output, "pipe") == 0) {
						if (dup2(pipefd[i][1], STDOUT_FILENO) < 0) {// Write to the next pipe
							perror("dup2 failed for pipe output");
							printf("array index: %d\n", i);
							exit(1);
						}
						if (i < numCommands - 1) close(pipefd[i][1]); // Close write end of the current pipe
					}
					else if (cmds->commands[i].output != NULL) {
						int fd = open(cmds->commands[i].output, cmds->commands[i].append ? O_WRONLY | O_APPEND : O_WRONLY | O_CREAT | O_EXCL, 0777);
						if (fd < 0) {
							perror("open");
							exit(-1);
						}
						if (dup2(fd, STDOUT_FILENO) < 0) {
							perror("dup2 failed for output");
							exit(1);
						}
					
						close(fd);
					}
					//printf("execing: %s\n", cmds->commands[i].inputLine[0]);
					execvp(cmds->commands[i].inputLine[0], cmds->commands[i].inputLine);
					perror(cmds->commands[i].inputLine[0]);
					exit(-1);
					
				}
				else { 
					//for (int i = 0; i < numCommands - 1; i++) {
					if (fcntl(pipefd[i-1][0], F_GETFD) != -1) {
						close(pipefd[i-1][0]);
					}
					if (fcntl(pipefd[i][1], F_GETFD) != -1) {
						close(pipefd[i][1]);
					}
						//close(pipefd[i][1]);
						//close(pipefd[i][0]);
					//}
					if (!cmds->background) {
						//wpid = wait(&status);
						for (int i = 0; i < numCommands; i++) {
							
							wpid = wait(&status);
							if (WIFEXITED(status)) {
								//printf("Child exited with status %d\n", WEXITSTATUS(status));
							}
							else {
								//printf("Child process did not terminate normally\n");
							}
						}
						
					}
				}
			}
			
				free(cmds);
				
		}
		else exit(0);
	}
}

