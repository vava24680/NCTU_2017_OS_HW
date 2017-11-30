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
	int pivot;
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
	void thread_dispatcher();
	void deactive_pool(void);
private:
	int total_dispatch_times_;
	int total_append_job_times_;
	int total_thread_;
	bool ready_destroy_;
	TASK finish_task_;
	queue<TASK*> task_queue_;
	queue<THREAD*> free_thread_queue_;
	vector<TASK> all_task_list_;
	vector<THREAD*> all_thread_list_;
private:
	sem_t* task_queue_mutex_;		//Same function as mutex semaphore in P-C problem.
	sem_t* task_queue_full_;		//Same function as full semaphore in P-C problem.
	sem_t* free_thread_queue_full_;	//Same function as full semaphore in P-C problem.
	sem_t* free_thread_queue_mutex_;//Same function as mutex semaphore in P-C problem.
	sem_t* bsort_done_sem_list_;	//Use to notify main thread that bubble sort are all done.
	sem_t* pivot_done_semaphore_;	//Use to notify main thread that partition is done
	sem_t* finish_task_mutex_;		/*Use to protect the finish_task_ this structure
						 for tranfer previous task information for apending new tasks*/
	sem_t* detach_done_;
private:
	void semaphore_enable(void);
	void create_all_task_container(void);
	void add_task(TASK* task_in);
	void push_idle_thread(THREAD* free_thread);
	void wake_one_thread(void);
	static void* thread_start_routine(void* run_data);
};

ThreadPool::ThreadPool()
{//Default constructor
	ready_destroy_ = false;
	total_dispatch_times_ = 0;
	total_append_job_times_ = 0;
	this->semaphore_enable();
	this->create_all_task_container();
}
ThreadPool::ThreadPool(int total_thread)
{//Constructor with total_thread number parameter
	ready_destroy_ = false;
	this->total_thread_ = total_thread;
	total_dispatch_times_ = 0;
	total_append_job_times_ = 0;
	this->semaphore_enable();
	this->create_all_task_container();
}

void ThreadPool::set_thread_number(int thread_number)
{
	this->total_thread_ = thread_number;
}

void ThreadPool::semaphore_enable(void)
{//Enable all the semaphores will be used in this thread pool
	this->free_thread_queue_mutex_ = new sem_t;
	this->free_thread_queue_full_  = new sem_t;
	this->task_queue_mutex_ = new sem_t;
	this->task_queue_full_  = new sem_t;
	this->bsort_done_sem_list_  = new sem_t[8];
	this->pivot_done_semaphore_ = new sem_t;
	this->finish_task_mutex_ = new sem_t;
	this->detach_done_ = new sem_t;
	sem_init(free_thread_queue_mutex_, 0, (unsigned int)1);
	sem_init(free_thread_queue_full_, 0, (unsigned int)0);
	sem_init(task_queue_mutex_, 0, (unsigned int)1);
	sem_init(task_queue_full_, 0, (unsigned int)0);
	for(int i = 0; i < 8; i++)
	{
		sem_init(bsort_done_sem_list_ + i, 0, (unsigned int)0);
	}
	sem_init(pivot_done_semaphore_, 0, (unsigned int)0);
	sem_init(finish_task_mutex_, 0, (unsigned int)1);
	sem_init(detach_done_, 0, (unsigned int)0);
	return;
}

void ThreadPool::create_all_task_container(void)
{//Create all the task container will be used in this thread pool
	all_task_list_.resize(16);
	for(int i = 1;i < 16; i++)
	{
		all_task_list_[i].task_number = i;
		all_task_list_[i].sort_func_ptr_ = ( i < 8 ?  &partition : &bubble_sort);
	}
	return;
}

void ThreadPool::push_idle_thread(THREAD* idle_thread)
{//Push a idle thread into free_thread_queue_
	sem_wait(this->free_thread_queue_mutex_);
		this->free_thread_queue_.push(idle_thread);
	sem_post(this->free_thread_queue_mutex_);
	sem_post(this->free_thread_queue_full_);
	return;
}

void ThreadPool::wake_one_thread(void)
{//Wake one thread from free_thread_queue_
	sem_wait(this->free_thread_queue_full_);
	sem_wait(this->free_thread_queue_mutex_);
			THREAD* wake_thread = this->free_thread_queue_.front();
			this->free_thread_queue_.pop();
	sem_post(this->free_thread_queue_mutex_);
	sem_post(&(wake_thread->execute_event));
	this->total_dispatch_times_++;
	return;
}

void ThreadPool::new_one_thread(void)
{//Create a new thread and also push it into the free_thread_queue_
	THREAD* tmp = new THREAD;
	sem_init(&(tmp->execute_event), 0, (unsigned int)0);
	tmp->thread_number = this->total_thread_;
	tmp->pool_instance = this;
	this->all_thread_list_.push_back(tmp);
	this->push_idle_thread(tmp);
	pthread_create(&(tmp->thread_id), NULL, &thread_start_routine, (void*)tmp);
	return;
}

void ThreadPool::active_pool(void)
{//Active the pool and push the first task into task_queue_
	this->all_task_list_[1].border_pair_.first = 0;
	this->all_task_list_[1].border_pair_.second = input_array.size() - 1;
	this->add_task(&all_task_list_[1]);
	this->total_dispatch_times_ = 0;
	this->total_append_job_times_ = 0;
	return;
}

void ThreadPool::add_task(TASK* task_in)
{//Add a task to task_queue_, OK
	sem_wait(this->task_queue_mutex_); //Entry-section
		this->task_queue_.push(task_in); //Critical-section
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
	int tmp_task_number;
	//cout << free_thread_queue_.size()  << " in the free_thread_queue_" << endl;
	this->wake_one_thread();
	while(this->total_append_job_times_ < 7)
	{
		sem_wait(this->pivot_done_semaphore_);
		sem_wait(this->finish_task_mutex_);
			TASK tmp = finish_task_;
		sem_post(this->finish_task_mutex_);
		tmp_task_number = tmp.task_number;
		all_task_list_[2*tmp_task_number].border_pair_.first = tmp.border_pair_.first;
		all_task_list_[2*tmp_task_number].border_pair_.second = tmp.pivot - 1;
		all_task_list_[2*tmp_task_number + 1].border_pair_.first = tmp.pivot + 1;
		all_task_list_[2*tmp_task_number + 1].border_pair_.second = tmp.border_pair_.second;
		/*Append two task in the job queue*/
		this->add_task(&all_task_list_[2*tmp_task_number]);
		this->add_task(&all_task_list_[2*tmp_task_number + 1]);
		this->total_append_job_times_++;
		/*Wake up one thread*/
		this->wake_one_thread();
	}
	//cout << "Main thread outside append block" << endl;
	while(this->total_dispatch_times_ < 15)
	{
		this->wake_one_thread();
	}

	sem_wait(this->bsort_done_sem_list_);
	sem_wait(this->bsort_done_sem_list_ + 1);
	sem_wait(this->bsort_done_sem_list_ + 2);
	sem_wait(this->bsort_done_sem_list_ + 3);
	sem_wait(this->bsort_done_sem_list_ + 4);
	sem_wait(this->bsort_done_sem_list_ + 5);
	sem_wait(this->bsort_done_sem_list_ + 6);
	sem_wait(this->bsort_done_sem_list_ + 7);
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
		if(pool_instance->ready_destroy_)
		{
			//cout << "here" << endl;
			pthread_detach(pthread_self());
			break;
		}
		/*Get one task from task_queue_*/
		sem_wait(pool_instance->task_queue_full_);
		sem_wait(pool_instance->task_queue_mutex_);//Entry-section
			TASK* task = pool_instance->task_queue_.front();
			pool_instance->task_queue_.pop();
		sem_post(pool_instance->task_queue_mutex_);//Out-section
		task_number = task->task_number;

		/*Use function pointer to direct which function it should go*/
		int pivot = (*(task->sort_func_ptr_))(task->border_pair_.first, task->border_pair_.second);

		/*According to the return value to do different things*/
		if(pivot >= 0)
		{
			//Signal main thread that array has been divided into two arrays.
			sem_wait(pool_instance->finish_task_mutex_);
				pool_instance->finish_task_.pivot = pivot;
				pool_instance->finish_task_.border_pair_.first = task->border_pair_.first;
				pool_instance->finish_task_.border_pair_.second = task->border_pair_.second;
				pool_instance->finish_task_.task_number = task->task_number;
				pool_instance->push_idle_thread(thread_data);
			sem_post(pool_instance->finish_task_mutex_);
			sem_post(pool_instance->pivot_done_semaphore_);
			/*Push itself intp idle_thread queue*/
		}
		else
		{
			pool_instance->push_idle_thread(thread_data);
			sem_post(pool_instance->bsort_done_sem_list_ + task_number - 8);
		}
	}
	sem_post(pool_instance->detach_done_);
	//return NULL;
	pthread_exit(NULL);
}

void ThreadPool::deactive_pool(void)
{
	this->ready_destroy_ = true;
	//cout << "all_thread_list_ size : " << this->all_thread_list_.size() << endl;
	//cout << "free_thread_queue_ size : " << this->free_thread_queue_.size() << endl;
	unsigned int free_thread_queue_size = this->free_thread_queue_.size();
	for(unsigned int i = 0; i < free_thread_queue_size;i++)
	{
		sem_post(&(free_thread_queue_.front()->execute_event));
		free_thread_queue_.pop();
		sem_wait(this->detach_done_);
	}
}

ThreadPool::~ThreadPool()
{//Destructor
	delete this->task_queue_mutex_;
	delete this->task_queue_full_;
	delete this->free_thread_queue_full_;
	delete this->free_thread_queue_mutex_;
	delete[] this->bsort_done_sem_list_;
	delete this->pivot_done_semaphore_;
	delete this->finish_task_mutex_;
	delete this->detach_done_;
	for(unsigned int i = 0; i < this->all_thread_list_.size(); i++)
	{
		delete this->all_thread_list_[i];
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
	ThreadPool* TPool = new ThreadPool(0);
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
		cout << "Elapsed time with " << i << " thread(s) in the pool :" << sec*1000 + usec/1000.0 << " msec" << endl;
		write_to_file(output_file_name[i-1]);
	}
	TPool->deactive_pool();
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
