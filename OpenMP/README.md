# Mandelbrot OpenMP Implementation

This is an individual OpenMP implementation intended to sit beside the group's existing `Sequential` project. The OpenMP version keeps the same Mandelbrot formula, coordinate mapping, image settings, maximum iteration count, and HSV-to-RGB colour conversion as `Sequential/Source.cpp`.

## Files

- `Source.cpp` — OpenMP implementation.
- `OpenMP.vcxproj` — Visual Studio C++ project with OpenMP enabled.
- `OpenMP.vcxproj.filters` — Solution Explorer file grouping.
- `Setup_Copy_Stb.ps1` — copies the existing `Sequential/stb.h` into this project folder.

The image writer header is intentionally reused from the provided repository rather than replaced with a different dependency.

## Setup in your repository

1. Extract the `OpenMP` folder from this package into the root of your cloned `Mandelbrot_Set_Generation` repository. It should be beside the existing `Sequential` folder.
2. Open a PowerShell terminal in the extracted `OpenMP` folder and run:

   ```powershell
   .\Setup_Copy_Stb.ps1
   ```

   This copies `..\Sequential\stb.h` to `OpenMP\stb.h`. It does not modify the sequential source file.
3. Open `OpenMP.vcxproj` directly, or add it to the existing `Mandelbrot_Set_Generation.sln` using **Solution > Add > Existing Project**.
4. Select **Release** and **x64** before measuring performance. Debug builds are not appropriate for the main timing comparison.
5. Build and run the project.

## Run with different thread counts

The program accepts an optional positive thread count as its first argument. Examples:

```powershell
.\x64\Release\OpenMP.exe 1
.\x64\Release\OpenMP.exe 2
.\x64\Release\OpenMP.exe 4
.\x64\Release\OpenMP.exe 8
```

The executable path may differ depending on your Visual Studio output-directory configuration. You can also set the command-line argument in **Project Properties > Debugging > Command Arguments**.

## Important comparison rules

- Keep the sequential and OpenMP parameters identical: 1000 x 1000 pixels and 1000 maximum iterations in the current repository baseline.
- The timer covers the pixel iteration loop only; RGB conversion and PNG writing happen after timing, matching the sequential implementation's measurement boundary.
- The output filename is `mandelbrot_openmp.png`, so it will not overwrite `mandelbrot_sequential.png`.
- Verify correctness by comparing the resulting images and, preferably, the iteration-count array or a pixel-level comparison before reporting final results.
- Record actual timings from your own machine. Do not use example or fabricated performance values.

## OpenMP directive used

```cpp
#pragma omp parallel for schedule(static)
```

The outer `y` loop is parallelised. Each iteration writes to a distinct row in `pixels`, while each pixel's mathematical calculation remains unchanged.
