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

SRCS = *.c \

CC = gcc
FLAGS = -o

.PHONY: all clean fclean re

all: $(NAME)

$(NAME):
	@$(CC) $(FLAGS) hello $(SRCS)
	@echo "Executable compiled as $(NAME)"

clean:
	@/bin/rm -f $(BINS)
	@echo "Files Cleaned"

fclean: clean
	@/bin/rm -f $(NAME)

re: fclean all
	@echo "Executable recompiled as $(NAME)"
