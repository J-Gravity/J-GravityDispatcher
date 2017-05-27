# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: scollet <marvin@42.fr>                     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2017/05/26 21:11:14 by scollet           #+#    #+#              #
#    Updated: 2017/05/26 21:11:16 by scollet          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = launch_dispatcher


.PHONY: all clean fclean re

all: $(NAME)

$(NAME):
	@gcc -o launch_dispatcher *.c

clean:
	@/bin/rm -f $(BINS)

fclean: clean
	@/bin/rm -f $(NAME)

re: fclean all
