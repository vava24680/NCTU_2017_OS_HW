#include<iostream>
#include<fstream>
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/time.h>
#include<string>
#include<vector>
#include<queue>
#include<utility>

using namespace std;
#define INPUTFILE "input.txt"
#define MAX_THREAD_NUMBER 8
/*Global variables*/
vector<int> input_array;
int num_eles = 0;

class ThreadPool;
typedef int (*SORT_FUNCTION)(int, int);
typedef struct task
{
	int task_number;
	pair<int, int> border_pair_;
	SORT_FUNCTION sort_func_ptr_;
} TASK;

typedef struct thread_information
{
	int thread_number;
	pthread_t thread_id;
	sem_t execute_event;
	ThreadPool* pool_instance;
} THREAD;

void write_to_file(string filename);
void swap(int left_index, int right_index)
{
	int temp = input_array[left_index];
	input_array[left_index] = input_array[right_index];
	input_array[right_index] = temp;
	return;
}

int partition(int start_index, int end_index)
{
	int pivot = end_index;
	int right_flag = start_index - 1;
	if(start_index < end_index)
	{
		for(int i = start_index; i < end_index; i++)
		{
			if(input_array[i] < input_array[pivot])
			{
				right_flag++;
				swap(right_flag, i);
			}
		}
		right_flag++;
		swap(right_flag, pivot);
		pivot = right_flag;
	}
	return pivot;
}

int bubble_sort(int start_index, int end_index)
{
	int run_times = end_index - start_index;
	for(int i = 0; i < run_times; i++)
	{
		for(int j = start_index; j <  (end_index - i); j++)
		{
			if(input_array[j] > input_array[j+1])
			{
				swap(j, j+1);
			}
		}
	}
	return -1;//Return value is not important, just for the function pointer
}

void file_input(void)
{
	fstream fin;
	fin.open(INPUTFILE);
	if(!fin)
	{
		cout << "Open file failed." << endl;
		return;
	}
	fin >> num_eles;
	input_array.resize(num_eles);
	for(int i=0; i < num_eles; i++)
	{
		fin >> input_array[i];
	}
	fin.close();
	return;
}

class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(int total_thread);
	~ThreadPool();
	void new_one_thread(void);
	void active_pool(void);
	void set_thread_number(int thread_number);
	void initial_all_task(void);
	void add_task(TASK* task_in);
	void push_idle_thread(THREAD* free_thread);
	void thread_dispatcher();
private:
	int total_dispatch_times;
	int total_thread_;
	bool ready_destroy_;
	queue<TASK*> task_queue;
	queue<THREAD*> free_thread_queue_;
	vector<THREAD*> thread_list_;
	TASK* task_list;
private:
	sem_t* task_queue_mutex_;	//Same function as mutex semaphore in P-C problem.
	sem_t* task_queue_full_;	//Same function as full semaphore in P-C problem.
	sem_t* free_thread_queue_full_;	//Same function as full semaphore in P-C problem.
	sem_t* free_thread_queue_mutex_;//Same function as mutex semaphore in P-C problem.
	sem_t* finish_semaphore_list_;	//Use to notify main thread that bubble sort are all done.
private:
	void semaphore_enable(void);
	void semaphore_disable(void);
	static void* thread_start_routine(void* run_data);
};

ThreadPool::ThreadPool()
{//Default constructor
	task_list = new TASK[16];
	total_dispatch_times = 0;
	this->semaphore_enable();
	this->initial_all_task();
	this->semaphore_enable();
}
ThreadPool::ThreadPool(int total_thread)
{//Constructor with total_thread number parameter
	this->total_thread_ = total_thread;
	task_list = new TASK[16];
	total_dispatch_times = 0;
	this->semaphore_enable();
	this->initial_all_task();
	this->semaphore_enable();
}

void ThreadPool::set_thread_number(int thread_number)
{
	this->total_thread_ = thread_number;
}

void ThreadPool::semaphore_enable(void)
{
	this->free_thread_queue_mutex_ = new sem_t;
	this->free_thread_queue_full_  = new sem_t;
	this->task_queue_mutex_ = new sem_t;
	this->task_queue_full_  = new sem_t;
	this->finish_semaphore_list_  = new sem_t[8];
	sem_init(free_thread_queue_mutex_, 0, (unsigned int)1);
	sem_init(free_thread_queue_full_, 0, (unsigned int)0);
	sem_init(task_queue_mutex_, 0, (unsigned int)1);
	sem_init(task_queue_full_, 0, (unsigned int)0);
	for(int i = 0; i < 8; i++)
	{
		sem_init(finish_semaphore_list_ + i, 0, (unsigned int)0);
	}
	return;
}

void ThreadPool::semaphore_disable(void)
{
	sem_destroy(this->free_thread_queue_mutex_);
	sem_destroy(this->free_thread_queue_full_);
	sem_destroy(this->task_queue_mutex_);
	sem_destroy(this->task_queue_full_);
	for(int i = 0; i < 8; i++)
	{
		sem_destroy(this->finish_semaphore_list_ + i);
	}
	return;
}

void ThreadPool::new_one_thread(void)
{
	THREAD* tmp = new THREAD;
	this->thread_list_.push_back(tmp);
	sem_init((&tmp->execute_event), 0, (unsigned int)0);
	tmp->thread_number = this->total_thread_;
	tmp->pool_instance = this;
	pthread_create(&(tmp->thread_id), NULL, &thread_start_routine, (void*)tmp);
	return;
}

void ThreadPool::active_pool(void)
{
	/*for(int i = 0; i < (this->total_thread_ - 1); i++)
	{
		sem_destroy(&(this->thread_list_[i]->execute_event));
		sem_init(&(this->thread_list_[i]->execute_event), 0, (unsigned int)0);
	}*/
	this->task_list[1].border_pair_.first = 0;
	this->task_list[1].border_pair_.second = input_array.size() - 1;
	this->add_task(task_list + 1);
	this->push_idle_thread(this->thread_list_[this->total_thread_ - 1]);
	this->total_dispatch_times = 0;
	//semaphore_enable();
	return;
}

void ThreadPool::initial_all_task(void)
{//Initialize task_queue, OK
	for(int i = 1;i <= 16; i++)
	{
		task_list[i].task_number = i;
		i < 8 ? task_list[i].sort_func_ptr_ = &partition : task_list[i].sort_func_ptr_ = &bubble_sort;
	}
	return;
}

void ThreadPool::push_idle_thread(THREAD* idle_thread)
{
	sem_wait(this->free_thread_queue_mutex_);
		this->free_thread_queue_.push(idle_thread);
	sem_post(this->free_thread_queue_mutex_);
	sem_post(this->free_thread_queue_full_);
	return;
}

void ThreadPool::add_task(TASK* task_in)
{//Add a task to task_queue, OK
	sem_wait(this->task_queue_mutex_); //Entry-section
		this->task_queue.push(task_in); //Critical-section
	sem_post(this->task_queue_mutex_); //Exit-section

	//Signal empty semaphroe to indicate that queue is not empty
	sem_post(this->task_queue_full_);
	return;
}

void ThreadPool::thread_dispatcher()
{
	/******************************************************
	 * Extract a idle thread from this->free_thread_queue_*
	 * and post its semaphore to let it executing	      *
	 ******************************************************/
	while(this->total_dispatch_times < 15)
	{
		sem_wait(this->free_thread_queue_full_);
		sem_wait(this->free_thread_queue_mutex_);
			THREAD* wake_thread = this->free_thread_queue_.front();
			this->free_thread_queue_.pop();
		sem_post(this->free_thread_queue_mutex_);
		sem_post(&(wake_thread->execute_event));
		this->total_dispatch_times++;
		//cout << "Already dispatched " << total_dispatch_times << " jobs" << endl;
	}

	sem_wait(this->finish_semaphore_list_);
	sem_wait(this->finish_semaphore_list_ + 1);
	sem_wait(this->finish_semaphore_list_ + 2);
	sem_wait(this->finish_semaphore_list_ + 3);
	sem_wait(this->finish_semaphore_list_ + 4);
	sem_wait(this->finish_semaphore_list_ + 5);
	sem_wait(this->finish_semaphore_list_ + 6);
	sem_wait(this->finish_semaphore_list_ + 7);

	//this->semaphore_disable();
	return;
}

/*************************************************************************************
 * thread_start_routine is a static class member function                            *
 * all semaphores except thread's event semaphore is member variable of pool_instance*
 * pool_instance is created by the main thread(Original one).			     *
 *************************************************************************************/
void* ThreadPool::thread_start_routine(void* run_data)
{//Thread entry-function
	int task_number;
	THREAD* thread_data = (THREAD*)run_data;
	ThreadPool* pool_instance = thread_data->pool_instance;
	while(1)
	{
		sem_wait(&(thread_data->execute_event));
		//cout << "Thread " << thread_data->thread_number << " wait its semaphore successfully" << endl;
		sem_wait(pool_instance->task_queue_full_);
		sem_wait(pool_instance->task_queue_mutex_);//Entry-section
			TASK* task = pool_instance->task_queue.front();
			pool_instance->task_queue.pop();
		sem_post(pool_instance->task_queue_mutex_);//Out-section
		task_number = task->task_number;

		int pivot = (*(task->sort_func_ptr_))(task->border_pair_.first, task->border_pair_.second);
		//cout << "task " << task->task_number << " has been done." << endl;
		if(pivot >= 0)
		{
			(task + task_number)->border_pair_.first = task->border_pair_.first;
			(task + task_number)->border_pair_.second = pivot - 1;
			(task + task_number + 1)->border_pair_.first = pivot + 1;
			(task + task_number + 1)->border_pair_.second = task->border_pair_.second;
			pool_instance->add_task((task + task_number));
			pool_instance->add_task((task + task_number + 1));
			pool_instance->push_idle_thread(thread_data);
		}
		else
		{
			pool_instance->push_idle_thread(thread_data);
			sem_post(pool_instance->finish_semaphore_list_ + task_number - 8);
		}
	}

	//pthread_detach(pthread_self());

	return NULL;
}

ThreadPool::~ThreadPool()
{//Destructor
	delete task_queue_mutex_;
	delete task_queue_full_;
	delete free_thread_queue_full_;
	delete free_thread_queue_mutex_;
	delete[] finish_semaphore_list_;
	delete[] task_list;
	for(int i = 0; i < this->total_thread_; i++)
	{
		delete thread_list_[i];
	}
}

int main(int argc, char** argv)
{
	vector<string> output_file_name;
	output_file_name.push_back("output_1.txt");
	output_file_name.push_back("output_2.txt");
	output_file_name.push_back("output_3.txt");
	output_file_name.push_back("output_4.txt");
	output_file_name.push_back("output_5.txt");
	output_file_name.push_back("output_6.txt");
	output_file_name.push_back("output_7.txt");
	output_file_name.push_back("output_8.txt");
	
	int sec;
	int usec;
	struct timeval start, end;
	ThreadPool* TPool = new ThreadPool;
	for(int i = 1; i <= MAX_THREAD_NUMBER; i++)
	{
		file_input();
		//Create a ThreadPool instance
		TPool->set_thread_number(i);
		TPool->new_one_thread();
		TPool->active_pool();
		gettimeofday(&start, 0);
		TPool->thread_dispatcher();
		gettimeofday(&end, 0);
		sec = end.tv_sec - start.tv_sec;
		usec = end.tv_usec - start.tv_usec;
		cout << "Elapsed time with " << i << " thread in the pool :" << sec + usec/1000000.0 << "sec" << endl;
		write_to_file(output_file_name[i-1]);
		//cout << "Only " << i << " thread done" << endl;
	}
	//TPool->destroy_pool();
	delete TPool;
	return 0;
}

void write_to_file(string filename)
{
	fstream fout;
	fout.open(filename.c_str(), ios::out | ios::trunc);
	if(!fout)
	{
		cout << "File open failed" << endl;
		return;
	}
	for(int i = 0; i < num_eles; i++)
	{
		fout << input_array[i] << " ";
	}
	fout.close();
	return;
}
