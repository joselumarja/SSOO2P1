#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <definitions.h>
#include <utilities.h>

/*	
	argv[0] = pipe descriptor
	argv[1] = students file path
	argv[2] = path students target directory
	argv[3] = info file name
	argv[4] = info message
*/

void parseArgv(int argc,const char *argv[], int *PipeWriteDescriptor, char *StudentsFilePath, char *StudentsTargetDirectory, char *InfoFileName, char *InfoMessage);

/*Read student info from students file*/
float readStudentsMarks(const char *StudentsFilePath,const char *StudentsTargetDirectory, const char *InfoFileName, const char *InfoMessage);

/*Create the info file into student directory*/
void createStudentMarkInfo(const char *StudentTargetPath, const char *InfoFileName, float LastMark, const char *InfoMessage);

/*Send academic media to manager*/
void notifyAverageGradeToParent(int PipeWriteDescriptor, float AverageGrade);

void installSignalHandler();
void signalHandler(int signo);

int main(int argc, char *argv[])
{
	int PipeDescriptor;
	
	float AverageGrade = 0.0;
	
	char StudentsFilePath[MAX_PATH_LEN];
	char StudentsTargetDirectory[MAX_PATH_LEN];
	char InfoFileName[MAX_NAME_LEN];
	char InfoMessage[MAX_PATH_LEN];
	
	installSignalHandler();
		
	parseArgv(argc,(const char**) argv, &PipeDescriptor, StudentsFilePath, StudentsTargetDirectory, InfoFileName, InfoMessage);
	AverageGrade = readStudentsMarks(StudentsFilePath, StudentsTargetDirectory, InfoFileName, InfoMessage);
	notifyAverageGradeToParent(PipeDescriptor, AverageGrade);
	
	return EXIT_SUCCESS;
}

void parseArgv(int argc, const char* argv[], int *PipeDescriptor, char *StudentsFilePath, char *StudentsTargetDirectory, char *InfoFileName, char *InfoMessage)
{

	if(argc!=5)
	{
		fprintf(stderr, "Wrong argument number in process PC\n");
		exit(EXIT_FAILURE);
	}
	
	*PipeDescriptor = atoi(argv[0]);
	strcpy(StudentsFilePath,argv[1]);
	strcpy(StudentsTargetDirectory,argv[2]);
	strcpy(InfoFileName,argv[3]);
	strcpy(InfoMessage, argv[4]);
}

float readStudentsMarks(const char *StudentsFilePath, const char *StudentsTargetDirectory, const char *InfoFileName, const char *InfoMessage)
{
	char StudentId[MAX_NAME_LEN];
	char LastMarkString[MAX_NAME_LEN];
	char Trash[MAX_NAME_LEN];
	
	char StudentTargetPath[MAX_PATH_LEN];
	
	float AverageGrade = 0.0;
	float LastMark = 0.0;
	
	FILE *StudentsFile;
	
	if((StudentsFile=fopen(StudentsFilePath,"r"))==NULL)
	{
		fprintf(stderr,"Error in the opening of the students file %s : %s\n",StudentsFilePath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while(fscanf(StudentsFile, "%s %s %s", StudentId, Trash, LastMarkString)>0)
	{
		sprintf(StudentTargetPath,"%s/%s", StudentsTargetDirectory, StudentId);
		LastMark = (float) atoi(LastMarkString);
		createStudentMarkInfo(StudentTargetPath, InfoFileName, LastMark, InfoMessage);
		AverageGrade = calcArithmeticAverage(AverageGrade, LastMark);
	}
	
	return LastMark;
}

void createStudentMarkInfo(const char *StudentTargetPath, const char *InfoFileName, float LastMark, const char *InfoMessage)
{
	FILE *Info;
	
	char InfoFilePath[MAX_PATH_LEN];
	
	sprintf(InfoFilePath,"%s/%s", StudentTargetPath, InfoFileName);
	
	if((Info=fopen(InfoFilePath,"w+"))==NULL)
	{
		fprintf(stderr,"Error in creation of the file %s : %s\n",StudentTargetPath,strerror(errno));
	}
	
	fprintf(Info,"%s %.2f\n", InfoMessage, (10.0-LastMark));
	
	fclose(Info);
}

void notifyAverageGradeToParent(int PipeWriteDescriptor, float AverageGrade)
{
	char AverageGradeString[MAX_NAME_LEN];

	sprintf(AverageGradeString,"%.2f",AverageGrade);
	write(PipeWriteDescriptor, AverageGradeString, MAX_NAME_LEN);
}

void installSignalHandler()
{
	if(signal(SIGINT, signalHandler) == SIG_ERR)
	{
		fprintf(stderr, "[PC %d] Error instaling handler: %s\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void signalHandler(int signo)
{
	printf("[PC %d] terminated (SIGINT)\n", getpid());
	exit(EXIT_SUCCESS);
}
