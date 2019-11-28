#ifndef UTILITIES_H_   /* Include guard */
#define UTILITIES_H_

#define true 0
#define false -1

int strequals(const char *str1, const char *str2);  /* Compara dos strings y devuelve -1 en caso de que sean diferentes y 1 en caso que sean iguales */
int isDir(const char *path);
int isFile(const char *path);
float calcArithmeticAverage(float a, float b);
char* const* createNArgvList(int argc, ...);
struct Config_Info_Node * createConfigFileStruct(const char *ConfigFilePath);
void getInfo(struct Config_Info_Node *ConfigInfoList,const char *ConfigId, char *ConfigInfo);
void freeConfigInfoList(struct Config_Info_Node *ConfigInfoList);
struct Config_Info_Node * createConfigInfoNode(char *ConfigId, char *ConfigInfo);
void removeNewLineJumps(char *String);

#endif
