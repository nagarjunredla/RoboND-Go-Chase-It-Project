#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{ 
  // Request wheel velocities
  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;
  
  // Call the commandProbot service and pass the requested wheel velocities
  if (!client.call(srv))
    ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel    = 255;
    int pixel_location = 0;
    int num_pixels = 0;
    bool ball_found    = false;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    for(int i = 0; i < img.step * img.height; i = i + 3)
    {
        if(img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel)
        {
            ball_found     = true;
            pixel_location += (i % (img.width * 3)) / 3;
            num_pixels+=1;
        }
    }

    int middle_window = 300;
    int left_window   = (img.width - middle_window) / 2;


  
    if (ball_found) {
        int mean_pixel_location = pixel_location / num_pixels;
        if (mean_pixel_location < left_window) {
            // Drive left
            drive_robot(0.25, 0.5);
        }
        else if (mean_pixel_location > (left_window + middle_window)) {
            // Drive right
            drive_robot(0.25, -0.5);
        }
        else {
            // Drive forward
            drive_robot(0.5, 0.0);
        }
    }
    else {
        // request to stop
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}