#include "microshell.h"

//for debug
//void	ft_putnbr_fd(int nb, int fd);
//void	ft_putchar_fd(char c, int fd);

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

/*
 * 1. [connected] -> [disconnected] == the process is the last in the pipeline
 * 2. [connected] -> [connected] == (same as position in the middle of the
 * pipeline)
 * 3. [disconnected] -> [connected] || [no process] -> [connected] == (same
 * as the first process in the pipeline)
 * 4. [disconnected] -> [disconnected] == just your regular execve
 *
 *
 * if this process is disconnected but the previous one is connected, it
 * should read from the input fd and close the write fd
 *
 * if this process is connected and the prev one is NULL or disconnected,
 * it should write to the write fd and close read fd
 *
 */

void	child_process(t_cmd *cmd, char **env, int input_fd)
{
	int	err;

	err = 0;
	if ((cmd->prev == NULL || !cmd->prev->connected) && cmd->connected)
	{
		close(cmd->fd[READ_END]);
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
	}
	else if (cmd->prev && (cmd->next == NULL || !cmd->next->connected) &&
	cmd->connected)
	{
		close(cmd->fd[WRITE_END]);
		err = dup2(input_fd, STDIN_FILENO);
	}
	else if (cmd->connected && cmd->next->connected)
	{
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
		if (err == ERR_CODE)
			fatal_exit();
		err = dup2(input_fd, STDIN_FILENO);
	}
	if (err == ERR_CODE)
		fatal_exit();
	err = execve(cmd->path, cmd->args, env);
	if (err == ERR_CODE)
	{
		write(2, "Program failed: ", 16);
		write(2, cmd->path, 7);
		write(2, "\n", 1);
	}
}

int	execute_cmd(t_cmd *cmd, char **env, int input_fd)
{
	int	err;
	int	pid;
	int status;

//	ft_putnbr_fd(input_fd, 2);
//	ft_putchar_fd('\n', 2);
	err = pipe(cmd->fd);
//	ft_putnbr_fd(cmd->fd[READ_END], 2);
//	ft_putchar_fd(' ', 2);
//	ft_putnbr_fd(cmd->fd[WRITE_END], 2);
//	ft_putchar_fd('\n', 2);
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
	if (!cmd->connected)
		return (0);
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

t_cmd	*create_cmd(char **args, t_cmd *prev, int connected)
{
	t_cmd	*new_elt;

	new_elt = malloc(sizeof(t_cmd));
	if (!new_elt)
		fatal_exit();
	new_elt->path = args[0];
	new_elt->args = args;
	new_elt->connected = connected;
	new_elt->prev = prev;
	new_elt->next = NULL;
	return (new_elt);
}

/*
 * if cmd->prev == NULL && encountered_sym == '|' -> pipe_in = NO, pipe_out
 * = YES
 * if cmd->prev == NULL && encountered_sym == ';' -> pipe_in = NO,
 * */

void	add_cmd(char **args, t_cmd **cmds, char sym)
{
	t_cmd			*new_elt;
	static t_cmd	*prev;
	int				connected;

	if (sym == '|')
		connected = YES;
	else
		connected = NO;
	new_elt = create_cmd(args, prev, connected);
	add_to_back(cmds, new_elt);
	prev = new_elt;
}

void	print_charr(char **args)
{
	int i = 0;
	while (args[i])
		printf("%s ", args[i++]);
	printf("\n");
}

/* argv =
 * { "/bin/ls", "|", "/usr/bin/grep", "microshell", ";", "/bin/echo", "i",
 * "love", "my", "microshell", NULL }*/

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
			prog_args = malloc(sizeof(char *) * (i - start_idx) + 1);
			while (start_idx < i)
				prog_args[j++] = argv[start_idx++];
			prog_args[j] = NULL;
			j = 0;
			start_idx++;
			if (argv[i])
				add_cmd(prog_args, &cmds, argv[i][0]);
			else
				add_cmd(prog_args, &cmds, 0);
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
	return (0);
}
