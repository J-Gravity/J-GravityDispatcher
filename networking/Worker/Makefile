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

NAME = launch_worker # Pending possible changes

SRCS = *.c # Pending updates

CC = gcc
FLAGS = -o
FRAMEWORK = -framework OpenCL # Required for Workers to function

BIN = *.o # Pending updates

.PHONY: all clean fclean re

all: $(NAME)

$(NAME):
	@$(CC) $(FLAGS) $(NAME) $(SRCS) $(FRAMEWORK)
	@echo "*** Executable compiled as $(NAME) ***"

clean:
	@/bin/rm -f $(BIN)
	@echo "*** $(BIN)Cleaned *** "

fclean: clean
	@/bin/rm -f $(NAME)

re: fclean all
