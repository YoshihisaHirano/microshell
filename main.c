#include "microshell.h"

int	ft_strlen(const char *str)
{
	int	res;

	res = 0;
	while (str[res])
		res++;
	return (res);
}

void	fatal_exit(void)
{
	write(2, "error: fatal\n", 13);
	exit(1);
}

void	free_charr(char **args)
{
	int i;

	i = 0;
	while (args[i])
	{
		free(args[i]);
		i++;
	}
	free(args);
}

void	clear_lst(t_cmd *lst)
{
	t_cmd	*temp;

	temp = lst;
	while (lst->next)
	{
		lst = lst->next;
		free_charr(temp->args);
		free(temp);
		temp = lst;
	}
	free_charr(temp->args);
	free(lst);
}

int	assign_child_fd(t_cmd *cmd, int input_fd)
{
	int	err;

	err = 2;
	if (cmd->in_pipe == NO && cmd->out_pipe == YES)
	{
		close(cmd->fd[READ_END]);
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
	}
	else if (cmd->in_pipe == YES && cmd->out_pipe == NO)
	{
		close(cmd->fd[WRITE_END]);
		err = dup2(input_fd, STDIN_FILENO);
	}
	else if (cmd->in_pipe == YES && cmd->out_pipe == YES)
	{
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
		if (err == ERR_CODE)
			return (err);
		err = dup2(input_fd, STDIN_FILENO);
	}
	return (err);
}

void	child_process(t_cmd *cmd, char **env, int input_fd)
{
	int fd_res;
	int	err;

	fd_res = assign_child_fd(cmd, input_fd);
	if (fd_res == 2)
	{
		close(cmd->fd[WRITE_END]);
		close(cmd->fd[READ_END]);
	}
	if (fd_res == ERR_CODE)
		fatal_exit();
	err = execve(cmd->path, cmd->args, env);
	if (err == ERR_CODE)
	{
		write(2, "error: cannot execute ", 22);
		write(2, cmd->path, ft_strlen(cmd->path));
		write(2, "\n", 1);
	}
}

int	execute_cmd(t_cmd *cmd, char **env, int input_fd)
{
	int	err;
	int	pid;
	int status;

	err = pipe(cmd->fd);
	if (err == ERR_CODE)
		fatal_exit();
	pid = fork();
	if (pid > 0)
	{
		waitpid(pid, &status, 0);
		close(cmd->fd[WRITE_END]);
		if (input_fd > 0)
			close(input_fd);
	}
	else
		child_process(cmd, env, input_fd);
	return (cmd->fd[READ_END]);
}

void	exec_list(t_cmd *cmds, char **env)
{
	int input_fd;

	input_fd = 0;
	while (cmds)
	{
		input_fd = execute_cmd(cmds, env, input_fd);
		cmds = cmds->next;
	}
	close(input_fd);
}

void	add_to_back(t_cmd **start, t_cmd *elt)
{
	t_cmd	*temp;

	if (!(*start))
	{
		*start = elt;
		return ;
	}
	temp = *start;
	while (temp->next)
		temp = temp->next;
	temp->next = elt;
}

t_cmd	*create_cmd(char **args, t_cmd *prev)
{
	t_cmd	*new_elt;

	new_elt = malloc(sizeof(t_cmd));
	if (!new_elt)
		fatal_exit();
	new_elt->path = args[0];
	new_elt->args = args;
	new_elt->prev = prev;
	new_elt->next = NULL;
	return (new_elt);
}

/*
 * if cmd->prev == NULL && encountered_sym == '|' -> pipe_in = NO, pipe_out
 * = YES
 * if cmd->prev == NULL && encountered_sym == ';' -> pipe_in = NO, pipe_out = NO
 * if cmd->prev->pipe_out == YES && encountered sym == '|' -> pipe_in = YES,
 * pipe_out = YES
 * if cmd->prev->pipe_out == YES && encountered sym == ';' -> pipe_in = YES,
 * pipe_out = NO
 * for the last command in the list, the pipe_out = NO
 * */

void	add_cmd(char **args, t_cmd **cmds, char sym)
{
	t_cmd			*new_elt;
	static t_cmd	*prev;

	new_elt = create_cmd(args, prev);
	new_elt->in_pipe = NO;
	new_elt->out_pipe = NO;
	if ((!new_elt->prev || !new_elt->prev->out_pipe) && sym == '|')
		new_elt->out_pipe = YES;
	else if (new_elt->prev && (new_elt->prev->out_pipe && sym == '|'))
	{
		new_elt->in_pipe = YES;
		new_elt->out_pipe = YES;
	}
	else if (new_elt->prev && (new_elt->prev->out_pipe && (sym == ';' || !sym)))
		new_elt->in_pipe = YES;
	add_to_back(cmds, new_elt);
	prev = new_elt;
}

t_cmd	*parse_args(char **argv)
{
	int		start_idx;
	int		i;
	char	**prog_args;
	t_cmd	*cmds;
	int		j;

	start_idx = 0;
	i = 0;
	j = 0;
	cmds = NULL;
	argv++;
	while (1)
	{
		if (!argv[i] || !(strncmp(argv[i], "|", ft_strlen(argv[i])))
		|| !(strncmp(argv[i], ";", ft_strlen(argv[i]))))
		{
			if (i - start_idx > 0)
			{
				prog_args = malloc(sizeof(char *) * (i - start_idx) + 1);
				while (start_idx < i)
					prog_args[j++] = argv[start_idx++];
				prog_args[j] = NULL;
				j = 0;
				if (argv[i])
					add_cmd(prog_args, &cmds, argv[i][0]);
				else
					add_cmd(prog_args, &cmds, 0);
			}
			start_idx++;
		}
		if (!argv[i])
			break ;
		i++;
	}
	return (cmds);
}

int main(int argc, char **argv, char **env)
{
	t_cmd	*cmds;

	(void)env;
	(void)argc;
	cmds = parse_args(argv);
	exec_list(cmds, env);
	clear_lst(cmds);
	return (0);
}
