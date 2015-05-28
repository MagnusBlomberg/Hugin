#include <stdio.h>
#include <unistd.h>

#include "MPU6050.h"
#include "i2cfunc.h"
#include "bsocket.h"
#include "rcreader.h"
#include "VC01.h"

int main(){
	// Create socket and set listen timeout
	bsocket sock(65536);
	sock.set_listen_timeout(1000);
	unsigned int data_size;
	
	// Values that are updated from socket
	const int num_socket_vals = 2;
	socket_val socket_vals[num_socket_vals] = {
		socket_val(INTEGER, 25001, 1, "Ready val"),
		socket_val(DOUBLE, 25002, 1, "value description")
	};
	
	// Initialise RC reader - Reads PWM and S.Bus signals from the receiver via the Atmega's I2C.
	rcreader rcr(i2c_open(1, 0x05), 4);
	unsigned int channels[4] = {0,0,0,0};
	double channels_d[4];
	usleep(50000);
	//rcr.get_readings(channels);
	//printf("channels[0]: %d, channels[1]: %d, channels[2]: %d, channels[3]: %d\n", channels[0], channels[1], channels[2], channels[3]);
	//return 0;
	
	// Initialise IMU
	MPU6050 imu(i2c_open(1, 0x68));
	imu.init();
	usleep(50000); // Makes sure the IMU with gyro is up and running (min 30 ms)
	imu.calibrate_gyroscope();
	usleep(50000);
	
	// Declare values to hold IMU readings
	double acc_d[3];
	double gyro_d[3];
	
	// Init VC01
	VC01 distance_sensor(1);
	// Declare values to hold VC01 
	double distance[1];
	
	
		
	printf("Test started\n");
	while(1){
		// Listen for packets and process if new packets have arrived
		data_size = sock.listen();
		if(data_size > 0){
			sock.process_packet(data_size, socket_vals, num_socket_vals);
			// Ready to receive
			if(socket_vals[0].i_vals[0] == 1){
				socket_vals[0].i_vals[0] = 0;
				
				// Get IMU readings
				imu.get_accelerations(acc_d);
				imu.get_angular_velocities(gyro_d);	

				// Get distance values
				distance[0] = distance_sensor.get_distance();
				
				// Get RC readings
				rcr.get_readings(channels);
				for(int i = 0; i < 4; i++){
					channels_d[i] = ((double)(channels[i]-1000))/2000.0;
				}
				
				// Send updates 
				sock.send(LOCAL_HOST, 22001, acc_d, 3);
				sock.send(LOCAL_HOST, 22002, gyro_d, 3);
				sock.send(LOCAL_HOST, 22003, distance, 1);
				sock.send(LOCAL_HOST, 22102, channels_d, 4);
				
				printf("acc_d[0]: %3.5f, gyro_d[0]: %3.5f, channel[0]: %d, distance[0]: %3.5f\n", acc_d[0], gyro_d[0], channels[0], distance[0]);
			}
		}
	}
}