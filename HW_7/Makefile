SOURCE=0416005_hw7.cpp
compile:
	g++ -g -Wall $(SOURCE)
run:compile
	./a.out
memcheck:compile
	valgrind --leak-check=full --log-file=memcheck.log ./a.out
clean:
	rm -f *.out
#####
