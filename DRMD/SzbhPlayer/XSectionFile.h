

#ifndef __X_SECTION_FILE_H__
#define __X_SECTION_FILE_H__


int XSectionSaveValue(const char *pInputFileName, const char *pInputSectionName, const char *pInputKey, char *pInputVal);
int XSectionGetValue(const char *pInputFileName, const char *pInputSectionName, const char *pInputKey, char *pOutputVal, int tInputDataLen);


#endif

