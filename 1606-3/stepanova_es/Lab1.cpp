#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <random>
#include <iomanip>
#include <string>
#include <limits>
#include <chrono>

void CSort(double *inp, double *out, int numByte, int size)
{
	unsigned char *mas = (unsigned char *)inp;
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
		out[counter[mas[8 * i + numByte]]] = inp[i];
		counter[mas[8 * i + numByte]]++;
	}
}
double* LSDSortDouble(double *inp, int size)
{
	int size_min=0;
	for (int i = 0; i < size; i++)
		if (inp[i] < 0)
			size_min++;
		
	double *out = new double[size];
	for (size_t i = 0; i < sizeof(double); i++) {

		CSort(inp, out, i, size);
		std::swap(inp, out);
	}
	if (size_min>0) {
		std::swap(inp, out);
		int tmp = size_min;
		for (size_t i = 0; i < size; i++) {
			if (tmp > 0) {
				inp[i] = out[size - i-1];
				tmp--;
			}
			else
				inp[i] = out[i-size_min];
		}
	}

	return inp;
	delete[] out;
}

int main() {

	double *data = nullptr;
	
	int n,t;

	std::cout << "Enter array size: ";
	std::cin >> n;
	std::cout<<std::endl << "Print the array: "<<std::endl;
	std::cout << std::endl << "Yes-1, No-0 " << std::endl;
	std::cin >> t;
	std::default_random_engine generator(static_cast<unsigned int>(time(nullptr)));
	std::uniform_real_distribution<double> distribution(-10, 10);

	data = new double[n];
	for (size_t i = 0; i < n; i++) {
		data[i] = distribution(generator);
		
	}
	if(t ==1)
	for (size_t i = 0; i < n; i++) {
		data[i] = distribution(generator);
		std::cout << data[i] << std::endl;
	}

	unsigned int start_time = clock();
	data = LSDSortDouble(data, n);
	unsigned int end_time = clock(); 
	
	std::cout << std::endl << "Print the array: " << std::endl;
	std::cout << std::endl << "Yes-1, No-0 " << std::endl;
	std::cin >> t;
	if( t == 1)
	for (int i = 0; i < n; i++) {
		std::cout << data[i] << std::endl;
	}
	

	for (int i = 1; i < n; i++) {
		if (data[i - 1] > data[i]) {
			std::cout << "Error\n";
		}

	}
	std::cout << "Total time: " << end_time - start_time << "ms\n";
	return 0;
}