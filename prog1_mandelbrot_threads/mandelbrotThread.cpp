#include <stdio.h>
#include <thread>

#include "CycleTimer.h"

typedef struct {
	float x0, x1;
	float y0, y1;
	unsigned int width;
	unsigned int height;
	int maxIterations;
	int* output;
	int threadId;
	int numThreads;
	int startRow;
	int numRows;
} WorkerArgs;


extern void mandelbrotSerial(
		float x0, float y0, float x1, float y1,
		int width, int height,
		int startRow, int numRows,
		int maxIterations,
		int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void* workerThreadStart(void* threadArgs) {

	WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

	// TODO: Implement worker thread here.
	mandelbrotSerial(args->x0, args->y0, args->x1, args->y1,
			args->width, args->height, args->startRow,
			args->numRows, args->maxIterations, args->output);

	//printf("Hello world from thread %d\n", args->threadId);

	return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
		int numThreads,
		float x0, float y0, float x1, float y1,
		int width, int height,
		int maxIterations, int output[], long long cdf[])
{
	const static int MAX_THREADS = 48;

	if (numThreads > MAX_THREADS)
	{
		fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
		exit(1);
	}

	

	std::thread workers[MAX_THREADS ];
	WorkerArgs args[MAX_THREADS];

	long long dis = cdf[959] / numThreads; 

	int rows = 0;

	for (int i = 0; i < numThreads; i++) {
		// TODO: Set thread arguments here.
		args[i].threadId = i;
		args[i].x0 = x0;
		args[i].x1 = x1;
		args[i].y0 = y0;
		args[i].y1 = y1;
		args[i].maxIterations = maxIterations;
		args[i].width = width;
		args[i].height = height;
		int pre_rows = rows;
		long long pre_num = cdf[pre_rows];
		while(cdf[rows] - pre_num < dis && rows < height) {
			rows ++;
		}
		args[i].startRow = pre_rows;
		//printf("%d start at %d\n", i, pre_rows);
		args[i].numRows = rows - pre_rows;
		//args[i].output = &output[ height / numThreads * i * width ];
		//args[i].output = output + height / numThreads * i * width ;
		args[i].output = output;
		args[i].numThreads = numThreads;
		//args[i].startRow = height / numThreads * i;
		//args[i].totalRows = height / numThreads;
	}

	// Fire up the worker threads.  Note that numThreads-1 pthreads
	// are created and the main app thread is used as a worker as
	// well.

	for (int i = 0; i < numThreads ; i++)
	{
		workers[i] = std::thread(workerThreadStart, &args[i]);
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i, &cpuset);
		pthread_setaffinity_np(workers[i].native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
	}

	//workerThreadStart(&args[0]);

	// wait for worker threads to complete
	for (int i = 0; i < numThreads ; i++)
		workers[i].join();
}
