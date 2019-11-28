#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <definitions.h>
#include <utilities.h>

/*
	argv[0] = path to delete
*/

void parseArgv(int argc, const char *argv[], char *FolderToDelete);
void deleteFolder(const char* FolderToDelete);

int main(int argc, char *argv[])
{
	char FolderToDelete[MAX_PATH_LEN];
	
	parseArgv(argc, (const char **) argv, FolderToDelete);
	deleteFolder(FolderToDelete);
	
	return EXIT_SUCCESS;
}

void parseArgv(int argc, const char *argv[], char *FolderToDelete)
{
	if(argc!=2)
	{
		fprintf(stderr, "Wrong argument number in process PD\n");
		fprintf(stderr, "Fail in cleaning directory\n");
		exit(EXIT_FAILURE);
	}
	
	strcpy(FolderToDelete,argv[0]);
}

void deleteFolder(const char *FolderToDelete)
{
		DIR *dir;
		struct dirent *node;
		struct stat st;
		
		char Name[MAX_PATH_LEN];
		
		if((dir=opendir(FolderToDelete))==NULL)
		{
			fprintf(stderr, "Fail opening %s folder to delete\n", FolderToDelete);
			 return;
		}
		
		while((node=readdir(dir))!=NULL)
		{
			if((strlen(FolderToDelete)+strlen(node->d_name)+2)>sizeof(Name))
			{
				fprintf(stderr,"Too long name %s/%s to delete\n",FolderToDelete,node->d_name);
				return;
			}
			sprintf(Name,"%s/%s", FolderToDelete, node->d_name);
			
			if(strequals(node->d_name,".")==false && strequals(node->d_name,"..")==false)
			{
				if(stat(Name, &st)==false) continue;
				
				if(S_ISDIR(st.st_mode))
				{
					deleteFolder((const char*) Name);
					continue;
				}
				
				if(unlink(Name)==false)
				{
					fprintf(stderr, "Error in delete of %s : %s\n",Name,strerror(errno));
					return;
				}
			}
		}
		
		rmdir(FolderToDelete);
		closedir(dir);
				
}
