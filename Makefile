

C_FILES= main.c lex.c util.c

build:
	@cc -o barmark $(C_FILES)

test: build
	@./barmark test_files/basic.md