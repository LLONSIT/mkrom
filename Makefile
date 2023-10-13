all:
	gcc -g -c *.c
	gcc *.o -lelf -o makerom

clean:
	rm *.o makerom
