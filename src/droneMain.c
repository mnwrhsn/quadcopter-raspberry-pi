#include "serv.h"
#include "motors.h"
#include "PID.hpp"
#include "Calibration/calibrate.h"

volatile sig_atomic_t boolStopMotor;
volatile sig_atomic_t boolStopServ;


void handler_SIGINT_Drone(int i){
	logString("THREAD MAIN : SIGINT catched -> process to stop");
	boolStopServ=1;
	boolStopMotor=1;
}

int main (int argc, char *argv[]){

	init_mask(handler_SIGINT_Drone);
	
	if(tokenAnalyse(argc,argv,FLAG_OPTIONS_DRONE)){
		return EXIT_FAILURE;
	}

	char myIP[64];
	if(getIP(myIP)){
		return EXIT_FAILURE;	
	}
	if (isIpSound()) {	
		readIpAdresse(myIP, 64);
	}

	args_SERVER * argServ;
	if(init_args_SERVER(&argServ,&boolStopServ)){
		return EXIT_FAILURE;
	}

	MotorsAll3 * motorsAll3;
	if (init_MotorsAll3(&motorsAll3,&boolStopMotor)) {
		return EXIT_FAILURE;
	}

	args_PID * argPID;
	if (init_args_PID(&argPID,argServ->dataController,motorsAll3)) {
		return EXIT_FAILURE;
	}

	pthread_t threadServer;
	pthread_t threadPID;

	void *threadPID_stack_buf=NULL;

	if(!isNoControl()){
		pthread_mutex_lock(&argServ->pmutexRemoteConnect->mutex);

		if (pthread_create(&threadServer, NULL, thread_UDP_SERVER, argServ)) {
			logString("THREAD MAIN : pthread_create SERVER\n");
			set_Motor_Stop(motorsAll3);
			return EXIT_FAILURE;
		}

		pthread_cond_wait(&argServ->pmutexRemoteConnect->condition, &argServ->pmutexRemoteConnect->mutex);

		pthread_mutex_unlock(&argServ->pmutexRemoteConnect->mutex);

		if (is_Serv_Stop(argServ)) {
			return EXIT_FAILURE;
		}
	}

	if(init_thread_PID(&threadPID,threadPID_stack_buf,argPID)){
		set_Serv_Stop(argServ);
		set_Motor_Stop(motorsAll3);
		return EXIT_FAILURE;
	}

	/**
	 * AFTER THE CALIBRATION THE PROGRAM FINISH
	 */
	if (isCalibration()) {
		calibrate_ESC(motorsAll3, isVerbose());
		set_Motor_Stop(motorsAll3);
		set_Serv_Stop(argServ);
	}


	int * returnValue;

	int re=0;

	if (pthread_join(threadPID, (void**) &returnValue)) {
		logString("THREAD MAIN : ERROR pthread_join PID");
		set_Motor_Stop(motorsAll3);
		set_Serv_Stop(argServ);
		return EXIT_FAILURE;
	}


	if (!isNoControl()) {
		if ((re = pthread_join(threadServer, NULL)) > 0) {
			logString("THREAD MAIN : ERROR pthread_join SERVER");
			set_Serv_Stop(argServ);
			return EXIT_FAILURE;
		}
	}


	if(threadPID_stack_buf!=NULL){
		//munmap(threadPID_stack_buf, PTHREAD_STACK_MIN);
	}

	clean_args_SERVER(argServ);
	clean_args_PID(argPID);

	logString("THREAD MAIN : END");
	closeLogFile();
	return EXIT_SUCCESS;
}
