#define PA_CLASS "PA"
#define PA_PATH "./exec/pa"
#define PB_CLASS "PB"
#define PB_PATH "./exec/pb"
#define PC_CLASS "PC"
#define PC_PATH "./exec/pc"
#define PD_CLASS "PD"
#define PD_PATH "./exec/pd"

enum Process_Class_Enum{PA,PB,PC,PD,ProcessClassEnumLength};

#define MAX_NAME_LEN 128
#define MAX_PATH_LEN 4096

#define CONFIG_FILE_PATH "./Resources.config"
#define PATTERN_IDENTIFICATION "Pattern"
#define EXTENSION_IDENTIFICATION "Extension"
#define INFO_FILE_NAME "Info_File_Name"
#define INFO_STRING "Info_String"

struct Config_Info_Node{
	char ConfigId[MAX_NAME_LEN];
	char ConfigInfo[MAX_NAME_LEN];
	struct Config_Info_Node *Next;
};

#define PIPE_READ_CHANNEL 0
#define PIPE_WRITE_CHANNEL 1

struct Student_Node{
	char StudentId[MAX_NAME_LEN];
	int ExamCopyTarget;
	struct Student_Node *Next;
};

struct Exam_Type_Node{
	char Id[MAX_NAME_LEN];
	struct Double_Ended_Linked_List *StudentsList;
	struct Exam_Type_Node *Next;
};

struct Double_Ended_Linked_List{
	struct Student_Node *FirstElement;
	struct Student_Node *LastElement;
};
