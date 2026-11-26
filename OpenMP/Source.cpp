#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb.h"

using namespace std;

// Convert HSV colour values to RGB. This matches the group's Sequential version.
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

// Calculate the Mandelbrot iteration count for every pixel in parallel.
// The mathematical calculation and coordinate mapping match Sequential/Source.cpp.
void calculateMandelbrotOpenMP(
    vector<int>& pixels,
    int WIDTH,
    int HEIGHT,
    int MAX_ITERATIONS) {

#pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Convert pixel coordinates to the complex plane.
            const double real = (x * 3.5 / WIDTH) - 2.5;
            const double imag = (y * 2.0 / HEIGHT) - 1.0;

            // Initialize Z = 0.
            double zr = 0.0;
            double zi = 0.0;
            int iteration = 0;

            // Mandelbrot iteration: same formula and stopping conditions as Sequential.
            while ((zr * zr + zi * zi <= 4.0) &&
                (iteration < MAX_ITERATIONS)) {
                const double temp = zr * zr - zi * zi + real;
                zi = 2.0 * zr * zi + imag;
                zr = temp;
                iteration++;
            }

            // Each loop iteration writes to a different pixel index.
            pixels[y * WIDTH + x] = iteration;
        }
    }
}

// Generate a PNG from a completed iteration-count array.
// Image generation is outside the timed computation section, matching Sequential.
void generatePNG(
    const vector<int>& pixels,
    int WIDTH,
    int HEIGHT,
    int MAX_ITERATIONS,
    const string& filename) {

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
                // Outside points use the same rainbow colour mapping as Sequential.
                const double hue = fmod(iteration * 3.0, 360.0);
                unsigned char red, green, blue;
                HSVtoRGB(hue, 1.0, 1.0, red, green, blue);
                rgb[index] = red;
                rgb[index + 1] = green;
                rgb[index + 2] = blue;
            }
        }
    }

    // Prefix the file name so OpenMP output cannot overwrite Sequential images.
    const int result = stbi_write_png(
        filename.c_str(),
        WIDTH,
        HEIGHT,
        3,
        rgb.data(),
        WIDTH * 3
    );

    if (result == 0) {
        cerr << "Error: Failed to save " << filename << endl;
    }
}

int main(int argc, char* argv[]) {
    // Keep the experiment settings aligned with the current Sequential baseline.
    const int WIDTH = 1000;
    const int HEIGHT = 1000;
    const int NUM_RUNS = 5;
    const int NUM_SIZES = 5;
    const int TOTAL_PIXELS = WIDTH * HEIGHT;

    // Same five maximum-iteration test cases as Sequential/Source.cpp.
    const int iterationSizes[NUM_SIZES] = { 1000, 2000, 3000, 4000, 5000 };
    double averageTimes[NUM_SIZES] = {};

    // Optional command-line argument specifies the number of OpenMP threads.
    // Example: OpenMP.exe 4
    omp_set_dynamic(0);
    if (argc >= 2) {
        try {
            size_t parsedCharacters = 0;
            const string threadText = argv[1];
            const int requestedThreads = stoi(threadText, &parsedCharacters);

            if (parsedCharacters != threadText.size() || requestedThreads < 1) {
                cerr << "Thread count must be a positive integer." << endl;
                cerr << "Usage: OpenMP.exe [positive_thread_count]" << endl;
                return 1;
            }

            omp_set_num_threads(requestedThreads);
        }
        catch (const exception&) {
            printf("Usage: OpenMP.exe [positive_thread_count]\n");
            return 1;
        }
    }

    const int configuredThreads = omp_get_max_threads();

    // Save each thread-count experiment to a separate CSV file.
    const string csvFilename =
        "openmp_results_threads_" + to_string(configuredThreads) + ".csv";
    ofstream csv(csvFilename);

    if (!csv.is_open()) {
        cerr << "Error: Cannot create " << csvFilename << endl;
        return 1;
    }

    csv << fixed << setprecision(9);
    csv << "Width,Height,Total Pixels,Maximum Iterations,Thread Count";
    for (int run = 1; run <= NUM_RUNS; run++) {
        csv << ",Run " << run;
    }
    csv << ",Average Execution Time (seconds)\n";

    printf("================================================\n");
    printf("       MANDELBROT - OPENMP BENCHMARK\n");
    printf("================================================\n");
    printf("Image Resolution: %d x %d\n", WIDTH, HEIGHT);
    printf("Total Pixels: %d\n", TOTAL_PIXELS);
    printf("OpenMP Threads: %d\n", configuredThreads);
    printf("Number of Test Cases: %d\n", NUM_SIZES);
    printf("Warm-up Runs per Test: 1\n");
    printf("Measured Runs per Test: %d\n", NUM_RUNS);
    printf("Total Measured Runs: %d\n", NUM_SIZES * NUM_RUNS);

    // Run the same iteration-count experiments as Sequential.
    for (int test = 0; test < NUM_SIZES; test++) {
        const int MAX_ITERATIONS = iterationSizes[test];
        vector<int> pixels(TOTAL_PIXELS);
        double totalTime = 0.0;
        double runTimes[NUM_RUNS] = {};

        printf("\n------------------------------------------------\n");
        printf("Test %d / %d\n", test + 1, NUM_SIZES);
        printf("Image Resolution: %d x %d\n", WIDTH, HEIGHT);
        printf("Maximum Iterations: %d\n", MAX_ITERATIONS);

        // One warm-up run, excluded from measurements, matching Sequential.
        printf("\nPerforming Warm-up Run...\n");
        calculateMandelbrotOpenMP(pixels, WIDTH, HEIGHT, MAX_ITERATIONS);
        printf("Warm-up Completed.\n");

        // Five measured runs, matching NUM_RUNS in the Sequential baseline.
        for (int run = 0; run < NUM_RUNS; run++) {
            const auto start = chrono::steady_clock::now();
            calculateMandelbrotOpenMP(pixels, WIDTH, HEIGHT, MAX_ITERATIONS);
            const auto end = chrono::steady_clock::now();

            const chrono::duration<double> elapsed = end - start;
            runTimes[run] = elapsed.count();
            totalTime += elapsed.count();

            printf("Run %d Execution Time: %f seconds\n", run + 1, runTimes[run]);
        }

        const double averageTime = totalTime / NUM_RUNS;
        averageTimes[test] = averageTime;

        printf("\nAverage Execution Time: %f seconds\n", averageTime);

        csv << WIDTH << ','
            << HEIGHT << ','
            << TOTAL_PIXELS << ','
            << MAX_ITERATIONS << ','
            << configuredThreads;
        for (int run = 0; run < NUM_RUNS; run++) {
            csv << ',' << runTimes[run];
        }
        csv << ',' << averageTime << '\n';

        // Keep images for the different iteration counts, without overwriting Sequential output.
        const string imageFilename =
            "mandelbrot_openmp_iter_" + to_string(MAX_ITERATIONS) + ".png";
        generatePNG(pixels, WIDTH, HEIGHT, MAX_ITERATIONS, imageFilename);
        printf("Image saved: %s\n", imageFilename.c_str());
    }

    csv.close();
    if (!csv) {
        cerr << "Error: Failed to write " << csvFilename << endl;
        return 1;
    }

    printf("\n\n");
    printf("=========================================================================\n");
    printf("               OPENMP BENCHMARK - PERFORMANCE SUMMARY\n");
    printf("=========================================================================\n");
    printf("%-10s %-20s %-20s %-25s\n", "Test", "Max Iterations", "Total Pixels", "Average Time (sec)");
    printf("-------------------------------------------------------------------------\n");

    for (int test = 0; test < NUM_SIZES; test++) {
        printf("%-10d %-20d %-20d %-25.6f\n",
               test + 1,
               iterationSizes[test],
               TOTAL_PIXELS,
               averageTimes[test]);
    }

    printf("=========================================================================\n");
    printf("\nOpenMP Benchmark Completed Successfully!\n");
    printf("Total Test Cases: %d\n", NUM_SIZES);
    printf("Warm-up Runs: %d\n", NUM_SIZES);
    printf("Total Measured Runs: %d\n", NUM_SIZES * NUM_RUNS);
    printf("Results saved: %s\n", csvFilename.c_str());

    return 0;
}