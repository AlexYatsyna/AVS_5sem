
#include <iostream>
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <cstdint>
#include <stdio.h>
#include <omp.h>
#include <vector>
#include <cstdint>
using namespace std;

int getRandomNumber(int min = -120, int max = 120)
{
	return min + rand() % (max - min + 1);
}
int main(int argc, char** argv)
{


	srand(time(NULL));
	const int n = 15000, m = 15000;
	int** a = new int*[m];
	vector<int> b(n) , res(m);

	for (int i = 0; i < m; i++)
	{
		a[i] = new int[n];
	}
	for (int i = 0; i < m; i++)
	{
		b[i] = getRandomNumber(0, 10);
		for (int j = 0; j < n; j++)
		{
			a[i][j] = getRandomNumber(0, 10);
		}
	}

	auto start = omp_get_wtime();
	for (int i = 0; i < m; i++)
	{
		res[i] = 0;
		for (int j = 0; j < n; j++)
		{
			res[i] += a[i][j] * b[j];
		}
	}

	auto end = omp_get_wtime();
	cout << "\n" << "Time:" << (end - start);

	start = omp_get_wtime();
#pragma omp parallel 
	{
		int countOfThreads = omp_get_num_threads();
		int threadNum = omp_get_thread_num();
		int countRows = m / countOfThreads;
		int st = threadNum * countRows;
		int end;

		if (threadNum == countOfThreads - 1)
		{
			end = m ;
		}
		else
		{
			end = st + countRows ;
		}

		for (int i = st; i < end; i++)
		{
			res[i] = 0;
			for (int j = 0; j < n; j++)
			{
				res[i] += a[i][j] * b[j];
			}
		}
	}
	end = omp_get_wtime();
	cout << "\n" << "Time:" << (end - start);
	cout << "\n";
	//for (int i = 0; i < m; i++)
	//{
	//		cout<<res[i]<<"\n";
	//}

}