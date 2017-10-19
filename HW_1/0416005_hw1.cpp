#include <iostream>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;

char** expand_CommandList(unsigned int command_list_length,char** original_address)
{
	char** new_address = new char*[2*command_list_length];
	for(int i=0;i<command_list_length;i++)
	{
		new_address[i]=original_address[i];
	}
	for(int i=command_list_length;i<2*command_list_length;i++)
	{
		new_address[i]=NULL;
	}
	delete[] original_address;
	return new_address;
}

int ExtractToken(char** &command_list_1, char** &command_list_2, unsigned int &command_list_1_length, unsigned int &command_list_2_length, bool &wait_c, int &pipe_red)
{
	string command;
	stringstream s_cut;
	string temp_token;
	unsigned int token_len;

	command_list_1 = new char*[command_list_1_length];//Initial, each list has 10 spaces
	command_list_2 = new char*[command_list_2_length];//Initial, each list has 10 spaces
	char** list_p_use = command_list_1;
	unsigned int list_length_use = command_list_1_length;
	unsigned int count_use = 0;

	for(int i=0;i<10;i++)
	{
		command_list_1[i]=NULL;
		command_list_2[i]=NULL;
	}
	getline(cin,command);
	if(command.length()==0)
	{
		return 0;
	}
	s_cut << command;
	while(getline(s_cut,temp_token,' '))
	{
		if(temp_token=="&")
		{
			wait_c = false;
			continue;
		}
		if(temp_token=="|" || temp_token==">")
		{
			temp_token=="|" ? pipe_red=2 : pipe_red=1;
			if(count_use>=list_length_use)
			{
				list_p_use = expand_CommandList(list_length_use,list_p_use);
				list_length_use*=2;
			}
			list_p_use[count_use] = NULL;
			command_list_1_length = list_length_use;
			command_list_1 = list_p_use;

			list_p_use = command_list_2;
			count_use = 0;
			list_length_use = command_list_2_length;
			continue;
		}
		token_len = temp_token.length();
		if(count_use>=list_length_use)
		{
			list_p_use = expand_CommandList(list_length_use,list_p_use);
			list_length_use*=2;
		}
		list_p_use[count_use] = new char[token_len+1];
		strcpy(list_p_use[count_use],temp_token.data());
		count_use++;
	}
	if(count_use>=list_length_use)
	{
		list_p_use = expand_CommandList(list_length_use,list_p_use);
		list_length_use*=2;
	}
	list_p_use[count_use] = NULL;
	if(pipe_red)
	{
		command_list_2 = list_p_use;
		command_list_2_length = list_length_use;
	}
	else
	{
		command_list_1 = list_p_use;
		command_list_1_length = list_length_use;
	}
	/*for(int i=0;i<counter;i++)
	{
		printf("%d-th token is %s\n",i+1,command_list_1[i]);
	}*/
	//cout << "Command_list_1_len : " << command_list_1_length << endl;
	//cout << "Command_list_2_len : " << command_list_2_length << endl;
	return command.length();
}

void fork_process(char** command_list, int* file_descritor, bool wait_c, int Stream_Mode)
{
	pid_t child_pid;
	pid_t grandson_pid;
	child_pid=fork();
	if(child_pid<0)
	{
		fprintf(stderr,"Fork failed\n");
	}
	else if(child_pid==0)
	{
		if(Stream_Mode>=2)
		{
			(Stream_Mode==2) ? dup2(file_descritor[1],STDOUT_FILENO) : dup2(file_descritor[0],STDIN_FILENO);
			(Stream_Mode==2) ? close(file_descritor[0]) : close(file_descritor[1]);
		}
		else
		{
			(Stream_Mode==1) ? dup2(file_descritor[0],STDOUT_FILENO) : Stream_Mode=Stream_Mode ;
			//(Stream_Mode==1) ? close(file_descritor[0]) : Stream_Mode=Stream_Mode ;
		}
		if(wait_c)
		{
			if(execvp(command_list[0],command_list)<0)
			{
				cout << "Child process run error" << endl;
				exit(0);
			}
		}
		else
		{
			grandson_pid=fork();
			if(grandson_pid<0)
			{
				fprintf(stderr, "Fork failed\n" );
			}
			else if(grandson_pid==0)
			{
				//cout << "Grandson Process ID is " << getpid() << endl;
				if(execvp(command_list[0],command_list)<0)
				{
					cout << "Child process run error" << endl;
					exit(0);
				}
			}
			else
			{
				exit(0);
			}
		}
	}
}

int main(int argc, char const *argv[])
{
	int command_length=0;
	const char* EXIT="exit";
	while(1)
	{
		bool wait_c=true;
		int pipe_red = 0;//0-> no pipe and nor IO redirection, 1-> I/O redirection, 2-> Pipe
		char** command_list_1=NULL;
		char** command_list_2=NULL;
		unsigned int command_list_1_length=10;
		unsigned int command_list_2_length=10;
		unsigned int n_command=0;//also ref to how many tokens
		cout << ">";

		command_length = ExtractToken(command_list_1,command_list_2,command_list_1_length,command_list_2_length,wait_c,pipe_red);
		if(command_length == 0)
			continue;
		n_command = (pipe_red==2) ? 2 : 1 ;
		if(!strcmp(command_list_1[0],EXIT))
		{
			for(int i=0;i<command_list_1_length;i++)
			{
				if(command_list_1[i]!=NULL)
					delete[] command_list_1[i];
			}
			for(int i=0;i<command_list_2_length;i++)
			{
				if(command_list_2[i]!=NULL)
					delete[] command_list_2[i];
			}
			if(command_list_1 != NULL)
				delete[] command_list_1;
			if(command_list_2 != NULL)
				delete[] command_list_2;
			break;
		}
		if(!pipe_red)
		{
			fork_process(command_list_1,NULL,wait_c,0);
			//cout << "Child process complete" << endl;
		}
		else
		{
			int *file_descritor = new int[2];
			if(pipe_red==1)
			{
				file_descritor[0] = open(command_list_2[0], O_WRONLY | O_CREAT | O_TRUNC);
				if(file_descritor[0]<0)
				{
					cout << "File open error" << endl;
					continue;
				}
				//cout << "file descritor is " << file_descritor <<  endl;
				fork_process(command_list_1,file_descritor,wait_c,1);
			}
			else
			{
				if(pipe(file_descritor)<0)
				{
					cout << "Pipe creation failed" << endl;
					continue;
				}
				fork_process(command_list_1,file_descritor,wait_c,2);
				fork_process(command_list_2,file_descritor,wait_c,3);
				close(file_descritor[0]);
				close(file_descritor[1]);
			}
			delete[] file_descritor;
		}
		for(int i=0;i<n_command;i++)
		{
			int reaped_pid;
			//wait(NULL);
			do
			{
				reaped_pid = waitpid(-1,NULL,WNOHANG);
			}while(reaped_pid<=0);
		}
		/*Put this segment at the end of while segment*/
		for(int i=0;i<command_list_1_length;i++)
		{
			if(command_list_1[i]!=0)
				delete[] command_list_1[i];
		}
		for(int i=0;i<command_list_2_length;i++)
		{
			if(command_list_2[i]!=0)
				delete[] command_list_2[i];
		}
		if(command_list_1!=0)
			delete[] command_list_1;
		if(command_list_2!=0)
			delete[] command_list_2;
		/*--------------------------------------------*/
	}
	return 0;
}
