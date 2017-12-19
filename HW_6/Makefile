SOURCE=0416005_find.cpp
compile:
	g++ -g -Wall $(SOURCE) -o my_find
	cp my_find ~/os_hw6_test/
run:compile
	./my_find
memcheck:compile
	valgrind --leak-check=full --log-file=memcheck.log ./a.out
clean:
	rm -f *.out my_find
#####
