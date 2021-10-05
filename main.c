#include "microshell.h"

/* The first descriptor ([0]) connects to the read end of the pipe; the second
 * ([1]) connects to the write end.
 */

void	fatal_exit(void)
{
	write(2, "error: fatal\n", 13);
	exit(1);
}

void	child_process(t_cmd *cmd, char **env, int input_fd)
{
	int	err;

	char *argv[] = {cmd->path, NULL};
	if (cmd->prev == NULL)
	{
		close(cmd->fd[READ_END]);
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
	}
	else if (cmd->next == NULL)
	{
		close(cmd->fd[WRITE_END]);
		err = dup2(input_fd, STDIN_FILENO);
	}
	else
	{
		err = dup2(cmd->fd[WRITE_END], STDOUT_FILENO);
		if (err == -1)
			fatal_exit();
		err = dup2(input_fd, STDIN_FILENO);
	}
	if (err == -1)
		fatal_exit();
	err = execve(cmd->path, argv, env);
	if (err == -1)
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

	err = pipe(cmd->fd);
	if (err == -1)
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

t_cmd 	*add_cmds(char **argv)
{
	int		i;
	t_cmd	*cmd;
	t_cmd	*new;
	t_cmd	*prev;

	i = 1;
	cmd = NULL;
	prev = NULL;
	while (argv[i])
	{
		new = malloc(sizeof(t_cmd));
		new->path = argv[i];
		new->prev = prev;
		new->next = NULL;
		add_to_back(&cmd, new);
		prev = new;
		i++;
	}
	return (cmd);
}

int main(int argc, char **argv, char **env)
{
//	int	fd[2];
//	int	p_err;
//	int	i;
//	char buf[1000];
	t_cmd	*cmds;

	cmds = add_cmds(argv);
//	printf("%s\n", cmds->path);
//	cmds = cmds->next;
//	printf("%s\n", cmds->path);
	exec_list(cmds, env);
//	p_err = pipe(fd);
//	if (p_err == -1)
//		fatal_exit("pipe");

//	i = 1;
//	while (argv[i])
//	{
//		exec_cmds(fd, env, argv[i], i);
//		i++;
//	}
//	int ret = read(fd[0], buf, 1000);
//	buf[ret] = 0;
//	printf("%s", buf);
//	close(fd[0]);
//	close(fd[1]);
	return (0);
}
