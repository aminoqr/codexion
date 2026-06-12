# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/09 17:50:42 by aasylbye          #+#    #+#              #
#    Updated: 2026/06/12 18:26:21 by aasylbye         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= codexion

CC			:= cc
CFLAGS		:= -Wall -Wextra -Werror -pthread
INCLUDES	:= -Iinclude

SRC_DIR		:= src
OBJ_DIR		:= obj

SRCS		:= \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/parse.c \
	$(SRC_DIR)/time.c \
	$(SRC_DIR)/log.c \
	$(SRC_DIR)/heap.c \
	$(SRC_DIR)/heap_ops.c \
	$(SRC_DIR)/init.c \
	$(SRC_DIR)/dongle.c \
	$(SRC_DIR)/coder.c \
	$(SRC_DIR)/monitor.c \
	$(SRC_DIR)/cleanup.c

OBJS		:= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS		:= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
