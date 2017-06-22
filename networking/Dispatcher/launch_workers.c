/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_workers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/21 17:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/21 19:55:43 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

size_t	handle_response(char *ptr, size_t size, size_t nmemb, void **response)
{
	void	*delete_me;
	char	*c_str;

	delete_me = *response;
	c_str = ft_strnew(size * nmemb);
	ft_memmove(c_str, ptr, size * nmemb);
	*response = ft_strjoin(*response, c_str);
	free(delete_me);
	free(c_str);
	return (size * nmemb);
}

void launch_workers(t_dispatcher *dispatcher, int worker_count)
{
	CURL		*curl;
	char		*json_response;
	int			worker_count;
	CURLcode	res;
	int i;

	curl_global_init(CURL_GLOBAL_ALL);
	/* Malloc empty string for response to be appended */
	if (!(json_response = (char *)calloc(1, 1)))
		return (0);
	/* get a curl handle */
	curl = curl_easy_init();
	if (curl)
	{
		/* Assemble GET URL */
		char *url;
		printf("launching %d workers!\n", worker_count);
		asprintf(&url, "ansiblecluster.42.us.org:8080/jgravity/%d", worker_count);
		printf("url '%s'\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* Don't Verify Cert */
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		/* Set the response callback */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_response);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		/* Parse Response */
		// lets just not and say we didn't
		/* always cleanup */
		free(url);
		free(json_response);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}