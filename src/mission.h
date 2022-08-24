#ifndef __MISSION__
#define __MISSION__

using namespace std;
#define WAYPOINT_THRESHOLD 0.05f 
#define TAKEOFF_HEIGHT 0.8f
#define LANDING_HEIGHT 0.2f

class MISSION{
private:
	ros::NodeHandle n;
	ros::Publisher mission_pub;
    ros::Subscriber get_px4_data_sub, pos_sub, web_phase_sub, web_waypoint_sub, land_sub;

	auto_flight::ncrl_link px4_data;
    auto_flight::ncrl_link mission_data;

    geometry_msgs::Pose uav_pose;
    geometry_msgs::Vector3 cur_target_pos;

    vector<auto_flight::ncrl_link> mission_list;

    int cur_task = 0;
    int final_task = 0;
    bool land_flag = false;

    bool hover = true;
	bool hover_start_handler = false;
	ros::Time hover_start_time = ros::Time::now();
	ros::Time hover_cluster_time = ros::Time::now();

public:
    MISSION();
    void px4_callback(const auto_flight::ncrl_link::ConstPtr& msg);
	void pos_callback(const geometry_msgs::PoseStamped::ConstPtr&);
	void web_phase_callback(const std_msgs::Int8::ConstPtr&);
	void land_callback(const std_msgs::Int8::ConstPtr&);
	void waypoint_callback(const geometry_msgs::Vector3::ConstPtr&);
    void publisher();
    bool mission_switch();
    void mission_insert(string m, string a, float d1,float d2, float d3);
    void process();
};

MISSION::MISSION(){
    mission_pub = n.advertise<auto_flight::ncrl_link>("pc_to_pixhawk",1);
	get_px4_data_sub = n.subscribe<auto_flight::ncrl_link>("pixhawk_to_pc",1,&MISSION::px4_callback,this);
	pos_sub = n.subscribe<geometry_msgs::PoseStamped>("/vrpn_client_node/MAV1/pose", 10, &MISSION::pos_callback, this); 
	web_phase_sub = n.subscribe<std_msgs::Int8>("/control_phase", 1, &MISSION::web_phase_callback, this); 
    land_sub = n.subscribe<std_msgs::Int8>("/land", 1, &MISSION::land_callback, this); 
	web_waypoint_sub = n.subscribe<geometry_msgs::Vector3>("/flight_destination", 1, &MISSION::waypoint_callback, this); 


    /* init */
    mission_data.mode = "0";
	mission_data.aux_info = "0";
	mission_data.data1 = 0.0;
	mission_data.data2 = 0.0;
	mission_data.data3 = 0.0;	

    /* If you want to add waypoint you can edit here */
    /* mission_insert( string mode, string aux_info, */
    /*      \  float data1,float data2, float data3) */
    /* Please check readme.md           */

    mission_insert("0", "0", 0.0, 0.0, 0.0);
    mission_insert("1", "0", 0.0, 0.0, 0.0);
}

void MISSION::px4_callback(const auto_flight::ncrl_link::ConstPtr& msg){
	// &px4_data = msg;
}

void MISSION::pos_callback(const geometry_msgs::PoseStamped::ConstPtr& msg){
    uav_pose.position.x = msg->pose.position.x;
    uav_pose.position.y = msg->pose.position.y;
    uav_pose.position.z = msg->pose.position.z;
}

void MISSION::web_phase_callback(const std_msgs::Int8::ConstPtr& msg){
    if (msg->data == 1){
        land_flag = true;
    }
}

void MISSION::land_callback(const std_msgs::Int8::ConstPtr& msg){
    

}

void MISSION::waypoint_callback(const geometry_msgs::Vector3::ConstPtr& msg){
    if(msg->x!=mission_list[cur_task].data1 || msg->y!=mission_list[cur_task].data2){
        mission_insert("2", "0", msg->x, msg->y, 0.0);
    }
}

void MISSION::publisher(){
    mission_pub.publish(mission_data);
}

bool MISSION::mission_switch(){
    float cur_pos_x = uav_pose.position.x;
    float cur_pos_y = uav_pose.position.y;
    float cur_pos_z = uav_pose.position.z;

    float cur_target_x = cur_target_pos.x;
    float cur_target_y = cur_target_pos.y;
    float cur_target_z = cur_target_pos.z;
    string mode = mission_list[cur_task].mode;

    if(mode == "1"){
        if ( abs(cur_pos_z-TAKEOFF_HEIGHT) <= WAYPOINT_THRESHOLD){
            return true;
        }else{
            return false;
        }
    }else if(mode == "2"){
        if ( abs(cur_pos_x-cur_target_x) <= WAYPOINT_THRESHOLD && 
                abs(cur_pos_y-cur_target_y) <= WAYPOINT_THRESHOLD){
            cout << "finish" << endl;
            if( hover == true ){ //for hover
                if(hover_start_handler == false){
                    hover_start_time = ros::Time::now();
                    hover_start_handler = true;
                    return false;
                }else{
                    cout << (ros::Time::now()-hover_start_time).toSec() << endl;
                    if((ros::Time::now()-hover_start_time).toSec()>5){
                        hover_start_handler = false;
                        return true;
                    }else{
                        return false;
                    }
                }
            }else{
                hover_start_handler = false;
                return true;
            }
 
        }else{
            return false;
        }
    }else if(mode == "3"){
        if ( cur_pos_z < LANDING_HEIGHT ){
            return true;
        }else{
            return false;
        }
    }else{
        return true;
    }

}

void MISSION::mission_insert(string m, string a, float d1,float d2, float d3){
    auto_flight::ncrl_link new_mission;
    new_mission.mode = m;
	new_mission.aux_info = a;
	new_mission.data1 = d1;
	new_mission.data2 = d2;
	new_mission.data3 = d3;	
    mission_list.push_back(new_mission);
    final_task +=1;
}

void MISSION::process(){
    cout << " cur_task : " << cur_task <<endl;
    cout << " final_task : " << final_task <<endl;
    cout << " === " << endl;
    if(cur_task!=final_task){
        if(mission_switch()==true && cur_task!=final_task-1){ // If this mission is not the last one.
            cur_task +=1 ;
            mission_data.mode = mission_list[cur_task].mode;
            mission_data.aux_info = mission_list[cur_task].aux_info;
            mission_data.data1 = mission_list[cur_task].data1;
            mission_data.data2 = mission_list[cur_task].data2;
            mission_data.data3 =  mission_list[cur_task].data3;

            cur_target_pos.x = mission_data.data1;
            cur_target_pos.y = mission_data.data2;
            cur_target_pos.z = mission_data.data3;
        }
    }else{
        cout << "MISSION COMPLETE!" <<  endl;
        if(land_flag){
            mission_data.mode = 3; 
        }

    }
    publisher();
    
}

#endif