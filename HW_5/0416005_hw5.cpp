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

class Page_node;
class Simulator;
class Doublely_Linklist;
typedef map<string, Page_node*>::iterator MAP_ITERATOR;

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

class Page_node
{
public:
	Page_node();
	Page_node(const string &self_address);
	void set_next_node(Page_node* next_node_ptr);
	void set_pre_node(Page_node* previous_node_ptr);
	Page_node* get_next_node(void) const;
	Page_node* get_previous_node(void) const;
	string get_address(void) const;
private:
	string self_address;
	Page_node* previous_node;
	Page_node* next_node;
};

Page_node::Page_node()
{
	previous_node = NULL;
	next_node = NULL;
}

Page_node::Page_node(const string &self_address)
{
	this->self_address = self_address;
	previous_node = NULL;
	next_node = NULL;
}

void Page_node::set_pre_node(Page_node* previous_node_ptr)
{
	this->previous_node = previous_node_ptr;
}

void Page_node::set_next_node(Page_node* next_node_ptr)
{
	this->next_node = next_node_ptr;
}

Page_node* Page_node::get_next_node(void) const
{
	return this->next_node;
}

Page_node* Page_node::get_previous_node(void) const
{
	return this->previous_node;
}

string Page_node::get_address(void) const
{
	return this->self_address;
}

class Doublely_Linklist
{
public:
	Doublely_Linklist();
	Doublely_Linklist(unsigned int max_node_number);
	~Doublely_Linklist();
	bool insert_node_to_tail(Page_node* node_ptr);
	bool delete_node_from_head();
	bool extract_node_from_list(Page_node* extract_node_ptr);
	Page_node* get_head(void) const;
	Page_node* get_tail(void) const;
private:
	const unsigned int max_node_number_;
	unsigned int current_node_count_;
	Page_node* head_;
	Page_node* tail_;
};

Doublely_Linklist::Doublely_Linklist() : max_node_number_(0)
{
	this->head_ = new Page_node;
	this->tail_ = new Page_node;
	this->head_->set_pre_node(NULL);
	this->head_->set_next_node(this->tail_);
	this->tail_->set_pre_node(this->head_);
	this->tail_->set_next_node(NULL);
	this->current_node_count_ = 0;
};

Doublely_Linklist::Doublely_Linklist(const unsigned int max_node_number) : max_node_number_(max_node_number)
{
	//cout << "max_node_number : " << this->max_node_number_ << endl;
	this->head_ = new Page_node;
	//cout << "linklist head : " << this->head_ << endl;
	this->tail_ = new Page_node;
	//cout << "linklist tail : " << this->tail_ << endl;
	this->head_->set_pre_node(NULL);
	this->head_->set_next_node(this->tail_);
	this->tail_->set_pre_node(this->head_);
	this->tail_->set_next_node(NULL);
	this->current_node_count_ = 0;
}

Page_node* Doublely_Linklist::get_head() const
{
	return this->head_;
}

Page_node* Doublely_Linklist::get_tail() const
{
	return this->tail_;
}

bool Doublely_Linklist::insert_node_to_tail(Page_node* node_ptr)
{
	if(this->current_node_count_ >= this->max_node_number_)
	{
		return false;
	}
	node_ptr->set_next_node(this->tail_);
	node_ptr->set_pre_node(this->tail_->get_previous_node());
	this->tail_->get_previous_node()->set_next_node(node_ptr);
	this->tail_->set_pre_node(node_ptr);
	this->current_node_count_++;
	return true;
}

bool Doublely_Linklist::delete_node_from_head()
{
	if(this->current_node_count_ <= 0)
	{
		return false;
	}
	Page_node* delete_node = this->head_->get_next_node();
	this->head_->set_next_node(delete_node->get_next_node());
	delete_node->get_next_node()->set_pre_node(this->head_);
	delete delete_node;
	this->current_node_count_--;
	return true;
}

bool Doublely_Linklist::extract_node_from_list(Page_node* extract_node_ptr)
{
	if(this->current_node_count_ == 0)
	{
		return false;
	}
	Page_node* prev = extract_node_ptr->get_previous_node();
	Page_node* next = extract_node_ptr->get_next_node();
	prev->set_next_node(next);
	next->set_pre_node(prev);
	this->current_node_count_--;
	return true;
}

Doublely_Linklist::~Doublely_Linklist()
{
	Page_node* temp = this->head_->get_next_node();
	while(temp != this->tail_)
	{
		Page_node* next = temp->get_next_node();
		delete temp;
		temp = next;
	}
	delete this->head_;
	delete this->tail_;
}

class Simulator
{
public:
	Simulator();
	Simulator(unsigned int algo_used, unsigned int total_frame_size);
	~Simulator();
	unsigned int get_hit_times(void) const;
	unsigned int get_miss_times(void) const;
	double get_miss_rate(void) const;
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
	Doublely_Linklist* link_list;
	map<string, Page_node*> page_cache_search_list_;
private:
	MAP_ITERATOR search(const string &search_address);
	void lru_routine(const string &fetch_address, MAP_ITERATOR &search_result_pos);
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
	this->link_list = new Doublely_Linklist(total_frame_size);
}

Simulator::~Simulator()
{
	delete this->link_list;
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

double Simulator::get_miss_rate(void) const
{
	return this->miss_rate_;
}

void Simulator::fifo_routine(const string &fetch_address, MAP_ITERATOR &search_result_pos)
{
	//cout << "fifo routine" << endl;
	if(search_result_pos == this->page_cache_search_list_.end())
	{//No-hit
		Page_node* node = new Page_node(fetch_address);
		//cout << "new node address : " << node << endl;
		if(this->frame_count_ == this->total_frame_size_)
		{//Full
			//string delete_address = page_cache_list_.front();//Find the first in the deque
			//this->page_cache_list_.pop_front();//Pop it from deque
			string delete_address = this->link_list->get_head()->get_next_node()->get_address();
			MAP_ITERATOR delete_position = this->page_cache_search_list_.find(delete_address);//Allocate where it is in the MAP STL
			this->page_cache_search_list_.erase(delete_position);//Delete it from the MAP STL
			this->frame_count_--;
			this->link_list->delete_node_from_head();
		}
		//this->page_cache_list_.push_back(fetch_address);
		this->link_list->insert_node_to_tail(node);
		this->page_cache_search_list_.insert(pair<string, Page_node*>(fetch_address, node));
		this->frame_count_++;
	}
	return;
}

void Simulator::lru_routine(const string &fetch_address, MAP_ITERATOR &search_result_pos)
{
	if(search_result_pos == this->page_cache_search_list_.end())
	{//No-hit
		Page_node* node = new Page_node(fetch_address);
		if(this->frame_count_ == this->total_frame_size_)
		{//Full
			string delete_address = this->link_list->get_head()->get_next_node()->get_address();
			MAP_ITERATOR delete_position = this->page_cache_search_list_.find(delete_address);
			this->page_cache_search_list_.erase(delete_position);
			this->frame_count_--;
			this->link_list->delete_node_from_head();
		}
		this->link_list->insert_node_to_tail(node);
		this->page_cache_search_list_.insert(pair<string, Page_node*>(fetch_address, node));
		this->frame_count_++;
	}
	else
	{//Hit
		Page_node* extract_node_ptr = search_result_pos->second;
		this->link_list->extract_node_from_list(extract_node_ptr);
		this->link_list->insert_node_to_tail(extract_node_ptr);
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
		//this->lru_routine(fetch_address, search_result_pos);
	}
	//cout << "Fetch done" << endl;
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
		Simulator* simulator_instance = new Simulator(FIFO, size);
		for(unsigned int i = 0; i < data_size; i++)
		{
			simulator_instance->fetch(input_array[i]);
		}
		cout << size << "\t" << simulator_instance->get_miss_times() << "\t" << simulator_instance->get_hit_times() << "\t" << simulator_instance->get_miss_rate() << endl;
		delete simulator_instance;
	}

	/*LRU part*/
	cout << "LRU--" << endl;
	cout << "size\tmiss\thit\tpage fault ratio\t" << endl;
	for(int size = 64; size < 1024; size*=2)
	{
		Simulator* simulator_instance = new Simulator(FIFO, size);
		for(unsigned int i = 0; i < data_size; i++)
		{
			simulator_instance->fetch(input_array[i]);
		}
		cout << size << "\t" << simulator_instance->get_miss_times() << "\t" << simulator_instance->get_hit_times() << "\t" << simulator_instance->get_miss_rate() << endl;
	}
	return 0;
}
