#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include <definitions.h>
#include <utilities.h>

/* 
	argv[0] = students file path
	argv[1] = students file directory 
*/

void parseArgv(int argc, const char *argv[], char *StudentsFilePath, char *StudentsTargetDirectory);
	
void createStudentsDirectory(const char *StudentsFilePath,const char *StudentsTargetDirectory);
void createSingleDirectory(const char *StudentsPathTarget, const char *StudentIdentification);

void installSignalHandler();
void signalHandler(int signo);

int main(int argc, char *argv[])
{

	char StudentsTargetDirectory[MAX_NAME_LEN];
	char StudentsFilePath[MAX_PATH_LEN];
	
	installSignalHandler();
	
	parseArgv(argc, (const char**) argv, StudentsFilePath, StudentsTargetDirectory);
	createStudentsDirectory(StudentsFilePath,(const char*) StudentsTargetDirectory);
	
	return EXIT_SUCCESS;
}

void parseArgv(int argc, const char *argv[], char *StudentsFilePath, char *StudentsTargetDirectory)
{
	if(argc!=3)
	{
		fprintf(stderr, "Wrong argument number in process PA\n");
		exit(EXIT_FAILURE);
	}
	strcpy(StudentsFilePath,argv[0]);
	strcpy(StudentsTargetDirectory,argv[1]);
}

void createStudentsDirectory(const char *StudentsFilePath, const char *StudentsTargetDirectory)
{

	FILE *StudentsFile;
	char StudentIdentification[MAX_NAME_LEN];
	char Trash[MAX_NAME_LEN];
	
	if((StudentsFile=fopen(StudentsFilePath, "r"))==NULL)
	{
		fprintf(stderr, "Error in the opening of the students file %s : %s\n",StudentsFilePath,strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while(fscanf(StudentsFile, "%s %s %s",StudentIdentification,Trash,Trash)>0)
	{
		createSingleDirectory(StudentsTargetDirectory,StudentIdentification);
	}
	
	fclose(StudentsFile);
}

void createSingleDirectory(const char *StudentsTargetDirectory, const char *StudentIdentification)
{

	char SingleStudentDirectory[MAX_PATH_LEN];
	
	sprintf(SingleStudentDirectory,"%s/%s",StudentsTargetDirectory,StudentIdentification);
	
	if(mkdir(SingleStudentDirectory,0700)==-1)
	{
		fprintf(stderr,"Error in creation of %s : %s\n",SingleStudentDirectory,strerror(errno));
		exit(EXIT_FAILURE);
	}
	
}
	
void installSignalHandler()
{
	if(signal(SIGINT, signalHandler) == SIG_ERR)
	{
		fprintf(stderr, "[PA %d] Error instaling handler: %s\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void signalHandler(int signo)
{
	printf("[PA %d] terminated (SIGINT)\n", getpid());
	exit(EXIT_SUCCESS);
}	
	
