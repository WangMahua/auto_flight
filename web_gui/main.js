var app = new Vue({
    el:'#control_gui',
    data: {
        connected: false,
        failure: false,
        ros: null,
        ws_address: '9090',
        logs: [],
        loading: false,
        topic: null,
        message: null,
        enable_takeoff: false,
        enable_return: false,
        enable_landing: false,
        control_signal: null,
        destination_x: 0.0,
        destination_y: 0.0,
        destination_z: 0.0,
        destination: null,
        direction: 1,
        cruise_height: 1,
        hover_time: 15
    },
    methods: {
        connect: function(){
            console.log('Connecting...')

            this.ros = new ROSLIB.Ros({
                url: 'ws://0.0.0.0:' + this.ws_address
            })

            this.ros.on('connection', () => {
                this.logs.unshift((new Date()).toTimeString() + ' - connected')
                this.connected = true
                this.failure = false
                console.log('Connected')
                
                // Create a control signal publisher: take off 1, return 2, stop 0
                this.control_pub = new ROSLIB.Topic({
                    ros: this.ros,
                    name: '/control_phase',
                    messageType: 'std_msgs/Int8'
                });

                this.control_signal = new ROSLIB.Message({
                    data: 0
                })

                this.control_pub.publish(this.control_signal)
                this.logs.unshift((new Date()).toTimeString() + 'Control Signal: ' + this.control_signal.data)

                //TODO
                /* Determine whether the uav is static*/ 
                //if yes
                this.enable_takeoff = true
                this.enable_return = false

                document.getElementById("cam_topic").style.display = "block";
                document.getElementById("mjpeg").style.display = "block";
                this.showCamera()
            })
            this.ros.on('error', (error) => {
                this.failure = true
                this.logs.unshift((new Date()).toTimeString() + ' - Cannot establish connection with server')
                console.log('Connection Failed', error)
            })
            this.ros.on('close', () => {
                if(!(this.failure)){
                    this.logs.unshift((new Date()).toTimeString() + ' - disconnected')
                    this.connected = false
                    console.log('Connection Closed')
                }   
            })
        },

        disconnect: function(){
			delete this.cameraViewer;
            this.ros.close()
            this.connected = false
            console.log('Connection Closed')
            document.getElementById("cam_topic").style.display = "none";
            document.getElementById("mjpeg").style.display = "none";
        },

        showCamera: function(){
            console.log('set camera method')

            const image_port = parseInt(this.ws_address, 10) - 1010
            
            this.cameraViewer = new MJPEGCANVAS.Viewer({
                divID: 'mjpeg',
                host: '0.0.0.0',
                width: 640,
                height: 480,
                topic: '/iris1/camera_forward/color/image_raw',
                port: image_port,
            })

            document.getElementById("cam_topic").innerHTML = '/iris1/camera_forward/color/image_raw';
        },

        takeoff: function(){
            console.log('UAV takeoff')
            this.logs.unshift((new Date()).toTimeString() + 'UAV takeoff')

            //TODO
            /* Publish Takeoff signal */
            // Create a control signal publisher: take off 1, return 2, stop 0
            this.control_signal = new ROSLIB.Message({
                data: 1
            })

            this.control_pub.publish(this.control_signal)
            this.logs.unshift((new Date()).toTimeString() + 'Control Signal: ' + this.control_signal.data)

            this.enable_return = true
            this.enable_takeoff = false
            this.enable_landing = true
        },
        landing: function(){
            console.log('UAV landing')
            this.logs.unshift((new Date()).toTimeString() + 'UAV landing')

            //TODO
            /* Publish Takeoff signal */
            // Create a control signal publisher: take off 1, return 2, stop 0
            this.control_signal = new ROSLIB.Message({
                data: 3
            })

            this.control_pub.publish(this.control_signal)
            this.logs.unshift((new Date()).toTimeString() + 'Control Signal: ' + this.control_signal.data)

            this.enable_return = true
            this.enable_landing = false
        },

        returning: function(){
            console.log('uav returning')
            this.logs.unshift((new Date()).toTimeString() + 'UAV returning')
            
            //TODO
            /* Publish uav landing signal */
            // Create a control signal publisher: take off 1, return 2, stop 0
            this.control_signal = new ROSLIB.Message({
                data: 2
            })

            this.control_pub.publish(this.control_signal)
            this.logs.unshift((new Date()).toTimeString() + 'Control Signal: ' + this.control_signal.data)

            this.enable_return = false
            this.enable_takeoff = true
            this.enable_landing = true
        },

        stop: function(){
            console.log('uav stopping')
            this.logs.unshift((new Date()).toTimeString() + 'UAV stopping')
            //TODO
            /*Send emergency stop signal (LAND DIRECTLY)*/
            // Create a control signal publisher: take off 1, return 2, stop 0
            this.control_signal = new ROSLIB.Message({
                data: 0
            })

            this.control_pub.publish(this.control_signal)
            this.logs.unshift((new Date()).toTimeString() + 'Control Signal: ' + this.control_signal.data)

            this.enable_return = false
            this.enable_takeoff = false
        },

        startFlight: function(){
            //TODO
            /*Send goal point, direction, hover_time & cruise_height*/
            // Create a control signal publisher: take off 1, return 2, stop 0
            console.log('uav set waypoint')
            this.logs.unshift((new Date()).toTimeString() + 'UAV moving')
            const destination_x = parseFloat(this.destination_x, 0.0)
            const destination_y = parseFloat(this.destination_y, 0.0)
            const destination_z = parseFloat(this.destination_z, 0.0)

            this.dest_pub = new ROSLIB.Topic({
                ros: this.ros,
                name: '/flight_destination',
                messageType: 'geometry_msgs/Vector3'
            });

            var destination = new ROSLIB.Message({
                x: destination_x,
                y: destination_y,
                z: destination_z
            })

            this.dest_pub.publish(destination)
            this.logs.unshift((new Date()).toTimeString() + 'Flying to tag ' + destination.x + ','+ destination.y + ','+ destination.z)
        
        },

        chooseDirection: function(action){
            if(document.getElementById("left_btn").disabled || document.getElementById("right_btn").disabled){
                document.getElementById("left_btn").disabled = false;
                document.getElementById("right_btn").disabled = false;
            }else{
                switch(action){
                    case "left":
                        this.direction = 0;
                        document.getElementById("left_btn").disabled = false;
                        document.getElementById("right_btn").disabled = true;
                        break;
                    case "right":
                        this.direction = 1;
                        document.getElementById("left_btn").disabled = true;
                        document.getElementById("right_btn").disabled = false;
                        break;
                }
            }

        }
    },
    updated(){
        //TODO
        //Read these info from ros topic
        document.getElementById("uav_id").innerHTML = "1";
        document.getElementById("uav_status").innerHTML = "Cruising";
        document.getElementById("uav_location").innerHTML = "Nav point";

        if(this.destination == ''){
            document.getElementById("btn_start").disabled = true;
        }else{
            document.getElementById('btn_start').disabled = false;
        }   
    },
})
