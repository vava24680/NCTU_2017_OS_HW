#include <iostream>
#include <sys/time.h>
#include <fstream>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char* argv[])
{
	struct timeval start,end;
	unsigned int second, u_second;
	gettimeofday(&start,NULL);
	/*Make the 100MB-volume fragmented*/
	system("ls -al");

	/*End of fragmenting th 100MB-volume*/	
	gettimeofday(&end,NULL);
	//system("filefrag -v largefile.txt");
	second = end.tv_sec - start.tv_sec;
	u_second = end.tv_usec - start.tv_usec;
	cout << "Elapsed time: " << (second + u_second/1000000.0) << " sec" << endl;
	return 0;	
}
