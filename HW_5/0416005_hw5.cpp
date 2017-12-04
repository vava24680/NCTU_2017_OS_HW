#include <iostream>
#include <deque>
#include <map>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <set>
#include <string>
#include <utility>
using namespace std;
#define FILENAME "trace.txt"
#define FIFO 1
#define LRU 2

typedef map<string,int>::iterator MAP_ITERATOR;

class Simulator;
void file_input(vector<string> &input_array)
{
	fstream fin;
	fin.open(FILENAME);
	string temp;
	string address;
	while(1)
	{
		fin >> temp;
		if(fin.eof())
		{
			break;
		}
		fin >> temp;
		address = temp.substr(0, 5);
		input_array.push_back(address);
	}
	fin.close();
	return;
}

class Simulator
{
public:
	Simulator();
	Simulator(unsigned int algo_used, unsigned int total_frame_size);
	unsigned int get_hit_times(void) const;
	unsigned int get_miss_times(void) const;
	unsigned int get_miss_rate(void) const;
	void fetch(const string &fetch_address);
private:
	unsigned int total_frame_size_;
	unsigned int algo_used_;
	unsigned int frame_count_;
	unsigned int hit_times_;
	unsigned int miss_times_;
	double miss_rate_;
	/*
	 *For FIFO, the mapped value in page_cache_search_list_ is not important since the position each page in the FIFO queue is not concerned
	 , only care about the head of FIFO queue and the last.
	 * */
	deque<string> page_cache_list_;
	map<string, int> page_cache_search_list_;
private:
	MAP_ITERATOR search(const string &search_address);
	void lru_routine();
	void fifo_routine(const string &fetch_address, MAP_ITERATOR &search_result_pos);
};

Simulator::Simulator(){};

Simulator::Simulator(unsigned int algo_used, unsigned int total_frame_size)
{
	this->hit_times_ = 0;
	this->miss_times_ = 0;
	this->miss_rate_ = 0;
	this->algo_used_ = algo_used;
	this->total_frame_size_ = total_frame_size;
	this->frame_count_ = 0;
}

MAP_ITERATOR Simulator::search(const string &search_address)
{
	return page_cache_search_list_.find(search_address);
}

unsigned int Simulator::get_hit_times(void) const
{
	return this->hit_times_;
}

unsigned int Simulator::get_miss_times(void) const
{
	return this->miss_times_;
}

unsigned int Simulator::get_miss_rate(void) const
{
	return this->miss_rate_;
}

void Simulator::fifo_routine(const string &fetch_address, MAP_ITERATOR &search_result_pos)
{
	if(search_result_pos == this->page_cache_search_list_.end())
	{//No-hit
		if(this->frame_count_ == this->total_frame_size_)
		{
			string delete_address = page_cache_list_.front();//Find the first in the deque
			this->page_cache_list_.pop_front();//Pop it from deque
			MAP_ITERATOR delete_position = this->page_cache_search_list_.find(delete_address);//Allocate where it is in the MAP STL
			this->page_cache_search_list_.erase(delete_position);//Delete it from the MAP STL
			this->frame_count_--;
		}
		this->page_cache_list_.push_back(fetch_address);
		this->page_cache_search_list_.insert(pair<string, int>(fetch_address,this->frame_count_));
		this->frame_count_++;
	}
	return;
}


void Simulator::fetch(const string &fetch_address)
{
	MAP_ITERATOR search_result_pos = this->search(fetch_address);
	search_result_pos == page_cache_search_list_.end() ? this->miss_times_++ : this->hit_times_++;
	this->miss_rate_ = (double)this->miss_times_ / (this->miss_times_ + this->hit_times_);
	if(this->algo_used_ == FIFO)
	{
		this->fifo_routine(fetch_address, search_result_pos);
	}
	else
	{
	
	}
	return;
}

int main(void)
{
	vector<string> input_array;
	file_input(input_array);
	unsigned int data_size = input_array.size();
	/*FIFO part*/
	cout << "FIFO--" << endl;
	cout << "size\tmiss\thit\tpage fault ratio\t" << endl;
	for(int size = 64; size < 1024; size*=2)
	{
		Simulator simulator_instance(FIFO, size);
		for(unsigned int i = 0; i < data_size; i++)
		{
			simulator_instance.fetch(input_array[i]);
		}
		cout << size << "\t" << simulator_instance.get_miss_times() << "\t" << simulator_instance.get_hit_times() << "\t" << simulator_instance.get_miss_rate() << endl;
	}

	/*LRU part*/
	cout << "LRU--" << endl;
	cout << "size\tmiss\thit\tpage fault ratio\t" << endl;
	for(int size = 64; size < 1024; size*=2)
	{
		Simulator simulator_instance(LRU, size);
		/*for(unsigned int i = 0; i < data_size; i++)
		{
			simulator_instance.fetch(input_array[i]);
		}*/
		cout << size << "\t" << simulator_instance.get_miss_times() << "\t" << simulator_instance.get_hit_times() << "\t" << simulator_instance.get_miss_rate() << endl;
	}
	return 0;
}
