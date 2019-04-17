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
#include <climits>
#include <omp.h>

using namespace std;

void CountingSort(double *in, double *out, int numByte, int size)
{
	unsigned char *mas = (unsigned char *)in;
	int counter[256];
	int offset;
	memset(counter, 0, sizeof(int) * 256);
	for (int i = 0; i < size; i++)
		counter[mas[8 * i + numByte]]++;
	int j = 0;
	for (; j < 256; j++)
	{
		if (counter[j] != 0)
			break;
	}
	offset = counter[j];
	counter[j] = 0;
	j++;
	for (; j < 256; j++)
	{
		int tmp = counter[j];
		counter[j] = offset;
		offset += tmp;
	}
	for (int i = 0; i < size; i++)
	{
		out[counter[mas[8 * i + numByte]]] = in[i];
		counter[mas[8 * i + numByte]]++;
	}
}

void SortDouble(double *inp, int size)
{
	int tid = omp_get_thread_num();

	int size_min = 0;
	for (int i = 0; i < size; i++)
		if (inp[i] < 0)
			size_min++;

	double *out = new double[size];
	for (size_t i = 0; i < sizeof(double); i++) {

		CountingSort(inp, out, i, size);
		std::swap(inp, out);

	}

	if (size_min > 0) {
		reverse(&inp[size - size_min], &inp[size]);
		//reverse(&inp[0], &inp[size - size_min]);
		//reverse(&inp[0], &inp[size]);
		rotate(&inp[0], &inp[size - size_min], &inp[size]);

	}
}

void CheckingSort(double *mas, int size) {
	bool flag = false;
	for (int i = 0; i < size - 1; i++) {
		if (mas[i] > mas[i + 1]) {
			std::cout << "sort is incorrecrtly" << std::endl;
			flag = true;
			break;
		}
	}
	if(!flag)
	std::cout << "sort is correcrtly" << std::endl;
}

void PrintArray(double *array, int size) {
	for (int i = 0; i < size; i++) {
		std::cout << array[i] << " ";
	}
	std::cout << std::endl;

}

void GenerateArray(double *mas, int size)
{
	std::default_random_engine generator(static_cast<unsigned int>(time(nullptr)));
	std::uniform_real_distribution<double> distribution(0, 100);
#pragma omp parallel for
	for (int i = 0; i < size; i++) {
		mas[i] = distribution(generator);
	}
	

}
void CopyArray(double *mas, double* tmp, int size)
{
	for (int i = 0; i < size; i++)
		tmp[i] = mas[i];
}
void merge(double* mas, int sizel, int sizer, int sizeR, int offset) {
	
	int size;

	if (omp_get_thread_num() == 0)
		if (sizeR <= sizel + sizer)
			size = sizeR;
		else
			size = sizel + sizer;
	else
		if (sizeR <= sizel + sizer)
			size = sizeR;
		else if (sizeR <= (offset + sizel + sizer) && omp_get_thread_num() == (omp_get_num_threads() - 1))
			size = sizeR - offset;
		else
			size = sizel + sizer;

	double* tempMas = new double[size];
	int i = 0, j = sizel, k = 0;
	
	while (i != sizel && j != size) {
		if (mas[i] <= mas[j]) {
			tempMas[k] = mas[i];
			++i;
			++k;
		}
		else {
			tempMas[k] = mas[j];
			++j;
			++k;
		}
	
	}

	if (i < sizel) {
		for (; i < sizel; ++i) {
			tempMas[k] = mas[i];
			++k;
		}
	}
	if (j < size) {
		for (; j < size; ++j) {
			tempMas[k] = mas[j];
			++k;
		}
	}
	for (i = 0; i < size; ++i) {
		mas[i] = tempMas[i];
	}

	delete[] tempMas;
}

int main() {

	double *data = nullptr;
	int size = 50000000, flag_print = 0;
	double *data_copy = nullptr;

	std::cout << "Enter array size: ";

	std::cin >> size;

	data = new double[size];
	data_copy = new double[size];
	GenerateArray(data, size);
	CopyArray(data, data_copy, size);
	//GenerateArray(data_copy, size);
	std::cout << std::endl << "Print the array (Yes-1, No-0): " << std::endl;
	std::cin >> flag_print;

	if (flag_print == 1)
		PrintArray(data, size);
	int n;
	std::cout << "Enter number of threads: ";
	std::cin >> n;
	int tail = size % n;
	double start_time = omp_get_wtime();
	
	omp_set_num_threads(n);
#pragma omp parallel
	{
		if (omp_get_thread_num() == 0) {
			SortDouble(data, size / n + tail);
		}
		else {
			SortDouble(data + tail + omp_get_thread_num()*(size / n), size / n);
		}
#pragma omp barrier
	}
	int j = 1;
	int k = n / 2;
	int d = n;
	while (k != 0) {
		omp_set_num_threads(k);
#pragma omp parallel 
		{
			
			if (omp_get_thread_num() == 0) {
				
				merge(data, (size / n) * j + tail, (size / n) * j, size, 2 * omp_get_thread_num()*(size / n) * j + tail);
				
			}
			else {
				merge(data + 2 * omp_get_thread_num()*(size / n) * j + tail, (size / n) * j, (size / n) * j, size, 2 * omp_get_thread_num()*(size / n) * j + tail);

			}
#pragma omp barrier
			if (omp_get_thread_num() == 0) {
				j *= 2;
				d = d / 2 + d % 2;
				k = d / 2;
			
			}
		}

	}
	double end_time = omp_get_wtime();
	std::cout << std::endl << "Print the sort array (Yes-1, No-0): " << std::endl;
	std::cin >> flag_print;

	if (flag_print == 1)
		PrintArray(data, size);
	std::cout << "parallel version ";
	CheckingSort(data, size);
	std::cout << "Total time parallel version: " << (end_time - start_time) << "ms\n";

	double start_time2 = omp_get_wtime();
	SortDouble(data_copy, size);
	double end_time2 = omp_get_wtime();
	std::cout << "serial version ";
	CheckingSort(data_copy, size);
	std::cout << "Total time serial version: " << (end_time2 - start_time2) << "ms\n";
	
	
	system("pause");
	return 0;
}