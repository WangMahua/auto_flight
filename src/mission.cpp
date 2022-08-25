#include "ros/ros.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <iterator>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Vector3.h>
#include <std_msgs/Int8.h>
#include "auto_flight/ncrl_link.h"

#include "mission.h"

int main(int argc ,char **argv){
    ros::init(argc,argv,"mission");
    MISSION mission;
    ros::Rate loop_rate(20);

    while(ros::ok()){
        mission.process();
		loop_rate.sleep();
		ros::spinOnce();
    }
    return 0;
}