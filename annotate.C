#pragma once

#ifndef PIXEL_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define PIXEL_H

#define M_PI 3.14159265358979323846

// Standard C++ libraries
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

//String libraries
#include <tchar.h>
#include <string>

// Math libraries
#include <cmath>

//File libraries
#include <fstream>
#include <sstream>

//Image libraries
#include <gdiplus.h> 
#include <iomanip>

// OpenCV libraries
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// SimConnect libraries
#include <SimConnect.h>

void calculatePixelCoordinates(double dx, double dy, double dz, double camera_yaw, double camera_pitch, double camera_roll, double horizontal_fov, double vertical_fov, int imageWidth, int imageHeight, double& pixelX, double& pixelY);

void createBoundingBox(double dx, double dy, double dz,
    double wingSpan, double height, double fuselage,
    double cameraPitch, double cameraYaw, double cameraRoll, double horizontalFov, double verticalFov,
    int imageWidth, int imageHeight,
    double& topLeftX, double& topLeftY, double& bottomRightX, double& bottomRightY);

void moveToLocation(double latitude, double longitude, double altitude, double pitch, double bank, double heading);

# endif PIXEL_H
