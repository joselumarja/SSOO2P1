#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <definitions.h>
#include <utilities.h>


/*	argv[0] = path students target directory
	argv[1] = path students file
	argv[2] = path exams folder
	argv[3] = pattern id
	argv[4] = extension id
*/

struct Exam_Type_Node *gExamTypesList;

void parseArgv(int argc, const char *argv[], char *StudentsTargetDirectory, char *StudentsFilePath, char *ExamsFolderPath, char *PatternId, char *ExtensionId);

/*Read values from students file*/
void loadStudentsValues(const char *StudentsFilePath);

/*Creation of structures to load the values into memory*/
void addExamTypeToList(const char *ExamId, const char *StudentId);
struct Exam_Type_Node* createExamType(const char *ExamId);
void addStudentToList(const char *StudentId,struct Double_Ended_Linked_List *StudentsList);
struct Student_Node* createStudent(const char *StudentId);

/*Group of functions to optimize the copy of the files*/
void handTheExamsToStudents(const char *TargetPath, const char *ExamsFolderPath, const char *Pattern, const char *Extension);
void copySingleExamType(const char *ExamFolder,const char *ExamName, const char *TargetPath,struct Double_Ended_Linked_List *StudentsList);
void copyBufferToList(ssize_t CountToCopy, const char *Buffer,struct Double_Ended_Linked_List *StudentsList);
void initializeCopyList(const char *ExamName, const char *TargetPath,struct Double_Ended_Linked_List *StudentsList);
void finalizeCopyList(struct Double_Ended_Linked_List *List);

/*Free used memory*/
void freeResources();
void freeStudentsList(struct Double_Ended_Linked_List *StudentsList);

void installSignalHandler();
void signalHandler(int signo);


int main(int argc, char *argv[])
{

	char StudentsTargetPath[MAX_PATH_LEN];
	char StudentsFilePath[MAX_PATH_LEN];
	char ExamsFolderPath[MAX_PATH_LEN];
	char PatternId[MAX_NAME_LEN];
	char ExtensionId[MAX_NAME_LEN];

	installSignalHandler();

	parseArgv(argc,(const char**) argv, StudentsTargetPath, StudentsFilePath, ExamsFolderPath, PatternId, ExtensionId);
	loadStudentsValues((const char*) StudentsFilePath);
	handTheExamsToStudents((const char*) StudentsTargetPath, (const char*) ExamsFolderPath, (const char*) PatternId, (const char*) ExtensionId);
	
	return EXIT_SUCCESS;
}

void parseArgv(int argc, const char *argv[], char *StudentsTargetPath, char *StudentsFilePath, char *ExamsFolderPath, char *PatternId, char *ExtensionId)
{
	if(argc!=5)
	{
		fprintf(stderr, "Wrong argument number in process PB\n");
		exit(EXIT_FAILURE);
	}

	strcpy(StudentsTargetPath,argv[0]);
	strcpy(StudentsFilePath,argv[1]);
	strcpy(ExamsFolderPath,argv[2]);
	strcpy(PatternId,argv[3]);
	strcpy(ExtensionId,argv[4]);
}

void loadStudentsValues(const char *StudentsFilePath)
{

	char StudentId[MAX_NAME_LEN];
	char ExamId[MAX_NAME_LEN];
	char Trash[MAX_NAME_LEN];
	
	FILE *StudentsFile;

	if((StudentsFile=fopen(StudentsFilePath,"r"))==NULL)
	{
		fprintf(stderr,"Error in the opening of the students file %s : %s\n",StudentsFilePath, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while(fscanf(StudentsFile, "%s %s %s", StudentId, ExamId, Trash)>0)
	{
		addExamTypeToList((const char*) ExamId, (const char*) StudentId);
	}
}

void addExamTypeToList(const char *ExamId, const char *StudentId)
{
	struct Exam_Type_Node *node;

	if(gExamTypesList==NULL)
	{
		gExamTypesList=createExamType(ExamId);
		addStudentToList(StudentId, gExamTypesList->StudentsList);
		return;
	}	
		
	node=gExamTypesList;
	
	while(node->Next!=NULL)
	{
		if(strequals(node->Id,ExamId)==true)
		{
			addStudentToList(StudentId, node->StudentsList);
			return;
		}
		
		node=node->Next;
		
	}
	
	if(strequals(node->Id, ExamId)==true)
	{
		addStudentToList(StudentId, node->StudentsList);
	}
	else
	{
		node->Next=createExamType(ExamId);
		addStudentToList(StudentId,node->Next->StudentsList);
	}
}
	
struct Exam_Type_Node* createExamType(const char *ExamId)
{
	struct Exam_Type_Node *node;
	
	node=malloc(sizeof(struct Exam_Type_Node));
	sprintf(node->Id,"%s", ExamId);
	node->StudentsList=malloc(sizeof(struct Double_Ended_Linked_List));

	return node;
}

void addStudentToList(const char *StudentId,struct Double_Ended_Linked_List *StudentsList)
{
	struct Student_Node *node;
	
	if(StudentsList->FirstElement==NULL)
	{
		StudentsList->FirstElement = StudentsList->LastElement = createStudent(StudentId);
		return;
	}
	
	node = createStudent(StudentId);
	StudentsList->LastElement->Next = node;
	StudentsList->LastElement = node;
}

struct Student_Node* createStudent(const char *StudentId)
{
	struct Student_Node *node;
	
	node=malloc(sizeof(struct Student_Node));
	sprintf(node->StudentId,"%s", StudentId);
	
	return node;
}

void handTheExamsToStudents(const char *TargetPath, const char *ExamsFolderPath, const char *Pattern, const char *Extension)
{
	struct Exam_Type_Node *node;
	char ExamName[MAX_NAME_LEN];
	
	node = gExamTypesList;
		
	while(node!=NULL)
	{
		sprintf(ExamName,"%s%s%s",Pattern,node->Id,Extension);
		copySingleExamType(ExamsFolderPath,ExamName, TargetPath, node->StudentsList);
		node = node->Next;
	}
	
	freeResources();
}
		
void copySingleExamType(const char *ExamFolderPath,const char *ExamName, const char *TargetPath,struct Double_Ended_Linked_List *StudentsList)
{
	char ExamPath[MAX_PATH_LEN];
	char Buffer[MAX_PATH_LEN];
	ssize_t count;
	int ExamToCopy;
	
	initializeCopyList(ExamName, TargetPath, StudentsList);
	sprintf(ExamPath,"%s/%s",ExamFolderPath,ExamName);
	
	if((ExamToCopy=open(ExamPath,O_RDONLY))==-1)
	{
		fprintf(stderr,"Error in aperture of the file %s : %s\n",ExamPath,strerror(errno));
		freeResources();
		exit(EXIT_FAILURE);
	}
	
	while((count=read(ExamToCopy,Buffer,MAX_PATH_LEN))>0)
	{
		copyBufferToList(count,(const char*) Buffer, StudentsList);
	}
	
	finalizeCopyList(StudentsList);
	close(ExamToCopy);
}

void initializeCopyList(const char *ExamName, const char *TargetPath,struct Double_Ended_Linked_List *StudentsList)
{
	struct Student_Node *node;
	char ExamTargetPath[MAX_PATH_LEN];
	int fd;
	
	node = StudentsList->FirstElement;
	
	while(node!=NULL)
	{
		sprintf(ExamTargetPath, "%s/%s/%s",TargetPath,node->StudentId,ExamName);
		
		if((fd=open(ExamTargetPath,O_CREAT|O_TRUNC|O_RDWR))==-1)
		{
			fprintf(stderr,"Warning, couldn't create %s file\n",ExamTargetPath);
		}
		
		node->ExamCopyTarget=fd;
		node=node->Next;
	}
}

void finalizeCopyList(struct Double_Ended_Linked_List *StudentsList)
{
	struct Student_Node *node;
	
	while(node!=NULL)
	{
		close(node->ExamCopyTarget);
		node->ExamCopyTarget=-1;
		node=node->Next;
	}
}

void copyBufferToList(ssize_t CountToCopy,const char *Buffer,struct Double_Ended_Linked_List *StudentsList)
{
	struct Student_Node *node;
	ssize_t x;
	node = StudentsList->FirstElement;
	
	while(node!=NULL)
	{
		if((x=write(node->ExamCopyTarget, Buffer, CountToCopy)!=CountToCopy))
		{
			fprintf(stderr, "Warning, fail in exam copy to %s student\n",node->StudentId);
		}
		
		node=node->Next;
	}
}

void freeResources()
{
	struct Exam_Type_Node *NodeToLiberate;
	
	if(gExamTypesList==NULL) return;
	
	NodeToLiberate = gExamTypesList;
	gExamTypesList = NodeToLiberate->Next;
	
	while(NodeToLiberate->Next!=NULL)
	{
		freeStudentsList(NodeToLiberate->StudentsList);
		free(NodeToLiberate);
		NodeToLiberate = gExamTypesList;
		gExamTypesList = NodeToLiberate->Next;
		
	}
	
	free(NodeToLiberate);
}

void freeStudentsList(struct Double_Ended_Linked_List *StudentsList)
{
	struct Student_Node *NodeToLiberate;
	
	if(StudentsList==NULL) return;
	
	if(StudentsList->FirstElement==NULL)
	{
		free(StudentsList);
		return;
	}
	
	NodeToLiberate = StudentsList->FirstElement;
	StudentsList->FirstElement = NodeToLiberate->Next;
	
	while(NodeToLiberate->Next!=NULL)
	{
		free(NodeToLiberate);
		NodeToLiberate = StudentsList->FirstElement;
		StudentsList->FirstElement = NodeToLiberate->Next;
	}
	
	free(NodeToLiberate);
	free(StudentsList);
}

void installSignalHandler()
{
	if(signal(SIGINT, signalHandler) == SIG_ERR)
	{
		fprintf(stderr, "[PB %d] Error instaling handler: %s\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void signalHandler(int signo)
{
	freeResources();
	printf("[PB %d] terminated (SIGINT)\n", getpid());
	exit(EXIT_SUCCESS);
}
