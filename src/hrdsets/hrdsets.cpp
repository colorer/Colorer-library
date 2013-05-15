#define STRICT
#include<io.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<string.h>
#include<windows.h>
#include<windowsx.h>
#include<commdlg.h>

#include<colorer/editor/BaseEditor.h>
#include<misc/registry.cpp>
#include"resource.h"

ParserFactory *parserFactory;
StyledMapper *regionMapper;

HFONT fview;
LOGFONT lf;

int maxreg, cfore, cback;
int curregion = -1;
int console = 0;
char fname[512];
HWND hDialog, hview, hrdList;
HINSTANCE hi;
int concolors[16];

BOOL WINAPI DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI ViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hi, HINSTANCE hpi, char *cmd, int show){
  ::hi = hi;

  parserFactory = new ParserFactory();
  regionMapper = parserFactory->createStyledMapper(DString("rgb"), DString("default"));

  // reading default console colors
  HKEY creg = rOpenKey(HKEY_CURRENT_USER, "console");
  for(int i = 0; i < 16; i++){
    unsigned col = i;
    unsigned tmp, mul = 0x80;
    if (col&8) mul = 0xFF;
    tmp = ((col&4)?mul:00) + ((col&2)?mul<<8:00) + ((col&1)?mul<<16:00);
    if ((col&8) && !tmp) tmp = 0x404040;
    concolors[i] = tmp;
    if (creg != INVALID_HANDLE_VALUE){
      char cval[20];
      sprintf(cval, "ColorTable%02d", i);
      DWORD dlen = 4;
      if (RegQueryValueEx(creg, cval, 0, NULL, (PBYTE)&col, &dlen) == ERROR_SUCCESS)
        concolors[i] = col;
    };
  };

  DialogBox(hi, MAKEINTRESOURCE(IDD_HRD), NULL, DialogProc);

  delete regionMapper;
  delete parserFactory;
  return 0;
};


bool view()
{
struct stat fst;
SCROLLINFO si;
  int file = open(fname, O_BINARY);
  if (file == -1){
    return false;
  };
  fstat(file, &fst);
  int len = fst.st_size;
  data = new byte[len];
  memset(data, 0, len);
  len = read(file, data, len);

  formatlines(data, len, 1);

  type = cd->selectType(&CDString(strrchr(fname,'\\')+1), &GetLine(0, 0, 0), 0);
  if (!type) return false;
  adbk = adata->getCData(2);

  hlines = clrCreateRegions(totallines);
  cd->loadType(type);
  cc = new CColorer();
  cc->setServices(GetLine, 0, clrAddRegion, 0, 0, hlines);
  cc->setColoringSource(type);
  clrSetFirstLine(hlines, 0);
  cc->quickParse(0, totallines);

  clrAssignRegions(hlines, adata, adbk);
  clrCompactRegions(hlines);
  topline = 0;
  close(file);

  maxpixpos = 0;
  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  si.nMax = totallines+20;
  si.nMin = 0;
  si.nPage = 20;
  si.nPos = 0;
  si.nTrackPos = 0;
  SetScrollInfo(hview, SB_VERT, &si, TRUE);
  InvalidateRect(hview, 0, 0);
  return true;
};

void review(int reparse)
{
  adbk = adata->getCData(2);
  if (reparse){
    clrClearRegions(hlines);
    clrSetFirstLine(hlines, 0);
    cc->quickParse(0, totallines);
  };
  clrAssignRegions(hlines, adata, adbk);
  clrCompactRegions(hlines);
  InvalidateRect(hview, 0, 0);
};

void unview()
{
  delete cc;
  clrDestroyRegions(hlines);
  delete[] lines;
  delete data;
  delete file_data;
};


void fillList(HWND hrdList){
bool bl = false;
PAssignData ad;
  ComboBox_ResetContent(hrdList);
  int i, no;
  for(int i = 0, no = 0;; i++){
    StyledRegion = regionMapper->enumerateRegionDefines
    ad = adata->getCData(i);
    if (!ad || !ad->bvalue){
      if (!bl){
        ComboBox_AddString(hrdList, "");
        ComboBox_SetItemData(hrdList, no, -1);
        no++;
      };
      bl = true;
      continue;
    };
    bl = false;
    ComboBox_AddString(hrdList, ad->descr->getBytes());
    ComboBox_SetItemData(hrdList, no, i);
    no++;
  };
};

int _getcolor(int col)
{
  if (col < 0 || col > 15) return col;
  return concolors[col];
};

int getcolor(int col)
{
  if (!console) return col;
  return _getcolor(col);
};

void updateinfo()
{
int j;
char text[0x20];
bool bl = false;
PAssignData ad;

  j = ComboBox_GetCurSel(hrdList);
  cfore = cback = -1;
  if (j == -1) return;

  curregion = ComboBox_GetItemData(hrdList, j);
  ad = adata->getCData(curregion);
  cfore = ad->fore;
  cback = ad->back;
  sprintf(text, "0x%06X", cfore);
  SetWindowText(GetDlgItem(hDialog, IDC_FORE), text);
  sprintf(text, "0x%06X", cback);
  SetWindowText(GetDlgItem(hDialog, IDC_BACK), text);
  sprintf(text, "%d", curregion);
  SetWindowText(GetDlgItem(hDialog, IDC_NUM), text);
  cfore = getcolor(cfore);
  cback = getcolor(cback);

  if (!ad->bfore){
    cfore = -1;
    SetWindowText(GetDlgItem(hDialog, IDC_FORE), "default");
  };
  if (!ad->bback){
    cback = -1;
    SetWindowText(GetDlgItem(hDialog, IDC_BACK), "default");
  };
  InvalidateRect(GetDlgItem(hDialog, IDC_FORE), 0, 0);
  InvalidateRect(GetDlgItem(hDialog, IDC_BACK), 0, 0);
  InvalidateRect(GetDlgItem(hDialog, IDC_SAMPLE), 0, 0);
};






BOOL WINAPI DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
int i, col;
bool bl = false;
char text[0x200];
LPDRAWITEMSTRUCT ds;
POINT pt;
HBRUSH br;
HMENU menu;
WNDCLASS wc;
OPENFILENAME ofn;
CHOOSECOLOR cc;
static COLORREF ccustc[16];
PAssignData ad;

  switch(msg){
    case WM_INITDIALOG:
      hDialog = hwnd;
      hrdList = GetDlgItem(hwnd, IDC_REGIONS);
      fillList(hrdList);
      memset(&wc, 0, sizeof(wc));
      wc.hInstance = hi;
      wc.lpszClassName = "view";
      wc.lpfnWndProc = ViewProc;
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.hCursor = LoadCursor(0,IDI_APPLICATION);
      wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
      RegisterClass(&wc);

      CreateWindow("view", "sample view", WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL,
        140, 285, 650, 310, 0, 0, hi, 0);

      memset(&lf, 0, sizeof(LOGFONT));

      updateinfo();
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam)){
        case IDC_REGIONS:
          updateinfo();
          break;
        case IDC_CONSOLE:
          console = Button_GetCheck((HWND)lParam);
          updateinfo();
          InvalidateRect(hview, 0, 0);
          break;
        case IDC_LOADSAMPLE:
          *fname = 0;
          memset(&ofn, 0, sizeof(ofn));
          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hwnd;
          ofn.nMaxFile = 512;
          ofn.lpstrFile = fname;
          ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
          ofn.lpstrFilter = "all files\0*.*\0\0";
          if (!GetOpenFileName(&ofn)) break;
          unview();
          view();
          break;
        case IDC_LOAD:
          *hrdpath = 0;
          memset(&ofn, 0, sizeof(ofn));
          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hwnd;
          ofn.nMaxFile = 512;
          ofn.lpstrFile = hrdpath;
          rGetValue(creg, "hrd_path", text, 0x200);
          ofn.lpstrInitialDir = text;
          ofn.lpstrFilter = "hrd files\0*.hrd\0all files\0*.*\0\0";
          ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
          if (!GetOpenFileName(&ofn)) break;
          adata->load(hrdpath);
          fillList(hrdList);
          review(0);
          break;
        case IDC_REPARSE:
          review(1);
          break;
        case IDC_SAVE:
          *hrdpath = 0;
          memset(&ofn, 0, sizeof(ofn));
          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hwnd;
          ofn.lpstrFilter = "hrd files\0*.hrd\0all files\0*.*\0\0";
          ofn.nMaxFile = 512;
          ofn.lpstrFile = hrdpath;
          rGetValue(creg, "hrd_path", text, 0x200);
          ofn.lpstrInitialDir = text;
          ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
          if (!GetSaveFileName(&ofn)) break;
          adata->save(hrdpath);
          break;
        case IDC_FORE:
          bl = true;
        case IDC_BACK:
          if (curregion == -1) break;
          menu = LoadMenu(hi, MAKEINTRESOURCE(IDR_COLORMENU));
          GetCursorPos(&pt);
          i = TrackPopupMenu(GetSubMenu(menu, 0), TPM_RIGHTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
            pt.x, pt.y, 0, hwnd,0);
          DestroyMenu(menu);
          if (!i) break;
          i -= ID_BLACK;
          ad = adata->getCData(curregion);
          if (!ad) break;
          if (i < 0x10){
            if (bl){
              ad->bfore = true;
              ad->fore = console ? i :_getcolor(i);
            }else{
              ad->bback = true;
              ad->back = console ? i :_getcolor(i);
            };
          }else if (i == 0x10){
            if (bl) ad->bfore = false;
            else ad->bback = false;
          }else{
            memset(&cc, 0, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.rgbResult = getcolor(bl ? ad->fore : ad->back);
            cc.Flags = CC_SOLIDCOLOR|CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
            cc.lpCustColors = ccustc;
            if (!ChooseColor(&cc)) break;
            if (bl){
              ad->bfore = true;
              ad->fore = cc.rgbResult;
            }else{
              ad->bback = true;
              ad->back = cc.rgbResult;
            };
          };
          review(0);
          updateinfo();
          break;
        case IDC_FONT:
          {
            char fname[256];
            fname[0] = 0;
            CHOOSEFONT cf;
            memset(&cf, 0, sizeof(CHOOSEFONT));
            cf.lStructSize = sizeof(CHOOSEFONT);
            cf.lpLogFont = &lf;
            cf.hwndOwner = hwnd;
            cf.Flags = CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
            BOOL ok = ChooseFont(&cf);
            if (ok){
              HFONT fview_new = CreateFontIndirect(&lf);
              if (fview_new){
                DeleteObject(fview);
                fview = fview_new;
              };
              InvalidateRect(hview, 0, 0);
            };
          };
          break;
        case IDCLOSE:
          SendMessage(hwnd, WM_CLOSE, 0, 0);
      };
      break;
    case WM_DRAWITEM:
      ds = (LPDRAWITEMSTRUCT)lParam;
      if (ds->itemAction != ODA_DRAWENTIRE) break;

      i = GetWindowText(ds->hwndItem, text, 128);
      col = ds->CtlID == IDC_FORE ? cfore : cback;
      if (col == -1) col = 0xFFFFFF;

      if (ds->CtlID == IDC_SAMPLE){
        if (cfore == -1) cfore = getcolor(adbk->fore);
        if (cback == -1) cback = getcolor(adbk->back);
        br = CreateSolidBrush(cback);
        SelectObject(ds->hDC, br);
        Rectangle(ds->hDC, ds->rcItem.left, ds->rcItem.top, ds->rcItem.right, ds->rcItem.bottom);
        DeleteObject(br);
        SetBkColor(ds->hDC, cback);
        SetTextColor(ds->hDC, cfore);
        DrawText(ds->hDC, text, i, &ds->rcItem, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        return 0;
      };

      br = CreateSolidBrush(col);
      SelectObject(ds->hDC, br);
      Rectangle(ds->hDC, ds->rcItem.left, ds->rcItem.top, ds->rcItem.right, ds->rcItem.bottom);
      DeleteObject(br);

      SetBkColor(ds->hDC, col);
      SetTextColor(ds->hDC, ~col & 0xFFFFFF);
      DrawText(ds->hDC, text, i, &ds->rcItem, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
      break;
    case WM_CLOSE:
      SendMessage(hview, WM_DESTROY, 0, 0);
      PostQuitMessage(0);
      break;
  };
  return FALSE;
};

LRESULT WINAPI ViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
PAINTSTRUCT ps;
HDC dc;
HBRUSH br;
HPEN pen;
RECT rc;
int i, x, y, col, pos;
static int height, width;
TEXTMETRIC tm;
SCROLLINFO si;

  switch(msg){
    case WM_CREATE:
      hview = hwnd;
      if (!view()){
        MessageBox(0, "can't find mancolorer sample file", "error", 0);
        ExitProcess(0);
      };
      fview = CreateFont(16,0,0,0,0,0,0,0,DEFAULT_CHARSET,0,0,0,0,"Lucida Console");
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      DeleteObject(fview);
      unview();
      break;
    case WM_KEYDOWN:
      if (wParam == VK_UP) SendMessage(hwnd, WM_VSCROLL, SB_LINELEFT, 0);
      if (wParam == VK_DOWN) SendMessage(hwnd, WM_VSCROLL, SB_LINERIGHT, 0);
      if (wParam == VK_LEFT) SendMessage(hwnd, WM_HSCROLL, SB_LINELEFT, 0);
      if (wParam == VK_RIGHT) SendMessage(hwnd, WM_HSCROLL, SB_LINERIGHT, 0);
      if (wParam == VK_PRIOR) SendMessage(hwnd, WM_VSCROLL, SB_PAGELEFT, 0);
      if (wParam == VK_NEXT) SendMessage(hwnd, WM_VSCROLL, SB_PAGERIGHT, 0);
      break;
    case 0x020A:
      {
        int num = (short)HIWORD(wParam)/120;
        topline -= num*2;
        if (topline > totallines) topline = totallines;
        if (topline < 0) topline = 0;
      };
      si.cbSize = sizeof(si);
      si.fMask = SIF_POS;
      si.nPos = topline;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
      InvalidateRect(hwnd, 0,0);
      break;
    case WM_VSCROLL:
      si.cbSize = sizeof(si);
      si.fMask = SIF_POS;
      switch((int) LOWORD(wParam)){
        case SB_LINELEFT:
          if (topline) topline--;
          break;
        case SB_LINERIGHT:
          if (topline < totallines) topline++;
          break;
        case SB_PAGERIGHT:
          topline += 20;
          if (topline > totallines) topline = totallines;
          break;
        case SB_PAGELEFT:
          topline -= 20;
          if (topline < 0) topline = 0;
          break;
        case SB_THUMBTRACK:
          topline = HIWORD(wParam);
          break;
      };
      si.nPos = topline;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
      InvalidateRect(hwnd, 0,0);
      break;
#define HORZ_MOVE 10
    case WM_HSCROLL:
      switch((int) LOWORD(wParam)){
        case SB_LINELEFT:
          leftpos-=HORZ_MOVE;
          if (leftpos < 0) leftpos = 0;
          break;
        case SB_LINERIGHT:
          if (leftpos < maxpixpos) leftpos+=HORZ_MOVE;
          break;
        case SB_PAGERIGHT:
          leftpos += HORZ_MOVE*20;
          if (leftpos > maxpixpos) leftpos = maxpixpos;
          break;
        case SB_PAGELEFT:
          leftpos -= HORZ_MOVE*20;
          if (leftpos < 0) leftpos = 0;
          break;
        case SB_THUMBTRACK:
          leftpos = HIWORD(wParam);
          break;
      };
      si.cbSize = sizeof(si);
      si.fMask = SIF_POS;
      si.nPos = leftpos;
      SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
      InvalidateRect(hwnd, 0,0);
      break;
    case WM_PAINT:
      dc = BeginPaint(hwnd, &ps);

      SelectObject(dc, fview);
      GetClientRect(hwnd, &rc);

      col = getcolor(adbk->back);
      br = CreateSolidBrush(col);
      pen = CreatePen(PS_NULL, 0, 0);
//printf("%d:%d\n", leftpos, topline);
      GetTextMetrics(dc, &tm);
      width = tm.tmMaxCharWidth;
      height = tm.tmHeight;

      SelectObject(dc, br);
      SelectObject(dc, pen);

      Rectangle(dc, rc.left, rc.top, rc.right+1, rc.bottom+1);

      for(i = topline; i < totallines; i++){
        pos = 0;
        int pixpos = 0;
        SIZE sz;
        for(l1 = hlines[i+1].next; l1; l1 = l1->next){
          if (l1->end == -1) l1->end = lines[i].data->length();
          if (l1->ad.type > 0xFF) continue;
          if (l1->start > pos){
            TextOutW(dc, pixpos-leftpos, (i-topline)*height, (wchar_t*)lines[i].data->getWChars() + pos, l1->start - pos);
            pos = l1->start;
            GetTextExtentPoint32W(dc, (wchar_t*)lines[i].data->getWChars() + pos, l1->start - pos, &sz);
            pixpos += sz.cx;
          };
          SetTextColor(dc, getcolor(l1->ad.fore));
          SetBkColor(dc, getcolor(l1->ad.back));
          TextOutW(dc, pixpos-leftpos, (i-topline)*height, (wchar_t*)lines[i].data->getWChars() + pos, l1->end - l1->start);
          GetTextExtentPoint32W(dc, (wchar_t*)lines[i].data->getWChars() + pos, l1->end - l1->start, &sz);
          pos += l1->end - l1->start;
          pixpos += sz.cx;
        };
        if (maxpixpos < pixpos-rc.right) maxpixpos = pixpos-rc.right;
        if (pos < lines[i].data->length()){
          TextOutW(dc, pixpos-leftpos, (i-topline)*height, (wchar_t*)lines[i].data->getWChars() + pos, lines[i].data->length() - pos);
        };

        if ((i-topline)*height > rc.bottom) break;
      };
      si.fMask = SIF_ALL;
      si.nMax = maxpixpos+200;
      si.nMin = 0;
      si.nPage = 200;
      si.nPos = leftpos;
      si.nTrackPos = 0;
      SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

      DeleteObject(br);
      DeleteObject(pen);
      EndPaint(hwnd, &ps);
      break;
    case WM_LBUTTONDOWN:
      x = LOWORD(lParam);
      y = HIWORD(lParam);

      i = topline + (y / height);
      if (i >= totallines) break;
      for(l1 = hlines[i+1].next; l1; l1 = l1->next){
        if (l1->start*width <= x && l1->end*width >= x){
          ComboBox_SelectString(hrdList, -1, l1->ad.descr->getBytes());
          updateinfo();
          break;
        };
      };
      break;

  };
  return DefWindowProc(hwnd, msg, wParam, lParam);
};
