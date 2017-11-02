#include<iostream>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<cstdlib>
#include<cstring>
#include<cstdio>
using namespace std;

#define int_size sizeof(unsigned int)

void fork_process(int start_row, int end_row, const int dimension, int* shm_id);

int main(void)
{
	struct timeval start, end;
	int *shm_id = NULL;
	int dimension_input;
	int index;
	int process_number = 1;
	unsigned int matrix_result = 0;
	
	int A_shm_id;
	int B_shm_id;
	int C_shm_id;
	unsigned int* A_shm_p = NULL;
	unsigned int* B_shm_p = NULL;
	unsigned int* C_shm_p = NULL;
	cout << "Input the matrix demension:";
	cin >> dimension_input;
	A_shm_id = shmget(IPC_PRIVATE, int_size*dimension_input*dimension_input, IPC_CREAT | IPC_EXCL | 0666);
	B_shm_id = shmget(IPC_PRIVATE, int_size*dimension_input*dimension_input, IPC_CREAT | IPC_EXCL | 0666);
	C_shm_id = shmget(IPC_PRIVATE, int_size*dimension_input*dimension_input, IPC_CREAT | IPC_EXCL | 0666);

	shm_id = new int[3];
	shm_id[0] = A_shm_id;
	shm_id[1] = B_shm_id;
	shm_id[2] = C_shm_id;

	/*cout << "A_shm_id : " << A_shm_id << endl;
	cout << "B_shm_id : " << B_shm_id << endl;
	cout << "C_shm_id : " << C_shm_id << endl;*/
	A_shm_p = (unsigned int*)shmat(A_shm_id, NULL, 0);
	B_shm_p = (unsigned int*)shmat(B_shm_id, NULL, 0);
	
	for(unsigned int i = 0;i < dimension_input*dimension_input ;i++)
	{
		A_shm_p[i] = i;
	}
	memcpy(B_shm_p,A_shm_p,int_size*dimension_input*dimension_input);

	while(shmdt(A_shm_p)!=0)
	{
	}
	while(shmdt(B_shm_p)!=0)
	{
	}


	for(;process_number <= 16;process_number++)
	{
		matrix_result = 0;	
		int quotient = dimension_input/process_number;
		int remainder = dimension_input%process_number;
		int start_row = 0;
		gettimeofday(&start,0);
		for(int round = 1;round < process_number;round++)
		{
			fork_process(start_row, (start_row+quotient-1), dimension_input, shm_id);
			start_row+=quotient;
		}
		fork_process(start_row, (start_row+quotient+remainder-1), dimension_input, shm_id);

		//gettimeofday(&end,0);	

		for(int i = 0;i < process_number;i++)
		{
			wait(0);
		}
		
		gettimeofday(&end,0);
		
		C_shm_p = (unsigned int*)shmat(C_shm_id, NULL, 0);
		for(int i = 0;i < dimension_input*dimension_input ;i++)
		{
			matrix_result+=C_shm_p[i];
		}
		
		int sec = end.tv_sec-start.tv_sec;
		int usec = end.tv_usec-start.tv_usec;

		if(process_number!=1)
			cout << "Multiplying matrices using " << process_number << " processes" << endl;
		else
			cout << "Multiplying matrices using " << process_number << " process" << endl;
		printf("Elapsed time: %f sec, Checksum: %u\n",sec+(usec/1000000.0),matrix_result);
	}




	/*Delete 3 SHM*/
		shmctl(A_shm_id, IPC_RMID, NULL);
		shmctl(B_shm_id, IPC_RMID, NULL);
		shmctl(C_shm_id, IPC_RMID, NULL);
	/*------------*/

	if(shm_id!=NULL)
	{
		delete[] shm_id;
	}
	return 0;
}

void fork_process(int start_row, int end_row, const int dimension, int* shm_id)
{
	pid_t pid;
	pid = fork();
	if(pid>0)
	{
		return;
	}
	else if(pid<0)
	{
		cout << "Fork process error" << endl;
	}
	else
	{
		//cout << "start_row : " << start_row << endl;
		//cout << "end_row : " << end_row << endl;
		unsigned int* A_shm_p = (unsigned int*)shmat(shm_id[0], NULL, 0);
		unsigned int* B_shm_p = (unsigned int*)shmat(shm_id[1], NULL, 0);
		unsigned int* C_shm_p = (unsigned int*)shmat(shm_id[2], NULL, 0);
		for(; start_row <= end_row; start_row++)
		{//Control Row
			int row_ele_index = start_row*dimension;
			for(int j = 0; j < dimension ; j++)
			{//Control Column
				unsigned int total = 0;
				int col_ele_index = j;
				for(int k = 0; k<dimension; k++)
				{
					total += (unsigned int)(A_shm_p[row_ele_index+k]*B_shm_p[col_ele_index+k*dimension]);
				}
				C_shm_p[row_ele_index+col_ele_index] = total;
				//cout << "total :" << total << endl;
			}	
		}
		while(shmdt(A_shm_p)!=0)
		{
		}
		while(shmdt(B_shm_p)!=0)
		{
		}
		while(shmdt(C_shm_p)!=0)
		{
		}
		exit(0);
	}
}
