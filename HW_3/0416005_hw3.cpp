#include <iostream>
#include <fstream>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <cstring>
#include <string>
using namespace std;

#define INPUTFILE_1 "input_1.txt"
#define INPUTFILE_2 "input_2.txt"
#define INPUTFILE_3 "input_3.txt"
#define INPUTFILE_4 "input_4.txt"
#define OUTPUTFILE_1 "output1.txt"
#define OUTPUTFILE_2 "output2.txt"
typedef struct params {
	int t_num;
	int work_type;
} PARAM;

/*Global variables declaration*/
int num_eles;           //number of elements in the file
int* ele_arr = NULL;    //pointer point to input array
int* ele_arr_2 = NULL;
sem_t* start_sem = NULL;
sem_t* complete_sem = NULL;
vector<int> start_index;//The start index that each thread uses
vector<int> end_index;  //The end index that each thread uses
/*----------------------------*/
void mem_release(void);
void write_file(int mode);
void init_sems(sem_t* sem_head, int sem_num, unsigned int init_value)
{
	for(int i=0; i<sem_num; i++)
	{
		sem_init(sem_head+i, 0, init_value);
	}
	return;
}

void swap(int* target_arr, int l_index, int r_index)
{
	int temp = target_arr[l_index];
	target_arr[l_index] = target_arr[r_index];
	target_arr[r_index] = temp;
	return;
}

int partition(int* target_arr, int start_pos, int end_pos)
{
	int pivot = end_pos;
	int right_flag = start_pos - 1;
	if(start_pos < end_pos)
	{
		for(int i=start_pos; i<end_pos; i++)
		{
			if(target_arr[i] < target_arr[pivot])
			{
				right_flag++;
				swap(target_arr, right_flag, i);
			}
		}
		//cout << "right_flag :" << right_flag << endl;
		right_flag++;
		swap(target_arr, right_flag, pivot);
		pivot = right_flag;
	}
	return pivot;
}

void bubble_sort(int* target_arr, int start_pos, int end_pos)
{
	if(start_pos < end_pos)
	{
		for(int i = 0; i < (end_pos - start_pos); i++)
		{
			for(int j = start_pos; j < (end_pos - i); j++)
			{
				if(target_arr[j] > target_arr[j+1])
				{
					swap(target_arr, j, j+1);
				}
			}
		}
	}
	return;
}


void* threads_work(void* param_in)
{
	PARAM* param = (PARAM*)param_in;
	int thread_number = param->t_num;
	int middle;
	sem_wait(start_sem + thread_number - 1);
	//cout << "Thread " << param->t_num << "start working" << endl;
	if(param->work_type == 0)
	{//Partition
		//cout << "start : " << start_index[thread_number] << ", end : " << end_index[thread_number] << endl;
		middle = partition(ele_arr, start_index[thread_number], end_index[thread_number]);
		//cout << "middle : " << middle << endl;

		start_index[2 * thread_number] = start_index[thread_number];
		end_index[2 * thread_number] = middle-1;
		start_index[2 * thread_number + 1] = middle+1;
		end_index[2 * thread_number + 1] = end_index[thread_number];
		sem_post(start_sem + 2*thread_number - 1);
		sem_post(start_sem + 2*thread_number);
		if(thread_number==1)
		{
			sem_wait(complete_sem + 7);
			sem_wait(complete_sem + 8);
			sem_wait(complete_sem + 9);
			sem_wait(complete_sem + 10);
			sem_wait(complete_sem + 11);
			sem_wait(complete_sem + 12);
			sem_wait(complete_sem + 13);
			sem_wait(complete_sem + 14);

			sem_post(complete_sem);/*Notify the main thread that sort is done completely*/
		}
	}
	else
	{//Bubble Sort
		bubble_sort(ele_arr, start_index[thread_number], end_index[thread_number]);
		sem_post(complete_sem + thread_number - 1);
	}

	pthread_detach(pthread_self());

	return NULL;

}

int* file_input(void)
{
	string input;
	cout << "Please input a file name :";
	cin >> input;
	int* l_ele_arr=NULL;
	fstream fin;
	fin.open(input.c_str(),ios::in);
	if(!fin)
	{
		cout << "Open file failed." << endl;
		return NULL;
	}
	fin >> num_eles;
	l_ele_arr = new int[num_eles];
	for(int i=0; i < num_eles; i++)
	{
		fin >> l_ele_arr[i];
	}
	fin.close();
	return l_ele_arr;
}

void single_quick_sort(int left_index, int right_index,int level)
{
	if(level==3)
	{
		bubble_sort(ele_arr_2, left_index, right_index);
		return;
	}
	int middle = partition(ele_arr_2, left_index, right_index);
	single_quick_sort(left_index, middle - 1, level + 1);
	single_quick_sort(middle + 1, right_index, level + 1);
	return;
}

int main(void)
{
	ele_arr = file_input();
	if(ele_arr==NULL)
		return 0;

	struct timeval m_start, m_end, s_start, s_end;
	int sec,usec;
	vector<pthread_t> pthread_list;
	vector<PARAM> parameter_list;

	ele_arr_2 = new int[num_eles];
	memcpy(ele_arr_2, ele_arr, sizeof(int)*num_eles);

	start_sem = new sem_t[15];
	complete_sem = new sem_t[15];
	/*Initial start_sem and complete_sem this two semaphores*/
		init_sems(start_sem, 15, (unsigned int)0);
		init_sems(complete_sem, 15, (unsigned int)0);
	/*------------------------------------------------------*/

	/*--------------Multi-thread start----------------------*/
	parameter_list.resize(16);
	pthread_list.resize(16);
	start_index.resize(16);
	end_index.resize(16);
	for(int i=1; i < 16; i++)
	{
		parameter_list[i].t_num = i;
		i > 7 ? parameter_list[i].work_type = 1 : parameter_list[i].work_type = 0;
	}
	start_index[1] = 0;
	end_index[1] = num_eles-1;
	gettimeofday(&m_start,0);
	pthread_create(&pthread_list[1], NULL, &threads_work, (void*)(&parameter_list[1]));
	pthread_create(&pthread_list[2], NULL, &threads_work, (void*)(&parameter_list[2]));
	pthread_create(&pthread_list[3], NULL, &threads_work, (void*)(&parameter_list[3]));
	pthread_create(&pthread_list[4], NULL, &threads_work, (void*)(&parameter_list[4]));
	pthread_create(&pthread_list[5], NULL, &threads_work, (void*)(&parameter_list[5]));
	pthread_create(&pthread_list[6], NULL, &threads_work, (void*)(&parameter_list[6]));
	pthread_create(&pthread_list[7], NULL, &threads_work, (void*)(&parameter_list[7]));
	pthread_create(&pthread_list[8], NULL, &threads_work, (void*)(&parameter_list[8]));
	pthread_create(&pthread_list[9], NULL, &threads_work, (void*)(&parameter_list[9]));
	pthread_create(&pthread_list[10], NULL, &threads_work, (void*)(&parameter_list[10]));
	pthread_create(&pthread_list[11], NULL, &threads_work, (void*)(&parameter_list[11]));
	pthread_create(&pthread_list[12], NULL, &threads_work, (void*)(&parameter_list[12]));
	pthread_create(&pthread_list[13], NULL, &threads_work, (void*)(&parameter_list[13]));
	pthread_create(&pthread_list[14], NULL, &threads_work, (void*)(&parameter_list[14]));
	pthread_create(&pthread_list[15], NULL, &threads_work, (void*)(&parameter_list[15]));

	sem_post(start_sem);
	sem_wait(complete_sem);
	gettimeofday(&m_end,0);
	sec = m_end.tv_sec - m_start.tv_sec;
	usec = m_end.tv_usec - m_start.tv_usec;
	cout << "Multi thread used time : " << sec + (usec/1000000.0) << " sec" << endl;
	write_file(0);
	/*Multi-thread end*/

	/*Single Thread*/
	gettimeofday(&s_start,0);
	single_quick_sort(0, num_eles - 1, 0);
	gettimeofday(&s_end,0);
	write_file(1);
	sec = s_end.tv_sec - s_start.tv_sec;
	usec = s_end.tv_usec - s_start.tv_usec;
	cout << "Single thread used time : " << sec + (usec/1000000.0) << " sec" << endl;
	mem_release();
	return 0;
}

void write_file(int mode)
{
	fstream fout;
	if(mode==0)
	{
		fout.open(OUTPUTFILE_1, ios::out | ios::trunc);

	}
	else
	{
		fout.open(OUTPUTFILE_2, ios::out | ios::trunc);
	}
	if(!fout)
	{
		cout << "File open failed" << endl;
	}
	for(int i=0; i < num_eles; i++)
	{
		if(mode==0)
			fout << ele_arr[i] << " ";
		else
			fout << ele_arr_2[i] << " ";
	}
	fout.close();
	return;
}

void mem_release(void)/*Release global variables' memory which ther were created by dynamically allocated*/
{
	if(ele_arr != NULL) delete[] ele_arr;
	if(ele_arr_2 != NULL) delete[] ele_arr_2;
	if(start_sem != NULL) delete[] start_sem;
	if(complete_sem != NULL) delete[] complete_sem;
	return;
}
