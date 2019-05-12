#pragma once
#include <stdlib.h>
#include <tbb/tbb.h>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <climits>
#include "tbb/task_scheduler_init.h"
using namespace tbb;

void CountingSort(double *inp, double *out, int byteNum, int size)
{
	unsigned char *mas = (unsigned char *)inp;
	int counter[256];
	int tem;
	memset(counter, 0, sizeof(int) * 256);
	for (int i = 0; i < size; i++)
		counter[mas[8 * i + byteNum]]++;
	int j = 0;
	for (; j < 256; j++)
	{
		if (counter[j] != 0)
			break;
	}
	tem = counter[j];
	counter[j] = 0;
	j++;
	for (; j < 256; j++)
	{
		int b = counter[j];
		counter[j] = tem;
		tem += b;
	}
	for (int i = 0; i < size; i++)
	{
		out[counter[mas[8 * i + byteNum]]] = inp[i];
		counter[mas[8 * i + byteNum]]++;
	}
}

void LSDSortDouble(double *inp, double* out, int size)
{
	//double *out = new double[size];
	CountingSort(inp, out, 0, size);
	CountingSort(out, inp, 1, size);
	CountingSort(inp, out, 2, size);
	CountingSort(out, inp, 3, size);
	CountingSort(inp, out, 4, size);
	CountingSort(out, inp, 5, size);
	CountingSort(inp, out, 6, size);
	CountingSort(out, inp, 7, size);
	//delete[] out;
}


class LSDParallelSorter :public tbb::task
{
private:
	double *mas;
	double *tmp;
	int size;
	int portion;
	void Merge(int size1, int size2)
	{
		for (int i = 0; i < size1; i++)
			tmp[i] = mas[i];
		double *mas2 = mas + size1;
		int a = 0;
		int b = 0;
		int i = 0;
		while ((a != size1) && (b != size2))
		{
			if (tmp[a] <= mas2[b])
			{
				mas[i] = tmp[a];
				a++;
			}
			else
			{
				mas[i] = mas2[b];
				b++;
			}
			i++;
		}
		std::cout << "RRR" << std::endl;
		if (a == size1)
			for (int j = b; j < size2; j++)
				mas[size1 + j] = mas2[j];
		else
			for (int j = a; j < size1; j++)
				mas[size2 + j] = tmp[j];
	}
public:
	LSDParallelSorter(double *_mas, double *_tmp, int _size,
		int _portion) : mas(_mas), tmp(_tmp),
		size(_size), portion(_portion)
	{}
	task* execute()
	{
		if (size <= portion)
		{
			LSDSortDouble(mas, tmp, size);
		}
		else
		{
			LSDParallelSorter &sorter1 = *new (allocate_child())
				LSDParallelSorter(mas, tmp, size / 2, portion);
			LSDParallelSorter &sorter2 = *new (allocate_child())
				LSDParallelSorter(mas + size / 2, tmp + size / 2,
					size - size / 2, portion);
			set_ref_count(3);
			spawn(sorter1);
			spawn_and_wait_for_all(sorter2);
			Merge(size / 2, size - size / 2);
		}
		return NULL;
	}
};


/*class Counter :public tbb::task
{
	double *mas;
	int size;
	int byteNum;
	int *counter;
public:
	Counter(double *_mas, int _size, int _byteNum,
		int *_counter) : mas(_mas), size(_size),
		byteNum(_byteNum), counter(_counter)

		{}
		task* execute()
	{
		unsigned char *masUC = (unsigned char *)mas;
		memset(counter, 0, sizeof(int) * 256);
		for (int i = 0; i < size; i++)
			counter[masUC[8 * i + byteNum]]++;
		return NULL;
	}
};

class Placer :public tbb::task
{
	double *inp, *out;
	int size;
	int byteNum;
	int *counter;
public:
	Placer(double *_inp, double *_out, int _size,
		int _byteNum, int *_counter) : inp(_inp),
		out(_out), size(_size), byteNum(_byteNum),
		counter(_counter)
	{}
	tbb::task* execute()
	{
		unsigned char *inpUC = (unsigned char *)inp;
		for (int i = 0; i < size; i++)
		{
			out[counter[inpUC[8 * i + byteNum]]] = inp[i];
			counter[inpUC[8 * i + byteNum]]++;
		}
		return NULL;
	}
};

class ParallelCounterSort :public task
{
private:
	double *mas;
	double *tmp;
	int size;
	int nThreads;
	int *counters;
	int byteNum;
public:
	ParallelCounterSort(double *_mas, double *_tmp,
		int _size, int _nThreads,
		int *_counters, int _byteNum) :
		mas(_mas), tmp(_tmp), size(_size),
		nThreads(_nThreads),
		counters(_counters),
		byteNum(_byteNum)
	{}
	task* execute()
	{
		Counter **ctr = new Counter*[nThreads - 1];
		Placer **pl = new Placer*[nThreads - 1];
		int s = size / nThreads;
		for (int i = 0; i < nThreads - 1; i++)
			ctr[i] = new (allocate_child()) Counter(mas + i * s, s,
				byteNum, counters + 256 * i);
		Counter &ctrLast = *new (allocate_child())
			Counter(mas + s * (nThreads - 1),
				size - s * (nThreads - 1),
				byteNum,
				counters + 256 * (nThreads - 1));
		set_ref_count(nThreads + 1);
		for (int i = 0; i < nThreads - 1; i++)
			spawn(*(ctr[i]));
		spawn_and_wait_for_all(ctrLast);
		int sm = 0;
		for (int j = 0; j < 256; j++)
		{
			for (int i = 0; i < nThreads; i++)
			{
				int b = counters[j + i * 256];
				counters[j + i * 256] = sm;
				sm += b;
			}
		}
		for (int i = 0; i < nThreads - 1; i++)
			pl[i] = new (allocate_child()) Placer(mas + i * s, tmp,
				s, byteNum,
				counters + 256 * i);
				Placer &plLast = *new (allocate_child())
				Placer(mas + s * (nThreads - 1), tmp,
					size - s * (nThreads - 1), byteNum,
					counters + 256 * (nThreads - 1));
		set_ref_count(nThreads + 1);
		for (int i = 0; i < nThreads - 1; i++)
			spawn(*(pl[i]));
		spawn_and_wait_for_all(plLast);
		delete[] pl;
		delete[] ctr;
		return NULL;
	}
};

class LSDParallelSorter :public task
{
private:
	double *mas;
	double *tmp;
	int size;
	int nThreads;
public:
	LSDParallelSorter(double *_mas, double *_tmp, int _size,
		int _nThreads) : mas(_mas), tmp(_tmp),
		size(_size), nThreads(_nThreads)
	{}
	task* execute()
	{
		int *counters = new int[256 * nThreads];
		ParallelCounterSort *pcs = new (allocate_child())
			ParallelCounterSort(mas, tmp, size, nThreads,
				counters, 0);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(tmp, mas, size, nThreads,
				counters, 1);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(mas, tmp, size, nThreads,
				counters, 2);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(tmp, mas, size, nThreads,
				counters, 3);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(mas, tmp, size, nThreads,
				counters, 4);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(tmp, mas, size, nThreads,
				counters, 5);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(mas, tmp, size, nThreads,
				counters, 6);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		pcs = new (allocate_child())
			ParallelCounterSort(tmp, mas, size, nThreads,
				counters, 7);
		set_ref_count(2);
		spawn_and_wait_for_all(*pcs);
		delete[] counters;
		return NULL;
	}
};*/