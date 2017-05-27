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

BIN = *.o

.PHONY: all clean fclean re

all: $(NAME)

$(NAME):
	@$(CC) $(FLAGS) $(NAME) $(SRCS)
	@echo " *** Executable compiled as $(NAME) ***"

clean:
	@/bin/rm -f $(BIN)
	@echo " *** $(BIN) Cleaned ***"

fclean: clean
	@/bin/rm -f $(NAME)

re: fclean all
