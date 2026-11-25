#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <stdexcept>
#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb.h"

using namespace std;

// Convert HSV colour values to RGB, matching the sequential implementation.
void HSVtoRGB(double h, double s, double v,
    unsigned char& r,
    unsigned char& g,
    unsigned char& b) {

    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double rr = 0, gg = 0, bb = 0;

    if (h < 60) {
        rr = c; gg = x; bb = 0;
    }
    else if (h < 120) {
        rr = x; gg = c; bb = 0;
    }
    else if (h < 180) {
        rr = 0; gg = c; bb = x;
    }
    else if (h < 240) {
        rr = 0; gg = x; bb = c;
    }
    else if (h < 300) {
        rr = x; gg = 0; bb = c;
    }
    else {
        rr = c; gg = 0; bb = x;
    }

    r = static_cast<unsigned char>((rr + m) * 255);
    g = static_cast<unsigned char>((gg + m) * 255);
    b = static_cast<unsigned char>((bb + m) * 255);
}

int main(int argc, char* argv[]) {
    // Keep these parameters identical to the group's sequential baseline.
    const int WIDTH = 1000;
    const int HEIGHT = 1000;
    const int MAX_ITERATIONS = 1000;

    // Optional command-line argument: number of OpenMP threads.
    // Example: OpenMP.exe 4
    omp_set_dynamic(0);
    if (argc >= 2) {
        try {
            const int requestedThreads = stoi(argv[1]);
            if (requestedThreads < 1) {
                cerr << "Thread count must be at least 1." << endl;
                return 1;
            }
            omp_set_num_threads(requestedThreads);
        }
        catch (const exception&) {
            cerr << "Usage: OpenMP.exe [positive_thread_count]" << endl;
            return 1;
        }
    }

    const int configuredThreads = omp_get_max_threads();

    // Store the iteration count of every pixel.
    vector<int> pixels(WIDTH * HEIGHT);

    // Measure only the Mandelbrot computation, matching the sequential code.
    const auto start = chrono::high_resolution_clock::now();

    // OpenMP distributes rows among CPU threads.
    // Each thread writes to different positions in the pixels array.
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Convert pixel coordinates to the same complex plane as Sequential.
            const double real = (x * 3.5 / WIDTH) - 2.5;
            const double imag = (y * 2.0 / HEIGHT) - 1.0;

            // Initialize Z = 0.
            double zr = 0.0;
            double zi = 0.0;
            int iteration = 0;

            // Mandelbrot iteration: identical calculation to Sequential.
            while ((zr * zr + zi * zi <= 4.0) &&
                   (iteration < MAX_ITERATIONS)) {
                const double temp = zr * zr - zi * zi + real;
                zi = 2.0 * zr * zi + imag;
                zr = temp;
                iteration++;
            }

            pixels[y * WIDTH + x] = iteration;
        }
    }

    const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double> elapsed = end - start;

    // Convert the iteration counts to RGB after timing, as in Sequential.
    vector<unsigned char> rgb(WIDTH * HEIGHT * 3);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            const int iteration = pixels[y * WIDTH + x];
            const int index = (y * WIDTH + x) * 3;

            if (iteration == MAX_ITERATIONS) {
                // Points inside the Mandelbrot set are black.
                rgb[index] = 0;
                rgb[index + 1] = 0;
                rgb[index + 2] = 0;
            }
            else {
                // Outside points use the same rainbow colour mapping.
                const double hue = fmod(iteration * 3.0, 360.0);
                unsigned char red, green, blue;
                HSVtoRGB(hue, 1.0, 1.0, red, green, blue);
                rgb[index] = red;
                rgb[index + 1] = green;
                rgb[index + 2] = blue;
            }
        }
    }

    // Keep a separate output filename so the sequential image is not overwritten.
    if (!stbi_write_png(
        "mandelbrot_openmp.png",
        WIDTH,
        HEIGHT,
        3,
        rgb.data(),
        WIDTH * 3)) {
        cerr << "Failed to save mandelbrot_openmp.png" << endl;
        return 1;
    }

    cout << "Mandelbrot Set Generation - OpenMP" << endl;
    cout << "Image Width: " << WIDTH << endl;
    cout << "Image Height: " << HEIGHT << endl;
    cout << "Maximum Iterations: " << MAX_ITERATIONS << endl;
    cout << "Total Pixels: " << WIDTH * HEIGHT << endl;
    cout << "Configured OpenMP Threads: " << configuredThreads << endl;
    cout << "Scheduling: static" << endl;
    cout << "Execution Time: " << elapsed.count() << " seconds" << endl;
    cout << "Image saved as mandelbrot_openmp.png" << endl;

    return 0;
}
