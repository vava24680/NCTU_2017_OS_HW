#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
using namespace std;
#define FILENUMBER 10000
int main(int argc, char* argv[])
{
	string mount_point;
	string largefile_path;
	unsigned int number = 0;
	long second, u_second;
	struct timeval start;
	struct timeval end;
	char filename[15];
	fstream fout;
	cout << "Please specify the mount point(With ABSOLUTE PATH and the trailing slash, e.g:\"./\") :";
	cin >> mount_point;
	if(mount_point[mount_point.length() - 1] != '/')
		mount_point = mount_point + '/';
	largefile_path = ("filefrag -v " + mount_point + "largefile.txt");
	gettimeofday(&start,NULL);
	/*Make the 100MB-volume fragmented*/
	for(;number < FILENUMBER; number++)
	{
		sprintf(filename,"%d",number);
		fout.open( (mount_point + string(filename)).c_str() ,ios::out | ios::trunc );
		/*Write Something to the file*/
		for(unsigned int i = 0; i < 7500; i++)
			fout << "a";
		fout.close();
	}
	sync();//Enforce the data written to the disk
	for(unsigned int i = 0; i < FILENUMBER; i = i + 7)
	{
		sprintf(filename,"%d",i);
		remove( (mount_point + string(filename)).c_str() );
	}
	fout.open( (mount_point + "largefile.txt").c_str() ,ios::out | ios::trunc );
	for(unsigned int i = 0; i < 67108863; i++)
		fout << "a";
	fout << endl;
	fout.close();
	sync();//Enforce the data written to the disk

	for(unsigned int i = 1; i < FILENUMBER; i++)
	{
		if(i % 7 == 0)
			continue;
		sprintf(filename,"%d",i);
		remove( (mount_point + string(filename)).c_str() );
	}
	/*End of fragmenting the 100MB-volume*/	
	gettimeofday(&end,NULL);
	system(largefile_path.c_str());
	second = end.tv_sec - start.tv_sec;
	u_second = end.tv_usec - start.tv_usec;
	cout << "Elapsed time: " << ( second + (u_second / 1000000.0) ) << " sec" << endl;
	return 0;	
}
