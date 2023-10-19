NAME = barmark

CFLAGS = -D UNITY_OUTPUT_COLOR -g -Wall -Wextra -pedantic -Werror -std=c18

CC		= clang 

MAIN 	= src/main
OBJS 	= $(patsubst %.c, %.o, $(filter-out $(MAIN).c, $(wildcard src/*.c)))

TEST_MAIN = test/test
TEST_OBJS = $(patsubst %.c, %.o, $(filter-out $(TEST_MAIN).c, $(wildcard test/*.c)))

CC_CMD = $(CC) $(CFLAGS) -c -o $@ $<


SPEC_TEST_RANGES = 1-11,13,42-55,57-64,67-70,77-78,83-90,92-101,103-105,107-109,111-118,122-137,139-144,146-147,169-173,178-181,219-225,227-230,232-288,290-316,318-326,650-652

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
	@python3 spec_test/spec_tests.py --spec spec_test/spec.txt --program ./$(NAME) -r=$(SPEC_TEST_RANGES)
	@python3 spec_test/spec_tests.py --spec spec_test/myspec.txt --program ./$(NAME) 

my_spec_test: clean $(NAME)
	@python3 spec_test/spec_tests.py --spec spec_test/myspec.txt --program ./$(NAME) 

spec_test_single: clean $(NAME)
	@python3 spec_test/spec_tests.py --spec spec_test/spec.txt --program ./$(NAME) -n=$(t)

spec_test_debug_single: clean $(NAME)
	@export DEBUG=1; python3 spec_test/spec_tests.py --spec spec_test/spec.txt --program ./$(NAME) -n=$(t) --no-normalize

my_spec_test_debug_single: clean $(NAME)
	@export DEBUG=1; python3 spec_test/spec_tests.py --spec spec_test/myspec.txt --program ./$(NAME) -n=$(t) --no-normalize

spec_test_all: clean $(NAME)
	@python3 spec_test/spec_tests.py --spec spec_test/spec.txt --program ./$(NAME) 

test: clean $(NAME)_test
	@./$(NAME)_test

test_all: test spec_test