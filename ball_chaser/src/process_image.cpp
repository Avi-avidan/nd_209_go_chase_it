#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO("Moving robot (%1.2f,%1.2f)", lin_x, ang_z);

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_to_target");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    float req_lin_x, req_ang_z;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int ball_index = -1;
    float ball_position = -1.0;

    // Loop through each pixel in the image and check if its equal to the first one
    for (int i = 0; i < img.height*img.step-2; i += 3) {
        if ((img.data[i] == 255) && (img.data[i+1] == 255) && (img.data[i+2] == 255)){
            ball_index = i%img.step;
            ball_position = float(ball_index)/float(img.step);
            break;
        }
    }
    
    //ROS_INFO("img height, width, step (%1.1f, %1.1f, %1.1f)", float(img.height), float(img.width), float(img.step));
    ROS_INFO("white ball identified at index (%1.3f) of image width", float(ball_index));
    //ROS_INFO_STREAM("calc ball pos ; img hight, width, pos: (%1.1f, %1.1f, %1.3f)", float(img.height), float(img.step), ball_position);

    if (ball_position == -1.0) {
        req_lin_x = 0.0;
        req_ang_z = 0.0;
    } else {
        ROS_INFO("white ball identified at (%1.3f) of image width", ball_position);
        if ((ball_position > 0.4) && (ball_position < 0.6)) {
            req_lin_x = 0.8;
            req_ang_z = 0.0;
        } else if (ball_position <= 0.4) {
            req_lin_x = 0.05;
            req_ang_z = 0.1;
        } else if (ball_position >= 0.6) {
            req_lin_x = 0.05;
            req_ang_z = -0.1;
        }
    }
    
    drive_robot(req_lin_x, req_ang_z);

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
