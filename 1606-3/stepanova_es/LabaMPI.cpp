#include "mpi.h" 
#include <iostream> 
#include <string>
#include <random>
#include <iomanip>

using namespace std;

void transposeMatrix(const double *matrix, double *trmatrix, const int column, const int row) {
	for (int i = 0; i<row*column; i++) {
		int k = i / row;
		int j = i % row;
		trmatrix[i] = matrix[column * j + k];

	}

}
void printMatrix(const double *arr, int n, int m)
{

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			cout << setw(5) << arr[i*n + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	cout << endl;
}

void printResult(double *arr, int size)
{
	for (int i = 0; i < size; i++) {
		cout << "the sum of the " << i << " column is " << arr[i] << endl;
	}

}
int main(int argc, char *argv[])
{
	int procNum, procRank;
	int m = argc == 1 ? 3 : std::stod(argv[1]);
	int n = argc == 1 ? 3 : std::stod(argv[2]);
	double *result = nullptr;
	double *tmp = nullptr;
	double *Matrх = nullptr;
	double *partresult = nullptr;
	double startTime = 0.0;

	MPI_Init(&argc, &argv);//инициализация среды выполнения

	MPI_Comm_size(MPI_COMM_WORLD, &procNum);// определение кол-ва процессов
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);//определение ранга процесса

	MPI_Status status; //переменная статуса выполнения операции приема данных

	if (procRank == 0) { //заполнение матрицы
		srand(time(0));
		Matrх = new double[m*n];
		for (int i = 0; i < m*n; i++)
			Matrх[i] = rand() % 10;

		startTime = MPI_Wtime();

		if (m*n < 10000) {
			cout << "original matrix: " << endl;
			printMatrix(Matrх, n, m);
		}

		double *nMatr = new double[m*n];
		transposeMatrix(Matrх, nMatr, n, m);
		delete Matrх;
		Matrх = nMatr;
	}

	int k = n / procNum;  // диапазон в кот. считаются значения по потокам
	tmp = new double[k*m];
	partresult = new double[k];
	memset(partresult, 0, k * sizeof(double));
	//теперь n- строки, m- столбцы

	MPI_Scatter(Matrх, k*m, MPI_DOUBLE, tmp, k*m, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < m; j++) {
			partresult[i] += tmp[i*m + j];
		}
	}

	if (procRank == 0) {
		result = new double[n];
		memset(result, 0, n * sizeof(double));
	}

	MPI_Gather(partresult, k, MPI_DOUBLE, result, k, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if (procRank == 0) {
		int tail = n - procNum * k; //если есть остаток

		for (int i = n - tail; i < n; i++) {
			for (int j = 0; j < m; j++) {
				result[i] += Matrх[i * m + j];
			}
		}

		if (n < 50)
			printResult(result, n);

		cout << endl;
		cout << endl << "Time is " << MPI_Wtime() - startTime << "s" << endl;
	}

	MPI_Finalize();//завершение работы с MPI

	return 0;

}