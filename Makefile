all:
	@gcc encoder.c -o encoder

clean:
	@rm -f encoder

memorycheck:
	@valgrind ./encoder
