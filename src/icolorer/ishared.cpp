
// This part describes registry key's names, and reading function.
// now colorer doesn't support internally this interface - you can
// compile colorint library yourself...
#ifndef __SHAREDDEFS__
#define __SHAREDDEFS__

#include<windows.h>

char rSharedKey[]    ="Software\\colorer\\4ever";
char rLibrary[]      ="library";
char rColorBaseHrc[] ="hrc";

BOOL GetLibraryPath(char *path, char*regName)
{
HKEY key;
int i;
DWORD len=255;
  RegCreateKeyEx(HKEY_CURRENT_USER,rSharedKey,0,NULL,REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS,NULL,&key,NULL);
  if (!key) return FALSE;
  i = RegQueryValueEx(key,regName,0,NULL,(PBYTE)path,&len);
  RegCloseKey(key);
  if (i) return FALSE;
  return TRUE;
};
#endif
