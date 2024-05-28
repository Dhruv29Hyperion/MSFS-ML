#include "pixel.h"

// SimConnect data IDs
enum SimConnectDataIds {
    DataPosition,
};

// Structure to hold position data
struct PositionStruct {
    double latitude;
    double longitude;
    double altitude;
    double pitch;
    double bank;
    double heading;
};

// SimConnect variables
HANDLE HSimConnect = NULL;

void moveToLocation(double latitude, double longitude, double altitude, double pitch, double bank, double heading)
{
    HRESULT hr;

    // Create SimConnect handle
    hr = SimConnect_Open(&HSimConnect, "My Application", NULL, 0, 0, 0);
    if (FAILED(hr))
    {
        std::cout << "Moving failed!" << std::endl;
        return;
    }

    // Set up data definition for position
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE LATITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE LONGITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE PITCH DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE BANK DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(HSimConnect, DataPosition, "PLANE HEADING DEGREES TRUE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);

    // Connect to SimConnect


    // Update the position
    PositionStruct newPosition;
    newPosition.latitude = latitude; // Update with desired latitude
    newPosition.longitude = longitude; // Update with desired longitude
    newPosition.altitude = altitude; // Update with desired altitude
    newPosition.pitch = pitch; // Update with desired pitch
    newPosition.bank = bank; // Update with desired bank
    newPosition.heading = heading; // Update with desired heading

    // Create a data structure to hold the updated position
    SIMCONNECT_DATA_INITPOSITION initPos;
    initPos.Latitude = newPosition.latitude;
    initPos.Longitude = newPosition.longitude;
    initPos.Altitude = newPosition.altitude;
    initPos.Pitch = newPosition.pitch;
    initPos.Bank = newPosition.bank;
    initPos.Heading = newPosition.heading;
    initPos.OnGround = 0;
    initPos.Airspeed = 0;

    // Send position data to SimConnect
    hr = SimConnect_SetDataOnSimObject(HSimConnect, DataPosition, SIMCONNECT_OBJECT_ID_USER, 0, 1, sizeof(newPosition), &newPosition);

    // Close SimConnect connection
    hr = SimConnect_Close(HSimConnect);

    return;
}

//int main(void) {
//    moveToLocation(37.618423, -122.375801, 1000, 0, 0, 0);
//	return 0;
//}

