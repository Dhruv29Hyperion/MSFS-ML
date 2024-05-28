#include "pixel.h" 

void multiplyMatrixVector(double matrix[3][3], double* result, double x, double y, double z) {
    result[0] = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
    result[1] = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
    result[2] = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;
}

void multiplyMatrices(double matrix1[3][3], double matrix2[3][3], double result[3][3]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

void calculatePixelCoordinates(double dx, double dy, double dz, double camera_yaw, double camera_pitch, double camera_roll, double horizontal_fov, double vertical_fov, int imageWidth, int imageHeight, double& pixelX, double& pixelY) {


    //Convert into radians
    camera_yaw = -camera_yaw * (M_PI / 180);
    /*std :: cout << "camera_yaw: " << camera_yaw << std::endl;*/
    camera_pitch = -camera_pitch * (M_PI / 180);
    camera_roll = -camera_roll * (M_PI / 180);
    horizontal_fov = horizontal_fov * (M_PI / 180);
    vertical_fov = -vertical_fov * (M_PI / 180);


    // Inverse camera rotation matrices
    double camera_yaw_inv_mat[3][3] = {
        {cos(camera_yaw), 0, sin(camera_yaw)},
        {0, 1, 0},
        {-sin(camera_yaw), 0, cos(camera_yaw)}
    };

    double camera_pitch_inv_mat[3][3] = {
        {1, 0, 0},
        {0, cos(camera_pitch), -sin(camera_pitch)},
        {0, sin(camera_pitch), cos(camera_pitch)}
    };

    double camera_roll_inv_mat[3][3] = {
        {cos(camera_roll), -sin(camera_roll), 0},
        {sin(camera_roll), cos(camera_roll), 0},
        {0, 0, 1}
    };

    // Compute plane2Camera matrix
    double plane2Camera[3][3];
    double intermediateMatrix[3][3];

    multiplyMatrices(camera_pitch_inv_mat, camera_yaw_inv_mat, intermediateMatrix);
    multiplyMatrices(intermediateMatrix, camera_roll_inv_mat, plane2Camera);

    // Compute position vector
    double position[3] = { dx, dy, dz };

    // Transform position vector using plane2Camera matrix
    double pos_pixel[3];
    multiplyMatrixVector(plane2Camera, pos_pixel, position[0], position[1], position[2]);

    // Calculate pixel coordinates

    pixelX = (960 * pos_pixel[0]) / (pos_pixel[2] * std::tan((horizontal_fov / 2))) + 960;
    double aspectRatio = (double)imageWidth / (double)imageHeight;
    /*double vertical_fov = 2.0 * std::atan(std::tan(horizontal_fov / 2.0) / aspectRatio);*/
    pixelY = ((540 * pos_pixel[1]) / (pos_pixel[2] * std::tan((vertical_fov / 2)))) + 540;

    double fx = (imageWidth / 2) / std::tan((horizontal_fov / 2));       //fx*pos_pixel[0] / pos_pixel[2]
    double fy = (imageHeight / 2) / std::tan((vertical_fov / 2));        //fy*pos_pixel[1] / pos_pixel[2]
}

void createBoundingBox(double dx, double dy, double dz,
    double wingSpan, double height, double fuselage,
    double cameraPitch, double cameraYaw, double cameraRoll, double horizontalFov, double verticalFov,
    int imageWidth, int imageHeight,
    double &topLeftX, double &topLeftY, double &bottomRightX, double &bottomRightY) {

    // Calculate half of the length, breadth, and height
    double halfLength = fuselage / 2;   // in the direction of dz
    double halfBreadth = wingSpan / 2;   // in the direction of dx
    double halfHeight = height / 2;     // in the direction of dy

    // Calculate the coordinates of the bounding box
    double objectLocal[8][3] = {
        {dx - halfBreadth, dy + halfHeight, dz - halfLength},   // top left front
        {dx + halfBreadth, dy + halfHeight, dz - halfLength},   // top right front
        {dx + halfBreadth, dy + halfHeight, dz + halfLength},   // top right back
        {dx - halfBreadth, dy + halfHeight, dz + halfLength},   // top left back
        {dx - halfBreadth, dy - halfHeight, dz - halfLength},   // bottom left front
        {dx + halfBreadth, dy - halfHeight, dz - halfLength},   // bottom right front
        {dx + halfBreadth, dy - halfHeight, dz + halfLength},   // bottom right back
        {dx - halfBreadth, dy - halfHeight, dz + halfLength}    // bottom left back
    };

    // Get the pixel coordinate of each point

    double pixelCoordinates[8][2];
    for (int i = 0; i < 8; i++) {
        calculatePixelCoordinates(objectLocal[i][0], objectLocal[i][1], objectLocal[i][2], cameraYaw, cameraPitch, cameraRoll, horizontalFov, verticalFov, imageWidth, imageHeight, pixelCoordinates[i][0], pixelCoordinates[i][1]);
    }

    // Find the four corners of the bounding box (min,min) (min,max) (max,min) (max,max)
    double minX = pixelCoordinates[0][0];
    double maxX = pixelCoordinates[0][0];
    double minY = pixelCoordinates[0][1];
    double maxY = pixelCoordinates[0][1];

    for (int i = 1; i < 8; i++) {
        if (pixelCoordinates[i][0] < minX) {
            minX = pixelCoordinates[i][0];
        }
        if (pixelCoordinates[i][0] > maxX) {
            maxX = pixelCoordinates[i][0];
        }
        if (pixelCoordinates[i][1] < minY) {
            minY = pixelCoordinates[i][1];
        }
        if (pixelCoordinates[i][1] > maxY) {
            maxY = pixelCoordinates[i][1];
        }
    }

    // Clip the bounding box to fit within the image boundaries
    minX = std::max(minX, 0.0);
    maxX = std::min(maxX, static_cast<double>(imageWidth));
    minY = std::max(minY, 0.0);
    maxY = std::min(maxY, static_cast<double>(imageHeight));

    // Check if the bounding box is still valid (not empty)
    if (minX < maxX && minY < maxY) {
        // Get the pixel coordinates of the four corners
        
        topLeftX = minX;
        topLeftY = minY;

        bottomRightX = maxX;
        bottomRightY = maxY;
    }
    else {
        // Empty bounding box (object is outside the image)
        // Set all coordinates to -1 to indicate invalid bounding box
        
        topLeftX = -1;
        topLeftY = -1;

        bottomRightX = -1;
        bottomRightY = -1;
    }
}

 
