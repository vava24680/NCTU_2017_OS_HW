#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<string>
using namespace std;

#define INPUTFILE "input.txt"
#define OUTPUTFILE "correct_sort.txt"

int main(void)
{
	string input;
	int num_eles;
	vector<int> input_series;
	fstream fin, fout;
	fin.open(INPUTFILE, ios::in);
	if(!fin)
	{
		cout << "open file failed" << endl;
		return 0;
	}
	fin >> num_eles;
	input_series.resize(num_eles);
	for(int i=0; i<num_eles; i++)
	{
		fin >> input_series[i];
	}
	fin.close();
	sort(input_series.begin(), input_series.end());
	fout.open(OUTPUTFILE, ios::out | ios::trunc);
	for(int i=0; i<num_eles; i++)
	{
		fout << input_series[i] << " ";
	}
	fout.close();
	return 0;
}
