#define _CRT_SECURE_NO_WARNINGS
#include "pixel.h"


#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "SimConnect.lib")

using namespace Gdiplus;
#define M_PI 3.14159265358979323846         // Added definition of pi

int quit = 0;
HANDLE hSimConnect = NULL;
std::ofstream outputFile, pixelFile;

// Check and indexing variables
int counter = 0;
int check = 0;
int index;

// Image variables
int screenWidth = 1920;
int screenHeight = 1080;
int No_of_Pictures;
double camera_horizontal_fov;
double camera_vertical_fov;

// Aircraft variables
double fuselageLength;
double wingSpan;
double height;

int GetEncoderClsid(const WCHAR* format, CLSID* pCLsid);
void DrawRedDot(cv::Mat& image, int pixelX, int pixelY);
void TakeScreenShot(double& pixelX, double& pixelY, double distance);
std::string convertToYoloFormat(int xmin, int xmax, int ymin, int ymax, int imageWidth, int imageHeight, int classIndex);


enum DATA_DEFINE_ID {
    DEFINITION_2, // Added definition for 6DoF
};

enum DATA_REQUEST_ID {
    REQUEST_2, // Added request for 6DoF
};


struct AircraftData {
    double latitude;
    double longitude;
    double altitude;
    double pitch;
    double bank;
    double heading;
};

struct ACFT_POSITION
{
    double lat;
    double lon;
    double alt;
};

struct SpawnData {
    double lat, lon, alt, pitch, bank, roll;
    double intruder_lat, intruder_lon, intruder_alt, intruder_pitch, intruder_bank, intruder_roll;
    double pixel_x, pixel_y;
    double dx, dy, dz;
};
// Data Storage
std::vector<SpawnData> spawn_data_list;

// Image CLSID - this is for the screenshot
int GetEncoderClsid(const WCHAR* format, CLSID* pCLsid) {
    // Get the CLSID of the image format from the registry
    UINT num = 0;
    UINT size = 0;

    ImageCodecInfo* pImage = NULL;

    GetImageEncodersSize(&num, &size);   // Get the size of the image codec

    if (size == 0)
        return -1;

    pImage = (ImageCodecInfo*)(malloc(size));   // Allocate the memory for the image codec

    if (pImage == NULL)
        return -1;

    GetImageEncoders(num, size, pImage);   // Get the image codec

    for (UINT j = 0; j < num; ++j) {
        // Search for the image format based on the passed format parameter
        if (wcscmp(pImage[j].MimeType, format) == 0) {
            *pCLsid = pImage[j].Clsid;
            free(pImage);
            return j;
        }
    }

    free(pImage);

    return -1;
}

// Draw a red dot at the specified pixel coordinate
void DrawRedDot(cv::Mat& image, int pixelX, int pixelY)
{
    cv::Scalar red(0, 0, 255); // Red color in BGR format

    // Draw a circle at the specified pixel coordinate
    cv::circle(image, cv::Point(pixelX, pixelY), 2, red, 1);
}

// Take a screenshot with a red dot at the specified pixel coordinate
void TakeScreenShot(double& pixelX, double& pixelY, double distance) {
    GdiplusStartupInput gdip;
    ULONG_PTR gdiToken;
    GdiplusStartup(&gdiToken, &gdip, NULL);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    SelectObject(hdcMemDC, hbmScreen);
    BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    CLSID encoderID;
    Status stat;

    GetEncoderClsid(L"image/png", &encoderID);

    Bitmap* bmp = new Bitmap(hbmScreen, (HPALETTE)0);

    std::string filename = std::to_string(counter) + ".png";
    wchar_t wFilename[256];
    mbstowcs(wFilename, filename.c_str(), 256);

    // Convert the HBITMAP to an OpenCV Mat
    cv::Mat cvImage;
    cvImage.create(screenHeight, screenWidth, CV_8UC4);
    GetBitmapBits(hbmScreen, cvImage.step * cvImage.rows, cvImage.data);

    //// Draw a red dot on the image at the specified pixel coordinate
     DrawRedDot(cvImage, static_cast<int>(pixelX), static_cast<int>(pixelY));

     // Calculate the bounding box coordinates of the intruder aircraft in the image
      double topLeftX, topLeftY, bottomRightX, bottomRightY;
     createBoundingBox(spawn_data_list[index].dx, spawn_data_list[index].dy, spawn_data_list[index].dz,
         wingSpan, height, fuselageLength,
         spawn_data_list[index].intruder_pitch, spawn_data_list[index].intruder_bank, spawn_data_list[index].intruder_roll,
         camera_horizontal_fov, camera_vertical_fov, screenWidth, screenHeight, topLeftX, topLeftY, bottomRightX, bottomRightY);

    std::string YOLO = convertToYoloFormat(topLeftX, topLeftY, bottomRightX, bottomRightY, screenWidth, screenHeight, counter);

    pixelFile << YOLO << "\n";

    // Draw a red rectangle on the image using the bounding box coordinates
    cv::rectangle(cvImage, cv::Point(topLeftX, topLeftY), cv::Point(bottomRightX, bottomRightY), cv::Scalar(0, 0, 255), 2);

    // Save the modified image back to the HBITMAP
    SetBitmapBits(hbmScreen, cvImage.step * cvImage.rows, cvImage.data);

    cv::imwrite("D:/MSFS DATA/Images/Relative" + std::to_string(counter) + ".png", cvImage);

    delete bmp;

    GdiplusShutdown(gdiToken);
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}

// Print the data to the console and store it in a text file
void printAndStoreData(const std::string& data) {
    if (check != counter) {
        std::cout << std::to_string(counter) << "\n" << data << "\n" << std::endl;
        outputFile << std::to_string(counter) << "\n" << data << "\n" << std::endl;
        check = counter;
    }
}

// Retrieve the aircraft position and orientation data
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {

    switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
        switch (pObjData->dwRequestID) {
        case REQUEST_2: {
            AircraftData* pAircraftData = (AircraftData*)&pObjData->dwData;
            std::string aircraftData = "Aircraft - Latitude: " + std::to_string(pAircraftData->latitude) + "\n" +
                "Aircraft - Longitude: " + std::to_string(pAircraftData->longitude) + "\n" +
                "Aircraft - Altitude: " + std::to_string(pAircraftData->altitude) + "\n" +
                "Aircraft - Pitch: " + std::to_string(pAircraftData->pitch) + " degrees\n" +
                "Aircraft - Bank: " + std::to_string(pAircraftData->bank) + " degrees\n" +
                "Aircraft - Heading: " + std::to_string(pAircraftData->heading) + " degrees";

            printAndStoreData(aircraftData);

            break;
        }
        }
        break;
    }
    case SIMCONNECT_RECV_ID_QUIT: {
        quit = 1;
        break;
    }
    default:
        break;
    }
}

// Set up the SimConnect client
bool initSimConnect() {
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Camera Variable Demo", NULL, 0, NULL, 0))) {
        std::cout << "Connected to Microsoft Flight Simulator\n";

        outputFile.open("D:/MSFS DATA/(R)Simdata.txt", std::ios::app);
        if (!outputFile) {
            std::cout << "Failed to open file for writing\n";
            return false;
        }
        pixelFile.open("D:/MSFS DATA/(R)PixelData.txt", std :: ios :: app);
        if (!pixelFile) {
            std::cout << "Failed to open PixelData file for writing\n";
        }

        // Set up the data definition, but do not yet do anything with it
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE LATITUDE", "Degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE LONGITUDE", "Degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE ALTITUDE", "feet");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE PITCH DEGREES", "Degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE BANK DEGREES", "Degrees");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_2, "PLANE HEADING DEGREES TRUE", "Degrees");

        // Request an event when the simulation starts
        hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_2, DEFINITION_2, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND);

        return true;
    }
    return false;
}

// Close SimConnect connection
void cleanupSimConnect() {
    if (hSimConnect) {
        if (outputFile.is_open())
            outputFile.close();
        SimConnect_Close(hSimConnect);
    }
}

// Convert coordinates to YOLO format
std::string convertToYoloFormat(int xmin, int xmax, int ymin, int ymax, int imageWidth, int imageHeight, int classIndex) {
    float centerX = static_cast<float>(xmin + (xmax - xmin) / 2) / imageWidth;
    float centerY = static_cast<float>(ymin + (ymax - ymin) / 2) / imageHeight;
    float width = static_cast<float>(xmax - xmin) / imageWidth;
    float height = static_cast<float>(ymax - ymin) / imageHeight;

    std::ostringstream oss;
    oss << classIndex << " " << centerX << " " << centerY << " " << width << " " << height;
    return oss.str();
}

// Limit index to size of list
void limit(int& index, std::vector<SpawnData> list) {
    if (index <= list.size() - 1) {
        index++;
    }
    else {
        std::cout << "End of list reached" << std::endl;
    }
}


// Read spawn data from file
std::vector<SpawnData> readSpawnDataFromFile(const std::string& file_path) {
    std::ifstream file(file_path);
    std::vector<SpawnData> spawn_data_list;

    if (file.is_open()) {
        std::string line;
        SpawnData spawn_data;

        while (std::getline(file, line)) {
            if (line.find("lat: ") != std::string::npos) {
                sscanf(line.c_str(), "lat: %lf, lon: %lf, alt: %lf, pitch: %lf, bank: %lf, roll: %lf",
                    &spawn_data.lat, &spawn_data.lon, &spawn_data.alt, &spawn_data.pitch, &spawn_data.bank, &spawn_data.roll);
            }
            else if (line.find("Camera Lat: ") != std::string::npos) {
                sscanf(line.c_str(), "Camera Lat: %lf, Camera lon: %lf, Camera alt: %lf, Camera pitch: %lf, Camera bank: %lf, Camera roll: %lf",
                    &spawn_data.intruder_lat, &spawn_data.intruder_lon, &spawn_data.intruder_alt, &spawn_data.intruder_pitch, &spawn_data.intruder_bank, &spawn_data.intruder_roll);
            }
            else if (line.find("pixel_x: ") != std::string::npos) {
                sscanf(line.c_str(), "pixel_x: %lf, pixel_y: %lf", &spawn_data.pixel_x, &spawn_data.pixel_y);
                spawn_data_list.push_back(spawn_data);
            }
            else if (line.find("dx: ") != std::string::npos) {
                sscanf(line.c_str(), "dx: %lf, dy: %lf, dz: %lf", &spawn_data.dx, &spawn_data.dy, &spawn_data.dz);
            }
        }

        file.close();
    }
    else {
        std::cout << "Unable to open file: " << file_path << std::endl;
    }

    return spawn_data_list;
}

int main() {
    if (!initSimConnect()) {
        std::cout << "Failed to connect to Microsoft Flight Simulator\n";
        return 1;
    }
    
    index = 0;

    // Read the spawn data from file
    std::string file_path = "D:/MSFS DATA/Relative_spawn.txt";
    spawn_data_list = readSpawnDataFromFile(file_path);

    // Aircraft dimensions
    fuselageLength = 10;
    wingSpan = 9.5;
    height = 7;

    // Camera dimensions
    camera_horizontal_fov = 72.5f;
    camera_vertical_fov = 44.5;
    No_of_Pictures = 1;
    double pixelX, pixelY;

    while (quit == 0) {

        // Move aircraft to the next location
        moveToLocation(spawn_data_list[index].lat, spawn_data_list[index].lon, spawn_data_list[index].alt, spawn_data_list[index].pitch, spawn_data_list[index].bank, spawn_data_list[index].roll);

        // Show the presence of the intruder aircraft
        HRESULT hr = SimConnect_CameraSetRelative6DOF(hSimConnect, 0, 0, -10, 0, 0, 0);
        Sleep(5000);

        // Move the camera to relative position
        hr = SimConnect_CameraSetRelative6DOF(hSimConnect, spawn_data_list[index].dx, spawn_data_list[index].dy, spawn_data_list[index].dz, spawn_data_list[index].intruder_pitch, spawn_data_list[index].intruder_roll, spawn_data_list[index].intruder_bank);

        // Calculate the pixel coordinates of the intruder aircraft in the image
        double pixelX, pixelY;
        calculatePixelCoordinates(spawn_data_list[index].dx, spawn_data_list[index].dy, spawn_data_list[index].dz, spawn_data_list[index].intruder_bank, spawn_data_list[index].intruder_pitch, spawn_data_list[index].intruder_roll, camera_horizontal_fov, camera_vertical_fov, screenWidth, screenHeight, pixelX, pixelY);

        SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);

        Sleep(16000);

        //  Take the number of pictures specified by the user
        do{
            counter++;
            TakeScreenShot(spawn_data_list[index].pixel_x, spawn_data_list[index].pixel_y, spawn_data_list[index].dz);
        } while (counter % No_of_Pictures != 0);

        // If the end of the list is reached, quit the program
        if (index == spawn_data_list.size() - 1) {
            quit = 1;
        }

        // Iterate to next data point
        limit(index, spawn_data_list);
        Sleep(0);
     
    }

    cleanupSimConnect();
    return 0;
}
