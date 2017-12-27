#include <iostream>
#include <map>
#include <utility>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <limits>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
using namespace std;
#define B2MB(bytes) ((bytes)/((1024.0)*(1024.0)))

map<string, int> OptionMap;
vector<bool> option_match_list;
pair<double, double> SizeRange(0, numeric_limits<unsigned int>::max());
string FileName;
unsigned int Inode;

bool Inode_check(bool previous_check_result,const unsigned int source_inode)
{
	if(!previous_check_result)
		return false;
	if(Inode == source_inode || (!option_match_list[0]))
		return true;
	else
		return false;
}
bool File_Name_Check(bool previous_check_result,const string source_file_name)
{
	if(!previous_check_result)
		return false;
	if(FileName.compare(source_file_name)==0 || (!option_match_list[1]))
		return true;
	else
		return false;
}
bool Min_Size_Check(bool previous_check_result, const unsigned int source_size)
{
	if(!previous_check_result)
		return false;
	if(SizeRange.first <= B2MB(source_size) || (!option_match_list[2]))
		return true;
	else
		return false;
	return false;
}
bool Max_Size_Check(bool previous_check_result, const unsigned int source_size)
{
	if(!previous_check_result)
		return false;
	if(SizeRange.second >= B2MB(source_size) || (!option_match_list[3]))
		return true;
	else
		return false;
}
void dfs_search(const string path_name)
{
	//cout << "current : " << path_name << endl;
	bool previous_check_result = true;
	vector<string> deep_path;
	DIR* dir_stream = NULL;
	dir_stream = opendir(path_name.c_str());
	if(dir_stream == NULL)
	{
		cout << "Specified path is not exist" << endl;
		return;
	}
	struct dirent* entry = readdir(dir_stream);
	while(entry!=NULL)
	{
		struct stat file_detail;
		previous_check_result = true;
		if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			if(path_name[path_name.length() - 1] != '/')
				lstat(string(path_name + "/" + entry->d_name).c_str(), &file_detail);
			else
				lstat(string(path_name + entry->d_name).c_str(), &file_detail);
			//cout << "file : " << entry->d_name << "," << file_detail.st_mode << endl;
			if(S_ISDIR(file_detail.st_mode))
			{
				if(path_name[path_name.length() - 1] != '/')
					deep_path.push_back(path_name + '/' + entry->d_name);
				else
					deep_path.push_back(path_name + entry->d_name);
			}
			if(S_ISDIR(file_detail.st_mode) || S_ISREG(file_detail.st_mode))
			{
				previous_check_result &= Inode_check(previous_check_result, entry->d_ino);
				previous_check_result &= File_Name_Check(previous_check_result, string(entry->d_name));
				previous_check_result &= Min_Size_Check(previous_check_result, file_detail.st_size);
				previous_check_result &= Max_Size_Check(previous_check_result, file_detail.st_size);
				if(previous_check_result)
				{
					cout.setf(iostream::left);
					string temp(path_name);
					if(temp[temp.length() - 1]!='/')
						temp+='/';
					temp+=entry->d_name;
					cout << setw(40) << temp;
					cout << setw(7) << entry->d_ino;
					cout.setf(iostream::fixed);
					cout.precision(9);
					cout << setw(10) << B2MB(file_detail.st_size) << " MB" << endl;
				}
			}
		}
		entry = readdir(dir_stream);
	}
	for(unsigned int i = 0; i < deep_path.size(); i++)
		dfs_search(deep_path[i]);
	closedir(dir_stream);
	return;
}

bool Option_test(const vector<string> argv_list)
{
	bool result = true;
	for(unsigned int i = 1;i < argv_list.size(); i++)
	{
		if(OptionMap.find(argv_list[i]) == OptionMap.end())
			result = false;
		else
		{	
			switch(OptionMap[argv_list[i]])
			{
				case 0:/*inode check case*/
					option_match_list[0] = true;
					Inode = atoi(argv_list[i + 1].c_str());
					i++;
					break;
				case 1:/*file name check case*/
					FileName = string(argv_list[i + 1]);
					i++;
					option_match_list[1] = true;
					break;
				case 2:/*minimum size check*/
					//Number check
					SizeRange.first = strtod(argv_list[i + 1].c_str(), NULL);
					option_match_list[2] = true;
					i++;
					break;
				case 3:/*maximum size check*/
					//Number check
					SizeRange.second = strtod(argv_list[i + 1].c_str(), NULL);
					option_match_list[3] = true;
					i++;
					break;
				default:
					break;
			}
		}
	}
	return result;
}

int main(int argc, char* argv[])
{
	vector<string> argv_list;
	OptionMap.insert(pair<string, int>(string("-inode"), 0));
	OptionMap.insert(pair<string, int>(string("-name"), 1));
	OptionMap.insert(pair<string, int>(string("-size_min"), 2));
	OptionMap.insert(pair<string, int>(string("-size_max"), 3));
	option_match_list.resize(4, false);
	if(argc == 1)
		cout << "Please re-execute and specify path and options" << endl;
	else if(argc == 2)
		cout << "Please re-execute abd specify search options" << endl;
	else
	{
		for(int i = 1;i < argc;i++)
		{
			argv_list.push_back(string(argv[i]));
		}
		if(Option_test(argv_list))
		{
			//Go to the Search Function and Do DFS-style search
			dfs_search(argv_list[0]);
		}
		else
			cout << "Please re-execute and specify the correct options" << endl;
	}
	return 0;
}
