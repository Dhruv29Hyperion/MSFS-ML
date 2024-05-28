import random
import math
import numpy as np
from geographiclib.geodesic import Geodesic
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import pytz
from datetime import datetime, time
from timezonefinder import TimezoneFinder

# Get the current date and time
now = datetime.now()

horizontal_fov = 72.5  # degrees
vertical_fov = 44.5  # degrees
image_width = 1920  # pixels
image_height = 1080  # pixels
iterations = 100  # number of iterations

# Write the data to a file in the Collective(C++ Visual Studio project) folder
file_path = "D:/MSFS DATA/Relative_spawn.txt"

with open(file_path, "w") as file:
    file.write("New Spawn Data [" + str(now.day) + "/" + str(now.month) + "/" + str(now.year) + " " + str(now.hour) +
               ":" + str(now.minute) + ":" + str(now.second) + "]\n")
    file.write("\n")


def generate_aircraft_params():
    # Generate random camera parameters
    lat = random.uniform(-90, 90)
    lon = random.uniform(-180, 180)
    alt = random.uniform(0, 5000)
    pitch = random.uniform(-90, 90)
    bank = random.uniform(-180, 180)
    roll = random.uniform(-180, 180)

    return lat, lon, alt, pitch, bank, roll


def is_time_between(start_time, end_time, current_time):
    # Check if the current_time is between start_time and end_time
    if start_time <= end_time:
        return start_time <= current_time <= end_time
    else:
        return start_time <= current_time or current_time <= end_time


for i in range(iterations):

    while True:
        aircraft_params = generate_aircraft_params()

        # Get the current time at the specified latitude and longitude
        timezone_str = TimezoneFinder().timezone_at(lng=aircraft_params[1], lat=aircraft_params[0])

        if timezone_str:
            timezone = pytz.timezone(timezone_str)
            current_time = datetime.now(timezone).time()

            # Check if the current time is between 6 am to 6 pm
            if is_time_between(time(6, 0), time(18, 0), current_time):
                break

    # Pixel checks
    pixel_x = -1
    pixel_y = -1
    data = 0

    # Generate random delta values based on the frustum size and the aircraft's position
    New_Pitch = random.uniform(-90, 90)
    New_Bank = random.uniform(-180, 180)
    New_Roll = random.uniform(-180, 180)

    while pixel_x < 0 or pixel_x > image_width or pixel_y < 0 or pixel_y > image_height:

        string1 = "lat: " + str(aircraft_params[0]) + ", lon: " + str(aircraft_params[1]) + ", alt: " + \
                  str(aircraft_params[2]) + ", pitch: " + str(aircraft_params[3]) + ", bank: " + str(
            aircraft_params[4]) + \
                  ", roll: " + str(aircraft_params[5]) + "\n"

        dy = random.uniform(-1000, 1000)
        dx = random.uniform(-1000, 1000)
        dz = random.uniform(-1000, 1000)

        string2 = "dx: " + str(dx) + ", dy: " + str(dy) + ",dz: " + str(dz) + "\n"

        # Calculate the new lat and lon values
        original_coords = (aircraft_params[0], aircraft_params[1])
        geod = Geodesic.WGS84

        # Calculate the distance and bearing in meters for north and east directions
        distance_north = geod.Direct(aircraft_params[0], aircraft_params[1], 0, dz)
        distance_east = geod.Direct(aircraft_params[0], aircraft_params[1], 90, dx)

        # Calculate the new coordinates
        new_coords = geod.Direct(aircraft_params[0], aircraft_params[1], distance_north['azi2'], distance_north['s12']
                                 + distance_east['s12'])

        New_Lat, New_Lon = new_coords['lat2'], new_coords['lon2']
        New_Alt = aircraft_params[2] + dy

        string4 = "Camera Lat: " + str(New_Lat) + ", Camera lon: " + str(New_Lon) + ", Camera alt: " + \
                  str(New_Alt) + ", Camera pitch: " + str(New_Pitch) + ", Camera bank: " + str(New_Bank) + \
                  ", Camera roll: " + str(New_Roll) + "\n"

        # Convert pitch, bank, and roll angles to radians
        pitch_rad = -math.radians(New_Pitch)
        bank_rad = -math.radians(New_Bank)
        roll_rad = -math.radians(New_Roll)

        # Create rotation matrices
        pitch_matrix = np.array([[1, 0, 0],
                                 [0, math.cos(pitch_rad), -math.sin(pitch_rad)],
                                 [0, math.sin(pitch_rad), math.cos(pitch_rad)]])

        bank_matrix = np.array([[math.cos(bank_rad), 0, math.sin(bank_rad)],
                                [0, 1, 0],
                                [-math.sin(bank_rad), 0, math.cos(bank_rad)]])

        roll_matrix = np.array([[math.cos(roll_rad), -math.sin(roll_rad), 0],
                                [math.sin(roll_rad), math.cos(roll_rad), 0],
                                [0, 0, 1]])

        # Create a delta vector
        object_coordinates = np.array([dx, dy, dz])

        camera_matrix = pitch_matrix @ bank_matrix @ roll_matrix
        # Rotate the delta vector
        rotated_delta = camera_matrix @ object_coordinates

        # Extract the rotated dx and dy values
        rotated_dx = rotated_delta[0]
        rotated_dy = rotated_delta[1]
        rotated_dz = rotated_delta[2]

        string3 = "rotated dx: " + str(rotated_dx) + ", rotated dy: " + str(rotated_dy) + \
                  ", rotated dz: " + str(rotated_dz) + "\n"

        frustum_height = 2 * rotated_dz * math.tan(-math.radians(vertical_fov / 2))
        frustum_width = 2 * rotated_dz * math.tan(math.radians(horizontal_fov / 2))
        image_distance = rotated_dz

        pixel_x = (image_width / 2 * rotated_dx) / (
                rotated_dz * math.tan(math.radians(horizontal_fov / 2))) + image_width / 2
        pixel_y = (image_height / 2 * rotated_dy) / (
                rotated_dz * math.tan(math.radians(-vertical_fov / 2))) + image_height / 2

        string5 = "pixel_x: " + str(pixel_x) + ", pixel_y: " + str(pixel_y) + "\n"

        data = string1 + string2 + string4 + string5

    camera_position = np.array([0, 0, 0])

    # Create the 3D plot for the current scenario
    # fig = plt.figure()
    # ax = fig.add_subplot(111, projection='3d')
    #
    # # Plot camera position
    # ax.scatter(*camera_position, color='red', label='Camera')
    #
    # # Plot frustum
    # frustum_vertices = [
    #     [frustum_width / 2, frustum_height / 2, image_distance],
    #     [frustum_width / 2, -frustum_height / 2, image_distance],
    #     [-frustum_width / 2, -frustum_height / 2, image_distance],
    #     [-frustum_width / 2, frustum_height / 2, image_distance],
    #     camera_position  # Camera position as the apex of the frustum
    # ]
    # frustum_faces = [[frustum_vertices[0], frustum_vertices[1], frustum_vertices[4]],
    #                  [frustum_vertices[1], frustum_vertices[2], frustum_vertices[4]],
    #                  [frustum_vertices[2], frustum_vertices[3], frustum_vertices[4]],
    #                  [frustum_vertices[3], frustum_vertices[0], frustum_vertices[4]]]
    # ax.add_collection3d(
    #     Poly3DCollection(frustum_faces, alpha=0.2, linewidths=1, edgecolors='cyan', facecolors='cyan'))
    #
    # # Plot the object
    # ax.scatter(*rotated_delta, color='green', label='Object')
    #
    # # Set axis labels
    # ax.set_xlabel('X')
    # ax.set_ylabel('Y')
    # ax.set_zlabel('Z')
    #
    # # Set axis limits based on the frustum dimensions and object coordinates
    # max_dim = max(frustum_width, frustum_height, image_distance)
    # ax.set_xlim([-1000, 1000])
    # ax.set_ylim([-1000, 1000])
    # ax.set_zlim([-1000, 1000])
    #
    # # Add legend
    # ax.legend()
    #
    # # Show the plot for the current scenario
    # plt.title(f"Scenario {i + 1}")  # Add a title with the scenario number
    # plt.show()

    with open(file_path, "a") as file:
        file.write(data)
        file.write("\n") : I still get a pixel coordinate sometimes even though the camera is looking completely opposite
