#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <random>
#include <iomanip>
#include <string>
#include <limits>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <tbb/tbb.h>
#include <algorithm>
#include "LSDParallelSorter.h"

void PrintArray(double *array, int size) {
	for (int i = 0; i < size; i++) {
		std::cout << array[i] << " ";
	}
	std::cout << std::endl;
}

void CopyArray(double *mas, double* tmp, int size) {
	for (int i = 0; i < size; i++)
		tmp[i] = mas[i];
}

void CheckingSort(double *mas, double* tmp, int size) {
	for (int i = 0; i < size; i++) {
		if (mas[i] != tmp[i]) {
			std::cout << "Sort is incorrectly" << std::endl;
			break;
		}
	}
	std::cout << "Sort is correctly" << std::endl;
}

void GenerateArray(double *mas, int size) {
	std::default_random_engine generator(static_cast<unsigned int>(time(nullptr)));
	std::uniform_real_distribution<double> distribution(0, 100);

	for (int i = 0; i < size; i++) {
		mas[i] = distribution(generator);
	}

}

void LSDParallelSortDouble(double *inp, int size, int nThreads)
{
	double *out = new double[size];
	int portion = size / nThreads;
	if (size % nThreads != 0)
		portion++;
	//tbb::task_scheduler_init init(nThreads);
	LSDParallelSorter& sorter = *new (tbb::task::allocate_root())
		LSDParallelSorter(inp, out, size, portion);
	tbb::task::spawn_root_and_wait(sorter);//запускает задачу и ожидает ее выполнения
	delete[] out;
}

int main(int argc, char *argv[]) {
	int size = 10;
	int n = 2;
	int  flag_print = 0;
	double *data, *tmp, *data_copy, *tmp1;

	std::cout << "Enter array size: ";
	std::cin >> size;

	data = new double[size];
	tmp = new double[size];
	data_copy = new double[size];
	tmp1 = new double[size];

	GenerateArray(data, size);
	CopyArray(data, tmp, size);
	std::sort(tmp, tmp + size);
	CopyArray(data, data_copy, size);

	std::cout << std::endl << "Print the array (Yes-1, No-0): " << std::endl;
	std::cin >> flag_print;

	if (flag_print == 1)
		PrintArray(data, size);

	std::cout << "Enter number of threads: ";
	std::cin >> n;
	tbb::task_scheduler_init init(n);
	std::cout << tbb::task_scheduler_init::default_num_threads()<<std::endl;
	
	tbb::tick_count t1 = tbb::tick_count::now();
	LSDParallelSortDouble(data, size, n);
	tbb::tick_count t3 = tbb::tick_count::now();

	double totalParallel = (t3 - t1).seconds();
	std::cout << "parallel version ";
	CheckingSort(data, tmp, size);

	std::cout << std::endl << "Print the array (Yes-1, No-0): " << std::endl;
	std::cin >> flag_print;
	if (flag_print == 1)
		PrintArray(data, size);

	std::cout << "Total parallel time on " << n <<
		" threads: " << totalParallel << std::endl;
	init.terminate();
	tbb::tick_count t11 = tbb::tick_count::now();
	LSDSortDouble(data_copy, tmp1, size);
	tbb::tick_count t22 = tbb::tick_count::now();
	double serialParallel = (t22 - t11).seconds();

	std::cout << "serial version ";
	CheckingSort(data_copy, tmp, size);
	std::cout << "Total time serial version: " << serialParallel << "s\n";

	delete[] data;
	delete[] tmp;
	delete[] data_copy;
	system("pause");
	return 0;
}