all:
	mips-linux-gnu-gcc -c *.c *.s
	mips-linux-gnu-gcc *.o -o main -L ~/ubuntu/libelf/libelf-0.8.13/lib -lelf

clean:
	rm *.o
