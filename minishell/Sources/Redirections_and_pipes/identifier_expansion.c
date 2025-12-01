/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   identifier_expansion.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pausanch <pausanch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/20 17:49:40 by pausanch          #+#    #+#             */
/*   Updated: 2025/01/22 20:22:31 by pausanch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../Includes/minishell.h"

void	replace_argv(t_cmd *cmd, t_data *data, char *expanded_var)
{
	int	i;

	i = 0;
	while (cmd->argv[i])
	{
		if (ft_strchr(cmd->argv[i], '$'))
		{
			expanded_var = ft_expand_env_cmd_nfd(data, cmd->argv[i]);
			if (expanded_var)
			{
				free(cmd->argv[i]);
				cmd->argv[i] = expanded_var;
			}
		}
		i++;
	}
}

char	*expand_handler(t_data *data, t_cmd *cmd, char *expanded_var)
{
	char	*cmd_path;

	if (expanded_var && expanded_var[0] != '\0')
	{
		cmd_path = find_command_path(data, expanded_var);
		if (!cmd_path)
			handle_command_not_found(expanded_var);
	}
	else
	{
		cmd_path = find_command_path(data, cmd->argv[0]);
		if (!cmd_path)
			handle_command_not_found(cmd->argv[0]);
	}
	return (cmd_path);
}

char	*ft_expand_env_cmd_nfd(t_data *data, char *content)
{
	char	*org_content;
	char	*special_result;
	char	*temp;

	if (!content)
		return (NULL);
	special_result = handle_special_expansion(content);
	if (special_result)
		return (special_result);
	org_content = ft_strdup(content);
	if (!org_content)
		return (NULL);
	if (content[0] == '$')
	{
		temp = find_env_value(data, content, org_content);
		free(org_content);
		return (temp);
	}
	free(org_content);
	return (ft_strdup(""));
}
