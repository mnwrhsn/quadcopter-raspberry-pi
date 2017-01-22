#include "serv.h"
#include "motors.h"
#include "controldeVol.h"

int main (int argc, char *argv[]){

	char verbose = 0;
	if (argc > 1) {
		if (strcmp(argv[1], "--verbose") == 0) {
			printf("verbose MODE select\n");
			verbose = 1;
		}
	}else{
		printf("add    --verbose   for verbose mode\n");
	}

	PMutex * PmutexRemoteConnect = (PMutex *) malloc(sizeof(PMutex));
	init_PMutex(PmutexRemoteConnect);

	PMutex * PmutexDataControler = (PMutex *) malloc(sizeof(PMutex));
	init_PMutex(PmutexDataControler);

	getIP();

	DataController * dataControl =(DataController *) malloc(sizeof(DataController));
	dataControl->pmutex=PmutexDataControler;
	dataControl->moteur_active=1;

	args_SERVER * argServ =(args_SERVER *) malloc(sizeof(args_SERVER));
	argServ->pmutexRemoteConnect = PmutexRemoteConnect;
	argServ->dataController = dataControl;
	argServ->verbose=verbose;

	MotorsAll * motorsAll =(MotorsAll *) malloc(sizeof(MotorsAll));
	motorsAll->bool_arret_moteur =(int *) malloc(sizeof(int));
	*(motorsAll->bool_arret_moteur)= 0;

	init_Value_motors(motorsAll);


	args_CONTROLDEVOL * argCONTROLVOL =(args_CONTROLDEVOL *) malloc(sizeof(args_CONTROLDEVOL));
	argCONTROLVOL->dataController=dataControl;
	argCONTROLVOL->motorsAll=motorsAll;
	argCONTROLVOL->verbose=verbose;

	pthread_t threadServer;
	pthread_t threadControlerVOL;


	pthread_mutex_lock(&PmutexRemoteConnect->mutex);

	if (pthread_create(&threadServer, NULL, thread_UDP_SERVER, argServ)) {
		perror("pthread_create TCP");
		return EXIT_FAILURE;
	}

	pthread_cond_wait(&PmutexRemoteConnect->condition, &PmutexRemoteConnect->mutex);

	pthread_mutex_unlock(&PmutexRemoteConnect->mutex);


	if (pthread_create(&threadControlerVOL, NULL, startCONTROLVOL, argCONTROLVOL)) {
		perror("pthread_create PID");
		return EXIT_FAILURE;
	}

	init_threads_motors(motorsAll,verbose);//start the 4 threads et ne rends pas la main

	if (pthread_join(threadServer, NULL)){
		perror("pthread_join");
		return EXIT_FAILURE;
	}
	if (pthread_join(threadControlerVOL, NULL)){
			perror("pthread_join");
			return EXIT_FAILURE;
	}
	clean_args_SERVER(argServ);
	clean_args_CONTROLDEVOL(argCONTROLVOL);
	return 0;
}
