#include "utilities.h"
#include "definitions.h"

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int strequals(const char *str1, const char *str2)
{

	int i=0;
	
	while(str1[i]!='\0' && str2[i]!='\0')
	{
		if(str1[i]!=str2[i]) return false; /* Compara caracter a caracter si son iguales */
		i++;
	}
	
	if(str1[i]!=str2[i]) return false; /* Comprueba que el carcter de finalizacion esta en la misma posicion para ambos */
	
	return true;
}

int isDir(const char *path)
{
	
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

int isFile(const char *path)
{

	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

float calcArithmeticAverage(float a, float b)
{
	float Average = (a+b)/2.0;
	return Average;
}

char* const* createNArgvList(int argc, ...)
{
	int i;
	char** argv=malloc(sizeof(char*)*(argc+1));
	
	va_list ArgList;
	va_start(ArgList,argc);
	
	for(i=0; i<argc; i++)
	{
		argv[i] = malloc(sizeof(char)*MAX_NAME_LEN);
		strcpy(argv[i],va_arg(ArgList,char*));
	}
	
	argv[i+1] = NULL;
	
	va_end(ArgList);
	
	return (char* const*) argv;
}

void getInfo(struct Config_Info_Node *ConfigInfoList,const char *ConfigId, char *ConfigInfo)
{
	struct Config_Info_Node *Node;
	
	if(ConfigInfoList==NULL)
	{
		fprintf(stderr,"Inserted Config_Info_Node is null\n");
		return;
	}
	
	Node=ConfigInfoList;
	
	while(Node!=NULL)
	{
		if(strequals(ConfigId,Node->ConfigId)==true)
		{
			strcpy(ConfigInfo,Node->ConfigInfo);
			return;
		}
		Node=Node->Next;
	}
	
	fprintf(stderr,"Cannot recognize %s in config file\n",ConfigId);
}

struct Config_Info_Node * createConfigFileStruct(const char *ConfigFilePath)
{
	struct Config_Info_Node *ConfigInfoList;
	struct Config_Info_Node *Node;

	FILE *ConfigFile;
	char Line[MAX_NAME_LEN];
	
	const char Separator[2] = ":";
	
	char *ConfigId;
	char *ConfigInfo;
	
	if((ConfigFile = fopen(ConfigFilePath,"r"))==NULL)
	{
		fprintf(stderr, "Error in open config file %s : %s\n",ConfigFilePath,strerror(errno));
		return NULL;
	}
	
	while(fgets(Line,MAX_NAME_LEN-1,ConfigFile)!=NULL)
	{
		
		ConfigId = strtok(Line,Separator);
		ConfigInfo = strtok(NULL,Separator);
		removeNewLineJumps(ConfigInfo);
		
		if(ConfigId==NULL || ConfigInfo==NULL)
		{
			fprintf(stderr,"Fail in configuration file %s , wrong format\n",ConfigFilePath);
			continue;
		}
		
		if(strlen(ConfigId)>=MAX_NAME_LEN || strlen(ConfigFilePath)>=MAX_NAME_LEN)
		{
			fprintf(stderr,"Fail in configuration file %s , overpass specified length\n",ConfigFilePath);
			continue;
		}
		
		if(ConfigInfoList==NULL)
		{
			ConfigInfoList = createConfigInfoNode(ConfigId,ConfigInfo);
			Node = ConfigInfoList;
			continue;
		}
		
		Node->Next = createConfigInfoNode(ConfigId, ConfigInfo);
		Node = Node->Next;
	}
	
	fclose(ConfigFile);
	
	return ConfigInfoList;
}

void removeNewLineJumps(char *String)
{
	int i=0;
	
	while(String[i]!='\0')
	{
		if(String[i]=='\n')
		{
			String[i]='\0';
			break;
		}
		i++;
	}
}

struct Config_Info_Node * createConfigInfoNode(char *ConfigId, char *ConfigInfo)
{
	struct Config_Info_Node *Node= malloc(sizeof(struct Config_Info_Node));
	
	strcpy(Node -> ConfigId,ConfigId);
	strcpy(Node -> ConfigInfo,ConfigInfo);
	
	return Node;
}

void freeConfigInfoList(struct Config_Info_Node *ConfigInfoList)
{
	struct Config_Info_Node *NodeToDelete;
	
	NodeToDelete = ConfigInfoList;
	ConfigInfoList = NodeToDelete->Next;
	
	while(NodeToDelete->Next!=NULL)
	{
		free(NodeToDelete);
		NodeToDelete = ConfigInfoList;
		ConfigInfoList = NodeToDelete->Next;
	}
	
	free(NodeToDelete);
}

