#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <definitions.h>
#include <utilities.h>

/*	
	argv[1] = Students file path
	argv[2] = Resources path folder
	argv[3] = students target directory 
*/

pid_t ProcessTable[ProcessClassEnumLength];
char StudentsTargetDirectory[MAX_PATH_LEN];
	
void parseArgv(int argc, const char *argv[],char *StudentsFilePath, char *ExamsFolder, char *StudentsTargetDirectory);

void printUsage();

/*Load values for execution*/
void loadConfigValues(char *Pattern, char *Extension, char *InfoFileName, char *InfoMessage);
void readFromConfigFile(FILE *ResourcesConfigFile, char *ValueToRead, const char *IdTesting);
void createStudentsFolder(const char *StudentsTargetDirectory);

/*Creation and process management*/
void createProcessesByClass(enum Process_Class_Enum ProcessClass, char *const argv[]);
pid_t createSingleProcess(const char *Path, char *const argv[]);
int waitNProcesses(int RemainingProcesses);
void terminateProcesses();

void readAverageResultFromPipe(int PipeDescriptor);
void managerLog(char *Message);

/*Clean the results of the execution*/
void cleanGeneratedDirectories();

/*Signal management*/
void installSignalHandler();
void signalHandler(int signo);

int main(int argc, char *argv[])
{

	char ExamsFolder[MAX_PATH_LEN];
	char StudentsFilePath[MAX_PATH_LEN];
	
	installSignalHandler();
	
	parseArgv(argc,(const char**) argv, StudentsFilePath, ExamsFolder, StudentsTargetDirectory);
	
	char Pattern[MAX_NAME_LEN];
	char Extension[MAX_NAME_LEN];
	char InfoFileName[MAX_NAME_LEN];
	char InfoMessage[MAX_PATH_LEN];
	
	loadConfigValues(Pattern,Extension,InfoFileName,InfoMessage);
	createStudentsFolder(StudentsTargetDirectory);
	
	char LogMessage[MAX_NAME_LEN];
	sprintf(LogMessage,"Deploying PA\n");
	managerLog(LogMessage);
	
	char *const *argvPA = createNArgvList(2, StudentsFilePath, StudentsTargetDirectory);
	
	createProcessesByClass(PA, argvPA);
	
	if(waitNProcesses(1)==false)
	{
		terminateProcesses();
		return EXIT_FAILURE;
	}

	sprintf(LogMessage,"Process PA finalization\n");
	managerLog(LogMessage);
	
	int PipeAverageResult[2];
	char StringPipe[MAX_NAME_LEN];
	
	pipe(PipeAverageResult);
	
	sprintf(StringPipe,"%d",PipeAverageResult[PIPE_WRITE_CHANNEL]);
	
	char *const* argvPB = createNArgvList(5, StudentsTargetDirectory, StudentsFilePath, ExamsFolder, Pattern, Extension);
	char *const* argvPC = createNArgvList(5, StringPipe, StudentsFilePath, StudentsTargetDirectory, InfoFileName, InfoMessage);
	
	sprintf(LogMessage,"Deploying PB and PC\n");
	managerLog(LogMessage);
	
	createProcessesByClass(PB, argvPB);
	createProcessesByClass(PC, argvPC);
	
	if(waitNProcesses(2)==false)
	{
		terminateProcesses();
		return EXIT_FAILURE;
	}
	
	sprintf(LogMessage,"PB and PC finalized\n");
	managerLog(LogMessage);
	
	readAverageResultFromPipe(PipeAverageResult[PIPE_READ_CHANNEL]);
	close(PipeAverageResult[PIPE_READ_CHANNEL]);
	close(PipeAverageResult[PIPE_WRITE_CHANNEL]);
	
	
	
	return EXIT_SUCCESS;	
}

void managerLog(char *Message)
{
	fprintf(stdout,"%s\n",Message);
}

void readAverageResultFromPipe(int PipeDescriptor)
{
	char Buffer[MAX_NAME_LEN];
	char LogMessage[MAX_NAME_LEN];
	
	read(PipeDescriptor, Buffer, sizeof(Buffer));
	sprintf(LogMessage,"The average students grade result is: %s",Buffer);
	managerLog(LogMessage);
}

void parseArgv(int argc, const char *argv[],char *StudentsFilePath, char *ExamsFolder, char *StudentsTargetDirectory)
{

	if(argc!=4)
	{
		fprintf(stderr,"Wrong number of arguments\n");
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	if(strlen(argv[1])>=MAX_PATH_LEN)
	{
		fprintf(stderr,"Error, specified students file path length overpass, MAX %d characters\n",MAX_PATH_LEN-1);
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	 if(strlen(argv[2])>=(MAX_PATH_LEN-(MAX_NAME_LEN*2)))
	{
		fprintf(stderr,"Error, specified Exams Folder path length overpass, MAX %d characters\n",(MAX_PATH_LEN-(MAX_NAME_LEN*2))-1);
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	if(isFile(argv[1])==false)
	{
		fprintf(stderr,"Error with the specified students file path %s : %s\n",argv[1],strerror(errno));
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	if(isDir(argv[2])==false)
	{
		fprintf(stderr,"Error with the specified exams folder %s : %s\n",argv[2],strerror(errno));
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	if(isDir(argv[3])==true)
	{
		fprintf(stderr, "The specified students target directory %s already exists\n",argv[3]);
		printUsage();
		exit(EXIT_FAILURE);
	}
	
	strcpy(StudentsFilePath,argv[1]);
	strcpy(ExamsFolder,argv[2]);
	strcpy(StudentsTargetDirectory,argv[3]);
	
}

void printUsage()
{

	char LogMessage[MAX_NAME_LEN];
	sprintf(LogMessage,"Correct use of the program: ./manager <Students File Path> <Exams Folder Path> <Students Target Dir>\n");
	managerLog(LogMessage);
	
}

void loadConfigValues(char *Pattern, char *Extension, char *InfoFileName, char *InfoMessage)
{
	
	struct Config_Info_Node *ConfigInfoList;
	
	ConfigInfoList = createConfigFileStruct(CONFIG_FILE_PATH);
	getInfo(ConfigInfoList,PATTERN_IDENTIFICATION,Pattern);
	getInfo(ConfigInfoList,EXTENSION_IDENTIFICATION,Extension);
	getInfo(ConfigInfoList, INFO_FILE_NAME,InfoFileName);
	getInfo(ConfigInfoList, INFO_STRING,InfoMessage);
	
}

void createStudentsFolder(const char *TargetPath)
{

	if(mkdir(TargetPath,0700)==-1)
	{
		fprintf(stderr,"Error in creation of the students folder: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
}
	
pid_t createSingleProcess(const char *Path, char *const argv[])
{
	pid_t pid;
	switch (pid = fork()) {
		case -1:
			fprintf(stderr, "[MANAGER] Error creating %s process: %s.\n", Path,strerror(errno));
		/* Child process */
		case 0:
			if (execv(Path, argv) == -1)
			{
				fprintf(stderr, "[MANAGER] Error using execv() in %s process: %s.\n", Path, strerror(errno));
				exit(EXIT_FAILURE);
			}
	}
	/* Parent process */
	return pid;
}

void createProcessesByClass(enum Process_Class_Enum ProcessClass, char *const argv[])
{
	pid_t pid;
	
	switch(ProcessClass)
	{
		case PA:
			pid = createSingleProcess(PA_PATH, argv);
			break;
		case PB:
			pid = createSingleProcess(PB_PATH, argv);
			break;
		case PC:
			pid = createSingleProcess(PC_PATH, argv);
			break;
		case PD:
			pid = createSingleProcess(PD_PATH, argv);
			
			/*f the cleaning process fails, it does not call itself again*/
			if(pid==-1) exit(EXIT_FAILURE);
			
			break;
		default:
			return;
	}
	
	if(pid==-1)
	{
		terminateProcesses();
		exit(EXIT_FAILURE);
	}
	ProcessTable[ProcessClass] = pid;
}

int waitNProcesses(int RemainingProcesses)
{
	int i,Status;
	char LogMessage[MAX_NAME_LEN];
	pid_t pid;
	
	while(RemainingProcesses>0)
	{
		pid = wait(&Status);
		
		for(i=0; i<ProcessClassEnumLength; i++)
		{
			if(ProcessTable[i]==pid)
			{
				ProcessTable[i] = 0;
				break;
			}
		}
		
		if(WIFEXITED(Status)==0)
		{
			sprintf(LogMessage,"Bad termination of the proccess %d",(int) pid);
			managerLog(LogMessage); 
			return false;
		}
		
		RemainingProcesses--;
	}
	
	return true;
}

void terminateProcesses()
{
	char LogMessage[MAX_NAME_LEN];
	sprintf(LogMessage,"Initiating Termination Sequence\n");
	managerLog(LogMessage);
	
	int i;
	for(i=0; i<ProcessClassEnumLength-1; i++)
	{
		if(ProcessTable[i]!=0)
		{
			kill(ProcessTable[i],SIGINT);
			wait(NULL);
			ProcessTable[i]=0;
		}
	}
	
	cleanGeneratedDirectories();
	exit(EXIT_FAILURE);
}

void cleanGeneratedDirectories()
{
	char LogMessage[MAX_NAME_LEN];
	sprintf(LogMessage,"Cleaning files generated in %s\n", StudentsTargetDirectory);
	managerLog(LogMessage);
	
	char *const *argvPD = createNArgvList(1, StudentsTargetDirectory);
	
	createProcessesByClass(PD,argvPD);
	wait(NULL);
}

void installSignalHandler()
{
	if(signal(SIGINT, signalHandler) == SIG_ERR)
	{
		fprintf(stderr, "[Manager %d] Error installing signal handler: %s\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void signalHandler(int signo)
{
	char LogMessage[MAX_NAME_LEN];
	sprintf(LogMessage,"[Manager %d] Ctrl + C user interruption\n", getpid());
	managerLog(LogMessage);
	terminateProcesses();
	exit(EXIT_FAILURE);
}

#undef _POSIX_SOURCE
