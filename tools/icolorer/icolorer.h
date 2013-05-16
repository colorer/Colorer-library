// really, you need to rewrite needed structures,
// if you porting this interface into other languages
#include<colorer/classes.h>
//
#include<windows.h>
//

typedef CColorData *PColorData;
typedef char*(*GETLINEPROC_OLD)(void *param, CColorer *who, int lno, int*len);

typedef struct SColorerClass
{
// size of this structure
  int sz;
// ccdata methods
  PColorData (WINAPI *cdCreate)(char *path, char *el);
  PColorData (WINAPI *cdCreateIndirect)(const byte *data, int len, char *el);
  BOOL       (WINAPI *cdDestroy)(PColorData pcd);
  unsigned int (WINAPI *cdGetRegion)(PColorData pcd, char *name);
  PType        (WINAPI *cdEnumTypes)(PColorData pcd, PType next);
  PType        (WINAPI *cdSelectType)(PColorData pcd, char *fname, char *fstline, int no = 0);
  BOOL         (WINAPI *cdLoadType)(PColorData pcd, PType type);
  const char*  (WINAPI *cdGetVersion)(PColorData pcd);
  const char*  (WINAPI *cdGetSchemeName)(PColorData pcd, PScheme scheme);
// ccolorer methods
  PColorer (WINAPI *ccCreate)();
  BOOL     (WINAPI *ccDestroy)(PColorer pc);
  BOOL     (WINAPI *ccClearCache)(PColorer pc);
  void     (WINAPI *ccBreakParse)(PColorer pc);
  BOOL     (WINAPI *ccSetServices)(PColorer pc, GETLINEPROC_OLD gl, VISREADYPROC vr, ADDREGIONPROC ac,
                   void*_glparam, void*_vrparam, void*_acparam);
  BOOL     (WINAPI *ccSetColoringSource)(PColorer pc, PType type);
  int      (WINAPI *ccQuickParse)(PColorer pc, int from, int num);
  int      (WINAPI *ccFullParse)(PColorer pc, int from, int fvis, int visnum, int allnum);
// clr Color Regions Support methods
  PLineHL  (WINAPI *crCreateRegions)(int lnum);
  BOOL     (WINAPI *crDestroyRegions)(PLineHL lines);
  BOOL     (WINAPI *crSetFirstLine)(PLineHL lines, int fst);
  ADDREGIONPROC crAddRegion;
  void     (WINAPI *crClearRegions)(PLineHL lines);
  BOOL     (WINAPI *crAssignRegions)(PLineHL lines, PAssign adata, PAssignData back);
  BOOL     (WINAPI *crCompactRegions)(PLineHL lines);
// CAssign class methods
  PAssign  (WINAPI *caCreate)();
  BOOL     (WINAPI *caDestroy)(PAssign pa);
  BOOL     (WINAPI *caLoad)(PAssign pa, char *fname);
  BOOL     (WINAPI *caSave)(PAssign pa, char *fname);
  PAssignData (WINAPI *caGetCData)(PAssign pa, int value);
  BOOL     (WINAPI *caSetCData)(PAssign pa, int value, PAssignData pad);
  int      (WINAPI *caMaxRegion)(PAssign pa);
// CSgml class methods
  PXmlEl  (WINAPI *csNext)(PXmlEl el);
  PXmlEl  (WINAPI *csPrev)(PXmlEl el);
  PXmlEl  (WINAPI *csChild)(PXmlEl el);
  PXmlEl  (WINAPI *csParent)(PXmlEl el);
  const char* (WINAPI *csGetName)(PXmlEl el);
  const char* (WINAPI *csGetParam)(PXmlEl el, int no);
  const char* (WINAPI *csGetChrParam)(PXmlEl el, char *par);
  BOOL     (WINAPI *csGetIntParam)(PXmlEl el, char *par, int *result);
  BOOL     (WINAPI *csGetFltParam)(PXmlEl el, char *par, double *result);

  BOOL     (WINAPI *cdLoadFile)(PColorData pcd, char* file);

} *PColorerClass;

// "GetColorerProc" is an exported function, that
// returns pointers to all library functions
typedef BOOL (WINAPI *PColorerProc)(PColorerClass);

//
