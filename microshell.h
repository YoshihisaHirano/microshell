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

typedef struct s_cmd
{
	int 			fd[2];
	char			*path;
	struct s_cmd	*prev;
	struct s_cmd	*next;
} t_cmd;

#endif
