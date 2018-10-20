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
	double *Matr� = nullptr;
	double *partresult = nullptr;
	double startTime = 0.0;

	MPI_Init(&argc, &argv);//������������� ����� ����������

	MPI_Comm_size(MPI_COMM_WORLD, &procNum);// ����������� ���-�� ���������
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);//����������� ����� ��������

	MPI_Status status; //���������� ������� ���������� �������� ������ ������

	if (procRank == 0) { //���������� �������
		srand(time(0));
		Matr� = new double[m*n];
		for (int i = 0; i < m*n; i++)
			Matr�[i] = rand() % 10;

		startTime = MPI_Wtime();

		if (m*n < 10000) {
			cout << "original matrix: " << endl;
			printMatrix(Matr�, n, m);
		}

		double *nMatr = new double[m*n];
		transposeMatrix(Matr�, nMatr, n, m);
		delete Matr�;
		Matr� = nMatr;
	}

	int k = n / procNum;  // �������� � ���. ��������� �������� �� �������
	tmp = new double[k*m];
	partresult = new double[k];
	memset(partresult, 0, k * sizeof(double));
	//������ n- ������, m- �������

	MPI_Scatter(Matr�, k*m, MPI_DOUBLE, tmp, k*m, MPI_DOUBLE, 0, MPI_COMM_WORLD);

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
		int tail = n - procNum * k; //���� ���� �������

		for (int i = n - tail; i < n; i++) {
			for (int j = 0; j < m; j++) {
				result[i] += Matr�[i * m + j];
			}
		}

		if (n < 50)
			printResult(result, n);

		cout << endl;
		cout << endl << "Time is " << MPI_Wtime() - startTime << "s" << endl;
	}

	MPI_Finalize();//���������� ������ � MPI

	return 0;

}