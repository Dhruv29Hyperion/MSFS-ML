# Cumulative ReadMe.md

## Description

This project comprises four C++ scripts that work together to capture and save screenshots of a flight simulation environment at specific camera locations. The scripts utilize the provided "pixel.h" header and integrate with Microsoft Flight Simulator using SimConnect to manipulate the camera position and capture screenshots. The goal is to capture images of the simulation environment with a red dot representing the target location and an overlaid bounding box around an intruder aircraft.

## Project Components

The project consists of the following four C++ scripts:

### 1. `pixel.h`

This header file contains function prototypes related to pixel calculations, camera movements, and bounding box creation. It includes standard C++ libraries for string manipulation and mathematical operations, as well as external libraries like OpenCV, Gdiplus, and SimConnect.

### 2. `pixel.cpp`

The `pixel.cpp` script implements the functions defined in the `pixel.h` header. It includes functions to calculate pixel coordinates, create bounding boxes, and move the camera to specific locations. These functions are essential for positioning the camera correctly and capturing the required screenshots.

### 3. `main.cpp`

The `main.cpp` script serves as the main program for this project. It utilizes SimConnect to communicate with Microsoft Flight Simulator and retrieves the aircraft's position and orientation data. The script then moves the camera to specific locations, captures screenshots, overlays bounding boxes around intruder aircraft, and stores relevant data in text files.

### 4. `helpers.cpp`

The `helpers.cpp` script contains helper functions to support the main functionalities in `main.cpp`. It includes functions to draw a red dot on the screenshot, convert bounding box coordinates to YOLO format, and limit the index when iterating through the list of camera positions.

## Dependencies

Before running the code, ensure that the following libraries are available and correctly linked during compilation:

- Gdiplus library: Required for capturing and saving screenshots in PNG format.
- SimConnect library: Required for interfacing with Microsoft Flight Simulator.
- OpenCV library: Required for image manipulation and drawing operations.

## Usage

1. Compile and run the `main.cpp` script to initialize SimConnect and begin capturing screenshots.

2. The `readSpawnDataFromFile()` function reads the spawn data from the file "D:/MSFS DATA/Relative_spawn.txt" to get the list of camera positions.

3. The `moveToLocation()` function moves the aircraft to the next location, and the `calculatePixelCoordinates()` function calculates the pixel coordinates of the intruder aircraft in the image.

4. The screenshots are captured using the `TakeScreenShot()` function, and the YOLO format bounding box coordinates are stored in "(R)PixelData.txt".

5. The program continues to iterate through the list of camera positions until the end of the list is reached.

6. The program outputs relevant aircraft position and orientation data to "(R)Simdata.txt".

## Important Note

- The code is designed to run with Microsoft Flight Simulator and may not function correctly with other flight simulation software.
- Ensure that the required image and SimConnect libraries are linked correctly during the compilation process.
- The program assumes that the provided "pixel.h" header and any other necessary files are available in the compilation environment.

Please make sure to run the code in an appropriate environment with Microsoft Flight Simulator and the necessary libraries to observe the intended functionality.
