#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include<cmath>

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

int main() {

    // Step 1: Set image parameters
    const int WIDTH = 1000;
    const int HEIGHT = 1000;
    const int MAX_ITERATIONS = 1000;
	//do one more iteration = 2000 to see the comnparison between sequential and parallel execution time, should be slower for sequential and faster for parallel execution

    // Store iteration count for every pixel
    vector<int> pixels(WIDTH * HEIGHT);

    // Step 2: Start execution timer
    auto start = chrono::high_resolution_clock::now();

    // Step 3: Sequentially calculate every pixel
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

            // Store the result
            pixels[y * WIDTH + x] = iteration;
        }
    }

    // Step 4: Stop execution timer
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end - start;

    // Step 5: Generate image
    vector<unsigned char> rgb(WIDTH * HEIGHT * 3);

    for (int y = 0; y < HEIGHT; y++) {

        for (int x = 0; x < WIDTH; x++) {

            int iteration = pixels[y * WIDTH + x];

            int index = (y * WIDTH + x) * 3;

            if (iteration == MAX_ITERATIONS) {

                // Points inside Mandelbrot Set: Black
                rgb[index] = 0;
                rgb[index + 1] = 0;
                rgb[index + 2] = 0;

            }
            else {
                // Outside Mandelbrot Set: Rainbow Colors
                double hue = fmod(iteration * 3.0, 360.0);

                unsigned char red, green, blue;

                HSVtoRGB(hue, 1.0, 1.0, red, green, blue);

                rgb[index] = red;
                rgb[index + 1] = green;
                rgb[index + 2] = blue;
            }
        }
    }

    // Save PNG image
    stbi_write_png(
        "mandelbrot_sequential.png",
        WIDTH,
        HEIGHT,
        3,
        rgb.data(),
        WIDTH * 3
    );

    // Step 6: Display results
    cout << "Mandelbrot Set Generation - Sequential" << endl;

    cout << "Image Width: " << WIDTH << endl;
    cout << "Image Height: " << HEIGHT << endl;
    cout << "Maximum Iterations: " << MAX_ITERATIONS << endl;

    cout << "Total Pixels: " << WIDTH * HEIGHT << endl;

    cout << "Execution Time: "
        << elapsed.count() << " seconds" << endl;

    cout << "Image saved as mandelbrot_sequential.png" << endl;

    return 0;
}