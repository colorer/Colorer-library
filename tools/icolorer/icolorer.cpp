#include<colorer/classes.h>
#include"icolorer.h"

PColorData WINAPI cdCreate(char *path, char *el)
{
PColorData tmp = new CColorData(CDString(path), el?&CDString(el):null);
  if (tmp->isOk()) return tmp;
  delete tmp;
  return NULL;
};
PColorData WINAPI cdCreateIndirect(const byte *data, int len, char *el)
{
PColorData tmp = new CColorData(data, len, el?&CDString(el):null);
  if (tmp->isOk()) return tmp;
  delete tmp;
  return NULL;
};
BOOL WINAPI cdDestroy(PColorData pcd)
{
  if (!pcd) return FALSE;
  delete pcd;
  return TRUE;
};
unsigned int WINAPI cdGetRegion(PColorData pcd, char *name)
{
  return pcd->getRegion(&CDString(name));
};
PType WINAPI cdEnumTypes(PColorData pcd, PType next)
{
  return pcd->enumTypes(next);
};
PType WINAPI cdSelectType(PColorData pcd, char *fname, char *fstline, int no = 0)
{
  return pcd->selectType(&CDString(fname), &CDString(fstline), no);
};
BOOL  WINAPI cdLoadType(PColorData pcd, PType type)
{
  return pcd->loadType(type);
};
const char* WINAPI cdGetVersion(PColorData pcd)
{
  return pcd->getVersion()->getBytes();
};
const char* WINAPI cdGetSchemeName(PColorData pcd, PScheme scheme)
{
  return pcd->getSchemeName(scheme)->getBytes();
};

PColorer WINAPI ccCreate()
{
  return new CColorer();
};
BOOL WINAPI ccDestroy(PColorer pc)
{
  if (!pc) return FALSE;
  delete pc;
  return TRUE;
};
BOOL WINAPI ccClearCache(PColorer pc)
{
  if (!pc) return FALSE;
  return pc->clearCache();
};
void WINAPI ccBreakParse(PColorer pc)
{
  if (!pc) return;
  pc->breakParse();
};

static GETLINEPROC_OLD glp = null;

CDString getLine(void *param, CColorer *who, int lno)
{
  int len;
  char * ptr = glp(param, who, lno, &len);
  return CDString(ptr, 0, len);
};

BOOL WINAPI ccSetServices(PColorer pc, GETLINEPROC_OLD gl, VISREADYPROC vr, ADDREGIONPROC ac,
                   void*_glparam, void*_vrparam, void*_acparam)
{
  if (!pc) return FALSE;
  glp = gl;
  return pc->setServices(getLine, vr, ac, _glparam, _vrparam, _acparam);
};
BOOL WINAPI ccSetColoringSource(PColorer pc, PType type)
{
  if (!pc) return FALSE;
  return pc->setColoringSource(type);
};
int WINAPI ccQuickParse(PColorer pc, int from, int num)
{
  if (!pc) return 0;
  return pc->quickParse(from, num);
};
int WINAPI ccFullParse(PColorer pc, int from, int fvis, int visnum, int allnum)
{
  if (!pc) return 0;
  return pc->fullParse(from, fvis, visnum, allnum);
};

PLineHL WINAPI crCreateRegions(int lnum)
{
  return clrCreateRegions(lnum);
};
BOOL WINAPI crDestroyRegions(PLineHL lines)
{
  return clrDestroyRegions(lines);
};
BOOL WINAPI crSetFirstLine(PLineHL lines, int fst)
{
  return clrSetFirstLine(lines, fst);
};

//void clrAddRegion(void *param, PColorer who, int lno, int sx, int ex, int region, int type);

void WINAPI crClearRegions(PLineHL lines)
{
  clrClearRegions(lines);
};
BOOL WINAPI crAssignRegions(PLineHL lines, PAssign adata, PAssignData back)
{
  return clrAssignRegions(lines, adata, back);
};
BOOL WINAPI crCompactRegions(PLineHL lines)
{
  return clrCompactRegions(lines);
};

// CAssign class methods
PAssign WINAPI caCreate()
{
  return new CAssign();
};
BOOL WINAPI caDestroy(PAssign pa)
{
  if (!pa) return FALSE;
  delete pa;
  return TRUE;
};
BOOL WINAPI caLoad(PAssign pa, char *fname)
{
  if (!pa) return FALSE;
  return pa->load(fname);
};
BOOL WINAPI caSave(PAssign pa, char *fname)
{
  if (!pa) return FALSE;
  return pa->save(fname);
};
PAssignData WINAPI caGetCData(PAssign pa, int value)
{
  if (!pa) return NULL;
  return pa->getCData(value);
};
BOOL WINAPI caSetCData(PAssign pa, int value, PAssignData pad)
{
  if (!pa) return FALSE;
  return pa->setCData(value, pad);
};
int WINAPI caMaxRegion(PAssign pa)
{
  return pa->maxRegion();
};

// CSgml class methods
PXmlEl WINAPI csNext(PXmlEl el)
{
  if (!el) return 0;
  return el->next();
};
PXmlEl WINAPI csPrev(PXmlEl el)
{
  if (!el) return 0;
  return el->prev();
};
PXmlEl WINAPI csChild(PXmlEl el)
{
  if (!el) return 0;
  return el->child();
};
PXmlEl WINAPI csParent(PXmlEl el)
{
  if (!el) return 0;
  return el->parent();
};
const char* WINAPI csGetName(PXmlEl el)
{
  if (!el) return 0;
  return el->getName()->getBytes();
};
const char* WINAPI csGetParam(PXmlEl el, int no)
{
  if (!el) return 0;
  return el->getParamValue(no)->getBytes();
};
const char* WINAPI csGetChrParam(PXmlEl el, char *par)
{
  if (!el) return 0;
  return el->getParamValue(CDString(par))->getBytes();
};
BOOL WINAPI csGetIntParam(PXmlEl el, char *par, int *result)
{
  if (!el) return FALSE;
  return el->getParamValue(CDString(par), result);
};
BOOL WINAPI csGetFltParam(PXmlEl el, char *par, double *result)
{
  if (!el) return FALSE;
  return el->getParamValue(CDString(par), result);
};

BOOL WINAPI cdLoadFile(PColorData pcd, char* file)
{
  if (!pcd) return FALSE;
  return pcd->loadFile(&CDString(file));
};


//
SColorerClass scc = {
  sizeof(scc),
  cdCreate, cdCreateIndirect,  cdDestroy,
  cdGetRegion, cdEnumTypes, cdSelectType, cdLoadType, cdGetVersion,
  cdGetSchemeName,

  ccCreate,  ccDestroy, ccClearCache, ccBreakParse,
  ccSetServices, ccSetColoringSource,
  ccQuickParse, ccFullParse,

  crCreateRegions, crDestroyRegions,
  crSetFirstLine,
  clrAddRegion, // note! there is no stub for it! this is not WINAPI function!
  crClearRegions,
  crAssignRegions, crCompactRegions,

  caCreate, caDestroy,
  caLoad, caSave,
  caGetCData, caSetCData,
  caMaxRegion,

  csNext, csPrev,
  csChild, csParent, csGetName,
  csGetParam, csGetChrParam,
  csGetIntParam, csGetFltParam,

  cdLoadFile,
};

extern "C" BOOL WINAPI _export GetColorerProc(PColorerClass pcc)
{
  if (!pcc) return FALSE;
  int sz = pcc->sz;
  if (scc.sz < sz) sz = scc.sz;
  MoveMemory(pcc, &scc, sz);
  pcc->sz = sz;
  return TRUE;
};
