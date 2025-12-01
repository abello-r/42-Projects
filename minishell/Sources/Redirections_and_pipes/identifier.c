/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   identifier.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pausanch <pausanch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 17:49:40 by pausanch          #+#    #+#             */
/*   Updated: 2025/01/22 20:21:44 by pausanch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Includes/minishell.h"

static void	perror_auxiliar(char *str, int code)
{
	perror(str);
	exit(code);
}

/*
	Here we established if the redir is for input or output,
	Depends on that, we make the choice about how to handle it.
*/
void	setup_redirections(t_cmd *cmd)
{
	int	fd;

	if (cmd->input_file)
	{
		fd = open(cmd->input_file, O_RDONLY);
		if (fd == -1)
			perror_auxiliar(cmd->input_file, 1);
		else if (dup2(fd, STDIN_FILENO) == -1)
			perror_auxiliar("dup2", 1);
		close(fd);
	}
	if (cmd->output_file)
	{
		if (cmd->append)
			fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd == -1)
			perror_auxiliar(cmd->output_file, 1);
		if (dup2(fd, STDOUT_FILENO) == -1)
			perror_auxiliar("dup2", 1);
		close(fd);
	}
}

/*
   1. Sets up input redirection from previous pipe if it exists
   2. Sets up output redirection to next pipe if it exists
   3. Closes all pipe file descriptors to prevent leaks
*/
void	pipes_handler(int *prev_pipe, int *pipe_fd)
{
	if (prev_pipe && dup2(prev_pipe[0], STDIN_FILENO) == -1)
		perror_auxiliar("dup2", 1);
	if (pipe_fd && dup2(pipe_fd[1], STDOUT_FILENO) == -1)
		perror_auxiliar("dup2", 1);
	if (prev_pipe)
	{
		close(prev_pipe[0]);
		close(prev_pipe[1]);
	}
	if (pipe_fd)
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
	}
}

void	execute_child(t_data *data, t_cmd *cmd, int *prev_pipe, int *pipe_fd)
{
	char	*cmd_path;
	char	*expanded_var;

	pipes_handler(prev_pipe, pipe_fd);
	setup_redirections(cmd);
	if (is_builtin(cmd->argv[0]))
	{
		ft_execute_builtin(data);
		exit(0);
	}
	else
	{
		expanded_var = ft_expand_env_cmd_nfd(data, cmd->argv[0]);
		cmd_path = expand_handler(data, cmd, expanded_var);
		replace_argv(cmd, data, expanded_var);
		if (execve(cmd_path, cmd->argv, data->envp) == -1)
		{
			free(cmd_path);
			perror_auxiliar(cmd_path, 126);
		}
	}
}

void	ft_execute_commands(t_data *data)
{
	t_cmd	*current;
	int		prev_pipe[2];
	int		curr_pipe[2];

	current = data->cmds;
	prev_pipe[0] = -1;
	prev_pipe[1] = -1;
	while (current)
	{
		if (ft_strcmp(current->argv[0], "exit") == 0)
			return (ft_exit(data));
		if (current->next && pipe(curr_pipe) == -1)
		{
			g_status = 1;
			return (perror("pipe"));
		}
		if (is_builtin(current->argv[0])
			&& !current->next && prev_pipe[0] == -1)
			execute_single_builtin(data, current);
		else
			pid_handler(data, current, prev_pipe, curr_pipe);
		current = current->next;
	}
	while (wait(NULL) > 0)
		continue ;
}
