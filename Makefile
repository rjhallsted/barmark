NAME = barmark

CFLAGS = -g -Wall -Wextra -pedantic -Werror -std=c18

MAIN 	= src/main
OBJS 	= $(patsubst %.c, %.o, $(filter-out $(MAIN).c, $(wildcard src/*.c)))

TEST_MAIN = test/test
TEST_OBJS = $(patsubst %.c, %.o, $(filter-out $(TEST_MAIN).c, $(wildcard test/*.c)))

CC_CMD = $(CC) $(CFLAGS) -c -o $@ $<

SPEC_TEST_MAX = 4

all: $(NAME)

$(NAME): $(OBJS) $(MAIN).o
	@$(CC) $(OBJS) $(MAIN).o -o $(NAME)

%.o: %.c
	@$(CC_CMD)

clean:
	@rm -f $(OBJS) $(MAIN).o $(NAME) $(TEST_OBJS) $(TEST_MAIN).o $(NAME)_test vendor/unity/unity.o

re: clean all

run: $(NAME)
	@./$(NAME) test_files/basic.md

vendor/unity/unity.o: vendor/unity/unity.c vendor/unity/unity.h vendor/unity/unity_internals.h
	@$(CC_CMD)

$(NAME)_test: $(OBJS) $(TEST_OBJS) $(TEST_MAIN).o vendor/unity/unity.o
	@$(CC) $(CFLAGS) -o $(NAME)_test $(OBJS) $(TEST_OBJS) $(TEST_MAIN).o vendor/unity/unity.o 

spec_test: clean $(NAME)
	@python3 spec_test/spec_tests.py --spec spec_test/spec.txt --program ./$(NAME) -r=1-$(SPEC_TEST_MAX)

test: clean $(NAME)_test
	@./$(NAME)_test

test_all: test spec_test