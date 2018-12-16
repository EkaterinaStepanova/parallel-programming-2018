#include <ctime>  
#include <mpi.h> 
#include <string>
#include <iomanip>
#include <iostream>
using namespace std;


void ShellSort(double *A, int n)
{
	int incr = n / 2;
	while (incr > 0)
	{
		for (int i = incr; i < n; i++)
		{
			int j = i - incr;
			while (j >= 0)
			{
				int k = j + incr;
				if (A[j] > A[k])
				{
					double temp;
					temp = A[j];
					A[j] = A[k];
					A[k] = temp;
					j = j - incr;
				}
				else j = -1;
			}
		}
		incr = incr / 2;
	}
}
//слияние массивов
double* Splitter(double *mas1, int size1, double *mas2, int size2, int size = 10)
{
	int a = 0;
	int b = 0;
	int i = 0;
	//double *tmp = new double[size1 + size2];
	double *tmp = new double[size];
	memset(tmp, 0, size * sizeof(double));
	while ((a != size1) && (b != size2))
	{
		if (mas1[a] <= mas2[b])
		{
			tmp[i] = mas1[a];
			a++;
		}
		else
		{
			tmp[i] = mas2[b];
			b++;
		}
		i++;
	}

	if (a == size1)
	{
		int j = b;
		for (; j < size2; j++, i++)
			tmp[i] = mas2[j];
	}
	else
	{
		int j = a;
		for (; j < size1; j++, i++)
			tmp[i] = mas1[j];
	}
	return tmp;
}
void printArray(double *arr, int size)
{
	for (int i = 0; i < size; ++i) {
		cout << i << " element is " << arr[i] << endl;
	}
	cout << endl;
}
//бинарный поиск для позиции во втором массиве
int BinSearch(double *mas, int l, int r, double x)
{
	if (l == r)
		return l;

	if (l + 1 == r)
		if (x < mas[l])
			return l;
		else
			return r;

	int m = (l + r) / 2;

	if (x < mas[m])
		r = m;
	else
		if (x > mas[m])
			l = m;
		else
			return m;

	return BinSearch(mas, l, r, x);
}
int main(int argc, char *argv[])
{
	int procNum, procRank;
	int sizeArray = argc == 1 ? 10000 : stoi(argv[1]);
	double *arr = nullptr;
	double startTime = 0.0;



	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	MPI_Status status;
	double *patrSortArray = nullptr;
	int *offset = new int[procNum];
	int *size = new int[procNum];

	if (procRank == 0) {
		arr = new double[sizeArray];
		srand(time(0));

		for (int i = 0; i < sizeArray ; i++) {
			arr[i] = rand() % 100;
		}
		

		startTime = MPI_Wtime();

		if (sizeArray <= 100) {
			printArray(arr, sizeArray);
		}
		offset[0] = 0;
		for (int i = 0; i < procNum; i++) {
			size[i] = i < (sizeArray % procNum) ? (sizeArray / procNum) + 1 : sizeArray / procNum;

			if (i > 0) {
				offset[i] = size[i - 1] + offset[i - 1];
			}

		}


	}


	int k = procRank < (sizeArray % procNum) ? (sizeArray / procNum) + 1 : sizeArray / procNum;
	patrSortArray = new  double[k];
	int iteration = log2(procNum);

	MPI_Scatterv(arr, size, offset, MPI_DOUBLE, patrSortArray, k, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	ShellSort(patrSortArray, k);

	//double *result = 0;
	double * res = nullptr;
	int sizePartForMerge = 0;
	int sizeSecondInteractivePr;
	for (int step = 0; step < iteration; step++) {
		
		if ((procRank % ((int)pow(2, step + 1))) == 0 && procRank != procNum - 1) {

			k = k * (step + 1);

			MPI_Recv(&sizeSecondInteractivePr, 1, MPI_INT, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD, &status);
			
			MPI_Send(&k, 1, MPI_INT, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD);
			MPI_Send(&patrSortArray[k / 2], k - k / 2, MPI_DOUBLE, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD);
			MPI_Recv(&sizePartForMerge, 1, MPI_INT, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD, &status);

			res = new double[sizePartForMerge];

			MPI_Recv(res, sizePartForMerge, MPI_DOUBLE, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD, &status);

			double *tmp = new double[sizeSecondInteractivePr + k];
			memset(tmp, 0, (sizeSecondInteractivePr + k) * sizeof(double));

			tmp = Splitter(patrSortArray, k / 2, res, sizePartForMerge, sizeSecondInteractivePr + k);

			MPI_Recv(&tmp[sizePartForMerge + k / 2], sizeSecondInteractivePr - sizePartForMerge + k - k / 2, MPI_DOUBLE, procRank + (int)pow(2, step), 0, MPI_COMM_WORLD, &status);
		
			delete[]res;
			delete[]patrSortArray;
			patrSortArray = tmp;
		}
		else {
			
			if (procRank % (int)pow(2, step) == 0)
			{
				k = k * (step + 1);

				MPI_Send(&k, 1, MPI_INT, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD);
				MPI_Recv(&sizeSecondInteractivePr, 1, MPI_INT, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD, &status);

				double *result = new double[sizeSecondInteractivePr - sizeSecondInteractivePr / 2];
				MPI_Recv(result, sizeSecondInteractivePr - sizeSecondInteractivePr / 2, MPI_DOUBLE, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD, &status);
				int midForX = BinSearch(patrSortArray, 0, sizeSecondInteractivePr, result[0]);
				
				MPI_Send(&midForX, 1, MPI_INT, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD);
				MPI_Send(&patrSortArray[0], midForX, MPI_DOUBLE, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD);

				double *tmp = new double[k - midForX + sizeSecondInteractivePr - sizeSecondInteractivePr / 2];
				tmp = Splitter(&patrSortArray[midForX], k - midForX, result, sizeSecondInteractivePr - sizeSecondInteractivePr / 2, k - midForX + sizeSecondInteractivePr - sizeSecondInteractivePr / 2);

				MPI_Send(tmp, k - midForX + sizeSecondInteractivePr - sizeSecondInteractivePr / 2, MPI_DOUBLE, procRank - (int)pow(2, step), 0, MPI_COMM_WORLD);
				delete[] result;
				delete[] tmp;
			}

		}
	}

	if (procRank == 0) {
		

		if (sizeArray < 100) {
			cout << "result:" << endl;
			printArray(patrSortArray, sizeArray);
		}
		cout << endl;
		cout << "parallel version:" << endl;
		cout << endl << "Time is " << MPI_Wtime() - startTime << "s" << endl;
		double sTime = 0.0;
		startTime = MPI_Wtime();
		ShellSort(arr, sizeArray);
		cout << endl;
		cout << "serial version:" << endl;
		cout << endl << "Time is " << MPI_Wtime() - startTime << "s" << endl;
		
	}
	MPI_Finalize();
	return 0;
}
