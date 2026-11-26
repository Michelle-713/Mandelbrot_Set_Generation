#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include<cmath>
#include<iomanip>
#include<string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb.h"
using namespace std;

// Convert HSV color to RGB
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

    r = (unsigned char)((rr + m) * 255);
    g = (unsigned char)((gg + m) * 255);
    b = (unsigned char)((bb + m) * 255);
}

// Sequential Mandelbrot Calculation
void calculateMandelbrot(
    vector<int>& pixels,
    int WIDTH,
    int HEIGHT,
    int MAX_ITERATIONS) {

    for (int y = 0; y < HEIGHT; y++) {

        for (int x = 0; x < WIDTH; x++) {

            // Convert pixel coordinates to complex plane
            double real = (x * 3.5 / WIDTH) - 2.5;
            double imag = (y * 2.0 / HEIGHT) - 1.0;

            // Initialize Z = 0
            double zr = 0.0;
            double zi = 0.0;

            int iteration = 0;

            // Mandelbrot iteration
            while ((zr * zr + zi * zi <= 4.0) &&
                (iteration < MAX_ITERATIONS)) {

                double temp = zr * zr - zi * zi + real;

                zi = 2.0 * zr * zi + imag;
                zr = temp;

                iteration++;
            }

            // Store iteration count
            pixels[y * WIDTH + x] = iteration;
        }
    }
}

// Generate PNG image
void generatePNG(
    const vector<int>& pixels,
    int WIDTH,
    int HEIGHT,
    int MAX_ITERATIONS,
    const string& filename) {

    vector<unsigned char> rgb(WIDTH * HEIGHT * 3);

    for (int y = 0; y < HEIGHT; y++) {

        for (int x = 0; x < WIDTH; x++) {

            int iteration = pixels[y * WIDTH + x];

            int index = (y * WIDTH + x) * 3;

            if (iteration == MAX_ITERATIONS) {

                // Inside Mandelbrot Set: Black
                rgb[index] = 0;
                rgb[index + 1] = 0;
                rgb[index + 2] = 0;
            }
            else {

                // Outside Mandelbrot Set: Rainbow Colors
                double hue = fmod(iteration * 3.0, 360.0);

                unsigned char red, green, blue;

                HSVtoRGB(hue, 1.0, 1.0,
                    red, green, blue);

                rgb[index] = red;
                rgb[index + 1] = green;
                rgb[index + 2] = blue;
            }
        }
    }

    // Save PNG image
    int result = stbi_write_png(
        filename.c_str(),
        WIDTH,
        HEIGHT,
        3,
        rgb.data(),
        WIDTH * 3
    );

    if (result == 0) {
        cout << "Error: Failed to save "
            << filename << endl;
    }
}


int main() {

    // Step 1: Set experiment parameters
    const int WIDTH = 1000;
    const int HEIGHT = 1000;

    const int NUM_RUNS = 5;
    const int NUM_SIZES = 5;

    const int TOTAL_PIXELS = WIDTH * HEIGHT;

    // Different Maximum Iteration values
    const int iterationSizes[NUM_SIZES] = {
        1000, 2000, 3000, 4000, 5000
    };

    // Store average execution times
    double averageTimes[NUM_SIZES] = {};

    // Step 2: Create CSV file
    ofstream csv("sequential_results.csv");

    if (!csv.is_open()) {
        cout << "Error: Cannot create CSV file." << endl;
        return 1;
    }

    // Set CSV precision to 9 decimal places
    csv << fixed << setprecision(9);

    csv << "Width,Height,Total Pixels,Maximum Iterations";

    for (int run = 1; run <= NUM_RUNS; run++) {
        csv << ",Run " << run;
    }

    csv << ",Average Execution Time (seconds)\n";

    cout << fixed << setprecision(9);

    cout << "================================================\n";
    cout << "     MANDELBROT - SEQUENTIAL BENCHMARK\n";
    cout << "================================================\n";

    cout << "Image Resolution: "
        << WIDTH << " x " << HEIGHT << endl;

    cout << "Total Pixels: "
        << TOTAL_PIXELS << endl;

    cout << "Number of Test Cases: "
        << NUM_SIZES << endl;

    cout << "Warm-up Runs per Test: 1" << endl;

    cout << "Measured Runs per Test: "
        << NUM_RUNS << endl;

    cout << "Total Measured Runs: "
        << NUM_SIZES * NUM_RUNS << endl;

    // Step 3: Test different Maximum Iterations
    for (int test = 0; test < NUM_SIZES; test++) {

        const int MAX_ITERATIONS = iterationSizes[test];

        vector<int> pixels(TOTAL_PIXELS);

        double totalTime = 0.0;

        double runTimes[NUM_RUNS];

        cout << "\n------------------------------------------------\n";

        cout << "Test " << test + 1
            << " / " << NUM_SIZES << endl;

        cout << "Image Resolution: "
            << WIDTH << " x " << HEIGHT << endl;

        cout << "Maximum Iterations: "
            << MAX_ITERATIONS << endl;

        // Step 4: Warm-up Run (not measured)
        cout << "\nPerforming Warm-up Run..." << endl;

        calculateMandelbrot(
            pixels,
            WIDTH,
            HEIGHT,
            MAX_ITERATIONS
        );

        cout << "Warm-up Completed." << endl;

        // Step 5: Perform 10 measured runs
        for (int run = 0; run < NUM_RUNS; run++) {

            auto start = chrono::steady_clock::now();

            calculateMandelbrot(
                pixels,
                WIDTH,
                HEIGHT,
                MAX_ITERATIONS
            );

            auto end = chrono::steady_clock::now();

            chrono::duration<double> elapsed = end - start;

            runTimes[run] = elapsed.count();

            totalTime += elapsed.count();

            cout << "Run " << run + 1
                << " Execution Time: "
                << runTimes[run]
                << " seconds" << endl;
        }

        // Step 6: Calculate average execution time
        double averageTime = totalTime / NUM_RUNS;

        averageTimes[test] = averageTime;

        cout << "\nAverage Execution Time: "
            << averageTime << " seconds" << endl;

        // Step 7: Save benchmark results to CSV
        csv << WIDTH << ","
            << HEIGHT << ","
            << TOTAL_PIXELS << ","
            << MAX_ITERATIONS;

        for (int run = 0; run < NUM_RUNS; run++) {
            csv << "," << runTimes[run];
        }

        csv << "," << averageTime << "\n";

        // Step 8: Generate PNG image
        // PNG generation is NOT included in execution time
        string filename =
            "mandelbrot_iter_" +
            to_string(MAX_ITERATIONS) + ".png";

        generatePNG(
            pixels,
            WIDTH,
            HEIGHT,
            MAX_ITERATIONS,
            filename
        );

        cout << "Image saved: " << filename << endl;
    }

    // Step 9: Close CSV file
    csv.close();

    if (!csv) {
        cout << "Error: Failed to write CSV file." << endl;
        return 1;
    }

    // Step 10: Display final performance summary
    cout << "\n\n";

    cout << "===============================================================\n";
    cout << "       SEQUENTIAL BENCHMARK - PERFORMANCE SUMMARY\n";
    cout << "===============================================================\n";

    cout << left
        << setw(10) << "Test"
        << setw(20) << "Max Iterations"
        << setw(20) << "Total Pixels"
        << setw(25) << "Average Time (sec)"
        << endl;

    cout << "---------------------------------------------------------------\n";

    for (int test = 0; test < NUM_SIZES; test++) {

        cout << left
            << setw(10) << test + 1
            << setw(20) << iterationSizes[test]
            << setw(20) << TOTAL_PIXELS
            << setw(25) << averageTimes[test]
            << endl;
    }

    cout << "===============================================================\n";

    cout << "\nBenchmark Completed Successfully!\n";

    cout << "Total Test Cases: " << NUM_SIZES << endl;

    cout << "Warm-up Runs: " << NUM_SIZES << endl;

    cout << "Total Measured Runs: "
        << NUM_SIZES * NUM_RUNS << endl;

    cout << "Results saved: sequential_results.csv" << endl;

    return 0;
}
