#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <unistd.h>
#include <cstdio>
#include <cstring>
using namespace std;

char** expand_CommandList(unsigned int command_list_length,char** original_address)
{
	char** new_address = new char*[2*command_list_length];
	for(int i=0;i<command_list_length;i++)
	{
		new_address[i]=original_address[i];
	}
	delete [] original_address;
	return new_address;
}

char** ExtractToken(string command,unsigned int &counter,unsigned int &command_list_length)
{
	char** command_list = new char*[command_list_length];
	for(int i=0;i<10;i++)
	{
		command_list[i]=NULL;
	}
	stringstream s_cut;
	string temp_token;
	unsigned int token_len;
	list<string> c_tokens;
	s_cut << command;
	while(getline(s_cut,temp_token,' '))
	{
		//printf("Token is %s\n",temp_token.c_str());
		token_len = temp_token.length();
		if(counter>=command_list_length)
		{

			command_list=expand_CommandList(command_list_length,command_list);
			command_list_length*=2;
		}
		command_list[counter] = new char[token_len+1];
		strcpy(command_list[counter],temp_token.data());
		counter++;
	}
	/*for(int i=0;i<counter;i++)
	{
		printf("%d-th token is %s\n",i+1,command_list[i]);
	}*/
	return command_list;
}

int main(int argc, char const *argv[])
{
	string command;
	while(1)
	{
		char** command_list=0;
		unsigned int command_list_length=10;
		unsigned int counter=0;//also ref to how many tokens
		cout << ">";
		getline(cin,command);
		//cout << "Your command is " << command << endl;
		/*Put this segment at the end of while segment*/
		command_list = ExtractToken(command,counter,command_list_length);
		for(int i=0;i<counter;i++)
		{
			if(command_list[i]!=0)
				delete[] command_list[i];
		}
		if(command_list!=0)
			delete[] command_list;
		/*--------------------------------------------*/
	}
	return 0;
}