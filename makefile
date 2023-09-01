assembler.o: main.c
	gcc -Wall -ansi -pedantic -g main.c assembler.c linear_allocator.c linked_list.c macros.c reader.c table.c utils.c -o assembler -std=c11
