#ifndef MICROSHELL_H
#define MICROSHELL_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#define READ_END 0
#define WRITE_END 1
#define ERR_CODE -1
#define YES 1
#define NO 0

typedef struct s_cmd
{
	int 			fd[2];
	char			*path;
	char			**args;
	struct s_cmd	*prev;
	struct s_cmd	*next;
	int				in_pipe;
	int				out_pipe;
} t_cmd;

#endif
