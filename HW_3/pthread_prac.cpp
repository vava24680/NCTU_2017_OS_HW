#include<iostream>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<vector>
using namespace std;
#define NUM 5
vector<int> arr;

void *entry_point(void*)
{
	arr.resize(2);
	arr[0] = 1;
	arr[1] = 2;
	cout << "This is new thread" << endl;	
	return NULL;
}

int main(void)
{
	pthread_t tid;
	cout << "Before create pthread" << endl;
	pthread_create(&tid,NULL,entry_point,NULL);
	sleep(2);
	cout << "pthread has been created" << endl;
	for(int i=0; i < arr.size() ;i++)
	{
		cout << i << "st : " << arr[i] << endl;
	}
	return 0;
}
