NAME = barmark

FILES 	= lex util symbols
MAIN 	= src/main
SRCS 	= $(addprefix src/, $(addsuffix .c, $(FILES)))
OBJS 	= $(addprefix src/, $(addsuffix .o, $(FILES)))

TEST_HARNESS = vendor/unity/unity
TEST_MAIN =
TEST_SRCS = $(addsuffix .c, $(TEST_HARNESS))
TEST_OBJS = $(addsuffix .o, $(TEST_HARNESS))

all: $(NAME)

$(OBJS) $(MAIN).o: $(SRCS) $(MAIN).c

$(TEST_OBJS): $(TEST_SRCS) $(SRCS)

$(NAME): $(OBJS) $(MAIN).o
	@cc $(OBJS) $(MAIN).o -o $(NAME)

clean:
	rm $(OBJS)

re: clean all

run: $(NAME)
	@./$(NAME) test_files/basic.md

$(NAME)_test: $(OBJS) $(TEST_OBJS)
	@cc $(OBJS) $(TEST_OBJS) -o $(NAME)_test

# test: $(NAME)_test
# 	@./$(NAME)_test 

.SILENT: $(OBJS)