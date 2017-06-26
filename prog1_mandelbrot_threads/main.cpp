#include <stdio.h>
#include <algorithm>
#include "CycleTimer.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

extern void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations,
    int output[]);

extern void writePPMImage(
    int* data,
    int width, int height,
    const char *filename,
    int maxIterations);

void
scaleAndShift(float& x0, float& x1, float& y0, float& y1,
              float scale,
              float shiftX, float shiftY)
{

    x0 *= scale;
    x1 *= scale;
    y0 *= scale;
    y1 *= scale;
    x0 += shiftX;
    x1 += shiftX;
    y0 += shiftY;
    y1 += shiftY;

}

void usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -t  --threads <N>  Use N threads\n");
    printf("  -v  --view <INT>   Use specified view settings\n");
    printf("  -?  --help         This message\n");
}

bool verifyResult (int *gold, int *result, int width, int height) {

    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (gold[i * width + j] != result[i * width + j]) {
                printf ("Mismatch : [%d][%d], Expected : %d, Actual : %d\n",
                            i, j, gold[i * width + j], result[i * width + j]);
                return 0;
            }
        }
    }

    return 1;
}

int main(int argc, char** argv) {

    const unsigned int width = 1200;
    const unsigned int height = 800;
    const int maxIterations = 256;
    int numThreads = 2;

    float x0 = -2;
    float x1 = 1;
    float y0 = -1;
    float y1 = 1;

    // parse commandline options ////////////////////////////////////////////
    
	for (int i = 1; i < argc; i++) {
		if (i < argc - 1)
		{
			if (strcmp(argv[i], "-threads") == 0 || strcmp(argv[i], "t") == 0)
			{
				numThreads = atoi(argv[i + 1]);
			}
			else if (strcmp(argv[i], "-view") == 0 || strcmp(argv[i], "-v") == 0)
			{
				int viewIndex = atoi(argv[i + 1]);
				// change view settings
				if (viewIndex == 2) {
					float scaleValue = .015f;
					float shiftX = -.986f;
					float shiftY = .30f;
					scaleAndShift(x0, x1, y0, y1, scaleValue, shiftX, shiftY);
				}
				else if (viewIndex > 1) {
					fprintf(stderr, "Invalid view index\n");
					return 1;
				}
			}
		}
		else
		{
			if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-?") == 0)
			{
				usage(argv[0]);
				return 1;
			}
		}
    }
    // end parsing of commandline options


    int* output_serial = new int[width*height];
    int* output_thread = new int[width*height];

    //
    // Run the serial implementation.  Run the code three times and
    // take the minimum to get a good estimate.
    //
    memset(output_serial, 0, width * height * sizeof(int));
    double minSerial = 1e30;
    for (int i = 0; i < 5; ++i) {
        double startTime = CycleTimer::currentSeconds();
        mandelbrotSerial(x0, y0, x1, y1, width, height, 0, height, maxIterations, output_serial);
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    printf("[mandelbrot serial]:\t\t[%.3f] ms\n", minSerial * 1000);
    writePPMImage(output_serial, width, height, "mandelbrot-serial.ppm", maxIterations);

    //
    // Run the threaded version
    //
    memset(output_thread, 0, width * height * sizeof(int));
    double minThread = 1e30;
    for (int i = 0; i < 5; ++i) {
        double startTime = CycleTimer::currentSeconds();
        mandelbrotThread(numThreads, x0, y0, x1, y1, width, height, maxIterations, output_thread);
        double endTime = CycleTimer::currentSeconds();
        minThread = std::min(minThread, endTime - startTime);
    }

    printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
    writePPMImage(output_thread, width, height, "mandelbrot-thread.ppm", maxIterations);

    if (! verifyResult (output_serial, output_thread, width, height)) {
        printf ("Error : Output from threads does not match serial output\n");

        delete[] output_serial;
        delete[] output_thread;

        return 1;
    }

    // compute speedup
    printf("\t\t\t\t(%.2fx speedup from %d threads)\n", minSerial/minThread, numThreads);

    delete[] output_serial;
    delete[] output_thread;

    return 0;
}