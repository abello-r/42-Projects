/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   identifier_pid.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pausanch <pausanch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 13:19:17 by pausanch          #+#    #+#             */
/*   Updated: 2025/01/22 13:26:35 by pausanch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Includes/minishell.h"

extern int	g_status;

void	pid_handler_aux(int *p_pipe, int *c_pipe, t_cmd *current, pid_t pid)
{
	int	status;

	status = 0;
	if (p_pipe[0] != -1)
	{
		close(p_pipe[0]);
		close(p_pipe[1]);
	}
	if (current->next)
	{
		p_pipe[0] = c_pipe[0];
		p_pipe[1] = c_pipe[1];
	}
	if (!current->next)
	{
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
			g_status = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			g_status = 128 + WTERMSIG(status);
	}
}

pid_t	pid_creation(int *curr_pipe)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		if (curr_pipe)
		{
			close(curr_pipe[0]);
			close(curr_pipe[1]);
		}
		g_status = 1;
		return (-1);
	}
	return (pid);
}

void	heredoc_handler(t_token *temp, t_data *data)
{
	int		status;
	int		fd;

	status = 0;
	fd = 0;
	if (ft_strcmp(temp->type, "HEREDOC") == 0)
	{
		ft_heredoc(data);
		fd = open("heredoc.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		close(fd);
		exit(EXIT_SUCCESS);
	}
}

void	pid_handler(t_data *data, t_cmd *current, int *p_pipe, int *c_pipe)
{
	t_token	*temp;
	int		*prev_pipe_ptr;
	int		*curr_pipe_ptr;
	pid_t	pid;

	prev_pipe_ptr = NULL;
	curr_pipe_ptr = NULL;
	temp = data->token;
	pid = pid_creation(c_pipe);
	if (pid == -1)
		return ;
	else if (pid == 0)
	{
		while (temp)
		{
			heredoc_handler(temp, data);
			temp = temp->next;
		}
		if (p_pipe[0] != -1)
			prev_pipe_ptr = p_pipe;
		if (current->next)
			curr_pipe_ptr = c_pipe;
		execute_child(data, current, prev_pipe_ptr, curr_pipe_ptr);
	}
	pid_handler_aux(p_pipe, c_pipe, current, pid);
}
