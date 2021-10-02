#include "microshell.h"

/* The first descriptor ([0]) connects to the read end of the pipe; the second
 * ([1]) connects to the write end.
 */

void	fatal_exit(char *arg)
{
	write(2, arg, 7);
	write(2, "error: fatal\n", 13);
	exit(1);
}

//void	parent_process(char **env, int *fd, int child_pid)
//{
//	int	status;
//	int err;
//
//	waitpid(child_pid, &status, 0);
//	close(fd[1]);
//	char *argv[] = {"/bin/echo", NULL};
//	err = dup2(fd[0], STDIN_FILENO);
//	close(fd[0]);
//	if (err == -1)
//		fatal_exit();
//	printf("%d - status\n", status);
//	execve("/bin/cat", argv, env);
//}

void	child_process(char **env, int *fd, char *arg, int i)
{
	int	err;

	printf("I am child\n");
//	if (i != 1)
//	{
//		char buf[1000];
//		int ret = read(fd[0], buf, 1000);
//		buf[ret] = 0;
//		printf("%s\n", buf);
//	}
	char *argv[] = {arg, NULL};
	printf("%d %d\n", fd[0], fd[1]);
	err = dup2(fd[0], STDIN_FILENO);
	if (err == -1)
		fatal_exit("dup_in");
	err = dup2(fd[1], STDOUT_FILENO);
	if (err == -1)
		fatal_exit("dup_out");
	close(fd[0]);
	close(fd[1]);
	execve(arg, argv, env);
	write(2, "h\n", 2);
}

void	exec_cmds(int *fd, char **env, char *arg, int i)
{
	int	pid;
	int status;

	pid = fork();
	if (pid == -1)
		fatal_exit("fork");
	if (pid > 0)
	{
		waitpid(pid, &status, WUNTRACED);
		printf("%d -- status\n", status);
	}
	else
		child_process(env, fd, arg, i);
}

int main(int argc, char **argv, char **env)
{
	int	fd[2];
	int	p_err;
	int	i;
	char buf[1000];

	p_err = pipe(fd);
	if (p_err == -1)
		fatal_exit("pipe");
	i = 1;
	while (argv[i])
	{
		exec_cmds(fd, env, argv[i], i);
		i++;
	}
	int ret = read(fd[0], buf, 1000);
	buf[ret] = 0;
	printf("%s", buf);
	close(fd[0]);
	close(fd[1]);
	return (0);
}
