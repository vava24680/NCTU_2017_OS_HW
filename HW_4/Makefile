SOURCE=0416005_hw4.cpp
compile:
	g++ -g -pthread -Wall $(SOURCE)
run:compile
	./a.out
memcheck:compile
	valgrind --leak-check=full --log-file=memcheck.log ./a.out
clean:
	rm -f *.out output*.txt
correct_sort:
	g++ -g correct_sort.cpp -o correct_sort
	./correct_sort
	rm correct_sort
#####
