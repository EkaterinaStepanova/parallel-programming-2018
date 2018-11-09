#include <iostream>
#include <string>
#include <iomanip>
#include"mpi.h"

using namespace std;

void transposeMatrix(const int *matrix, int *trmatrix, const int column, const int row) {
	for (int i = 0; i < row*column; i++) {
		int k = i / row;
		int j = i % row;
		trmatrix[i] = matrix[column * j + k];
	}
}
void printMatrix(const int *arr, int n, int m)
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

int main(int argc, char *argv[])
{
	int procNum, procRank;
	int columnA = argc == 1 ? 4 : std::stod(argv[2]);
	int rowB = argc == 1 ? 4 : std::stod(argv[2]);
	int columnB = argc == 1 ? 5 : std::stod(argv[3]);

	int *matrixA = nullptr;
	int *matrixB = nullptr;
	int *matrixResult = nullptr;
	int *partresult = nullptr;
	double startTime = 0.0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	MPI_Status status;

	matrixB = new int[rowB*columnB];
	int *offset = new int[procNum];
	int *size = new int[procNum];
	if (procRank == 0) {
		srand(time(0));
		matrixA = new int[rowA*columnA];
		for (int i = 0; i < rowA*columnA; i++) {
			matrixA[i] = rand() % 10;
		}
		for (int i = 0; i < rowB*columnB; i++) {
			matrixB[i] = rand() % 10;
		}

		startTime = MPI_Wtime();

		if (rowA*columnB < 100) {
			cout << "Matrix A: " << endl;
			printMatrix(matrixA, columnA, rowA);
			cout << "Matrix B: " << endl;
			printMatrix(matrixB, columnB, rowB);
		}

		int *nMatr = new int[rowB*columnB];
		transposeMatrix(matrixB, nMatr, columnB, rowB);
		delete[] matrixB;
		matrixB = nMatr;


		for (int i = 0; i < procNum; i++) {
			size[i] = i < (rowA%procNum) ? (rowA / procNum + 1)*columnA : columnA * (rowA / procNum);
			if (i == 0) {
				offset[i] = 0;
			}
			else {
				offset[i] = size[i-1] + offset[i-1];
			}
		}
		
	}
	MPI_Bcast(matrixB, rowB*columnB, MPI_INT, 0, MPI_COMM_WORLD);

	int k = procRank < (rowA % procNum) ? (rowA / procNum + 1)*columnA : columnA * (rowA / procNum);
	int *tmp = new int[k];

	partresult = new int[k*columnB / columnA];
	memset(partresult, 0, k * columnB / columnA * sizeof(int));

	MPI_Scatterv(matrixA, size, offset, MPI_INT, tmp, k, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < k / columnA; ++i) {
		for (int j = 0; j < columnB; j++) {
			for (int l = 0; l < rowB; l++) {
				partresult[i*columnB + j] += tmp[columnA*i + l] * matrixB[l + j * rowB];
			}
		}
		
	}

	if (procRank == 0) {
		matrixResult = new int[rowA*columnB];
		memset(matrixResult, 0, rowA*columnB * sizeof(int));

		for (int i = 0; i < procNum; i++) {
			size[i] = size[i] * columnB / columnA;
			if (i == 0) {
				offset[i] = 0;
			}
			else {
				offset[i] = size[i - 1] + offset[i - 1];
			}
			
			
		}
	}

	MPI_Gatherv(partresult, k * columnB / columnA, MPI_INT, matrixResult, size, offset, MPI_INT, 0, MPI_COMM_WORLD);

	if (procRank == 0) {

		if (rowA*columnB < 100) {
			printMatrix(matrixResult, columnB, rowA);
		}
		cout << endl;
		cout << endl << "Time is " << MPI_Wtime() - startTime << "s" << endl;

	}

	MPI_Finalize();
	return 0;
}
