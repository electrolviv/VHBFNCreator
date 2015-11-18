#include "main.h"

// System Dialog Selected
extern LOGFONT lf;

// Generated font content
stCharProps sSymProps[256];
vector<u8> arrCharacters[256];

// Step Properties
int nXStart = 40;
int nYStart = 40;
int stepX = 32;
int stepY = 32;

static int GetSymbolStartX(int symn) { int i = symn % 16; return nXStart + (i*stepX); }
static int GetSymbolStartY(int symn) { int j = symn/16; return nYStart + (j*stepY); }

static void DrawRectangle(HWND hwnd, HDC hDC);
static void Draw256Chars(HDC hDC, HFONT hCustomFont);
static vector<u8> ConvertChar(int n, HWND hwnd, HDC hDC);
static void AnalyzeChar( vector<u8> arr, stCharProps *pResult ); // arrCharacter
static void SaveBFN2();

// ------------------
// Main Procedure ...
// ------------------
void DrawSymbols(HWND hwnd, HFONT hCustomFont) {
     
  HDC hDC = GetWindowDC(hwnd);
    
  // 1. Clear Screen
  DrawRectangle(hwnd, hDC);
 
 // 2. Draw characters
 Draw256Chars(hDC, hCustomFont);
 
 // 3. Convert characters
 for(int i=0;i<256;i++) {
	 stCharProps *props = &sSymProps[i];
	 arrCharacters[i] = ConvertChar(i, hwnd, hDC);
	 AnalyzeChar(arrCharacters[i], props);
 }

 ReleaseDC(hwnd, hDC);

 // 4. Save to file
 SaveBFN2();

 return;
}

//  Clear Screen
static void DrawRectangle(HWND hwnd, HDC hDC) {
  HBRUSH hNewBrush = CreateSolidBrush( RGB(0,0,0) );
  HBRUSH hOldBrush = (HBRUSH)SelectObject( hDC, hNewBrush );
  RECT rect;
  GetWindowRect( hwnd, &rect );
  Rectangle(hDC, 0,0, rect.right - rect.left, rect.bottom - rect.top );
  SelectObject( hDC, hOldBrush );
  DeleteObject( hNewBrush ); 
}

// int iWidth  = sFont.iWidths[iN];
// int iHeight = sFont.iHeight;

// Draw symbols
static void Draw256Chars(HDC hDC, HFONT hCustomFont) {

 HFONT hfntDefault;
 hfntDefault = (HFONT)SelectObject( hDC, hCustomFont); 

 SetTextColor(hDC, RGB(255,255,255));
 SetBkMode(hDC, TRANSPARENT);

 // Draw Graphic
 for( int j=0;j<16;j++) {
  for( int i=0;i<16;i++) {
       int iN = (j*16)+i; unsigned char sym[2] = { iN, 0 };
       TextOut( hDC, GetSymbolStartX(iN), GetSymbolStartY(iN), (char *)sym, 1 );
   }
 }   

 SelectObject( hDC, hfntDefault ); 
 DeleteObject( hCustomFont ); 
 
}

u8 ConvertColor(COLORREF color) {

	float gray = 0;

	if (color) {
		gray = 1; 
	}

	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);

	gray = 0.2125f * r + 0.7154f * g + 0.0721f * b;
	u8 result = (u8)gray;
	return result;
}

vector<u8> result;

static vector<u8> ConvertChar(int n, HWND hwnd, HDC hDC) {
	int sx = GetSymbolStartX(n);
	int sy = GetSymbolStartY(n);
	
	result.clear();
	
	for(int y=0; y<stepY;y++) { 
		for(int x=0; x<stepX;x++) { 
			COLORREF clr = GetPixel(hDC, sx + x, sy + y);
			result.push_back(ConvertColor(clr));
		}
	}

	return result;
}

u8 templine[32];
#define FASTDAT(X,Y) arr[((Y)*32)+X]

static int scanarr(u8 *ptr) { for(int i=0;i<32;i++) { if(ptr[i]) return 1; } return 0; }

static void AnalyzeChar( vector<u8> arr, stCharProps *pResult ) {

     int top = -1, left, right, bottom;
       
     // Find offset from top
	 for (int z = 0; z < 32; z++) { for (int i = 0; i < 32; i++) templine[i] = FASTDAT(i, z); if (scanarr(templine)) { top = z; z = 32; } }

	 // Find offset from bottom
	 for (int z = 0; z < 32; z++) { for (int i = 0; i < 32; i++) templine[i] = FASTDAT(i, 31 - z); if (scanarr(templine)) { bottom = z; z = 32; } }

     // Find offset from left
	 for (int z = 0; z < 32; z++) { for (int i = 0; i < 32; i++) templine[i] = FASTDAT(z, i); if (scanarr(templine)) { left = z; z = 32; } }
	 
     // Find offset from right
	 for (int z = 0; z < 32; z++) { for (int i = 0; i < 32; i++) templine[i] = FASTDAT(31 - z, i); if (scanarr(templine)) { right = z; z = 32; } }

	 if (top == -1) {
		 pResult->offs_top = 0;
		 pResult->offs_left = 0;
		 pResult->charw = 0;
		 pResult->charh = 0;
	 }
	 else {
		 pResult->offs_top = top;
		 pResult->offs_left = left;
		 pResult->charw = 32 - left - right;
		 pResult->charh = 32 - top - bottom;
	 }

}

static void SaveBFN2() {

 char szFontFileBIN[100];

 char *pName = lf.lfFaceName;
 int namelen = strlen(pName);
 if (namelen > 30) namelen = 30;

 sprintf(szFontFileBIN, "fnt_%s_%d.bf2", pName, abs(lf.lfHeight) );
 FILE *hFILEBIN = fopen( szFontFileBIN, "wb" );

 // Store Header
 stBFN2HDR sHDR;
 memcpy(sHDR.szName, pName, namelen);
 sHDR.szZ = 0; sHDR.attrs = 0;
 fwrite(&sHDR, sizeof(stBFN2HDR), 1, hFILEBIN);

 // Store characters meta
 stBFN2CHR sCHR[256];
 int offset = sizeof(stBFN2HDR) + (256 * sizeof(stBFN2CHR));
 
 sCHR[0].offs = offset;
 sCHR[0].t = sSymProps[0].offs_top;
 sCHR[0].w = sSymProps[0].charw;
 sCHR[0].h = sSymProps[0].charh;
 sCHR[0].res = 0;
 
 offset += sCHR[0].w * sCHR[0].h;

 for (int i = 1; i < 256; i++) {
	 sCHR[i].offs = offset;
	 sCHR[i].t = sSymProps[i].offs_top;
	 sCHR[i].w = sSymProps[i].charw;
	 sCHR[i].h = sSymProps[i].charh;
	 sCHR[i].res = 0;
	 offset += sCHR[i].w * sCHR[i].h;
 }

 fwrite(&sCHR[0], sizeof(stBFN2CHR) * 256, 1, hFILEBIN);

 // Store characters as byte-field 
 for (int z = 0; z < 256; z++) {

	 u8 szLineY[128];

	 for (int y = 0; y < sSymProps[z].charh; y++) {

		 for (int x = 0; x < sSymProps[z].charw; x++)
		 {
			 vector<u8> chr = arrCharacters[z];
			 int sx = sSymProps[z].offs_left + x;
			 int sy = sSymProps[z].offs_top  + y;
			 int o = (sy * stepX) + sx;
			 szLineY[x] = chr[o];
		 }

		 fwrite(&szLineY[0], sSymProps[z].charw, 1, hFILEBIN);
	 }

 }
  
 fclose( hFILEBIN );
}


// uchar8 ucHeight = sFont.iHeight;
// fwrite( &ucHeight, 1, 1, hFILEBIN);
// for( int i=0;i<256;i++ ) { uchar8 ucWidth = sFont.iWidths[i]; fwrite( &ucWidth, 1, 1, hFILEBIN);  }
// for( int i=0;i<256;i++ ) { uint16 uiAddr = sFont.iOffset[i]; fwrite( &uiAddr, 2, 1, hFILEBIN); }
/*  uchar8 szSybol[100]; uchar8 ucBytes = 1; uchar8 ucMask = 0x80;  uint16 uiAddr = sFont.iOffset[i]; memset( szSymbol, 0, 100 ); */

/*
for( int i=0;i<256;i++ ) {
int iLen    = sFont.iWidths[i] * sFont.iHeight;
int iOffset = sFont.iOffset[i];
fwrite( &sFont.szFontData[iOffset], iLen, 1, hFILEBIN);
}
*/

// stFONTHDR sFont;
// char *szFONTNAME = "VHFONT1";
// char szFontFileCPP[100];
// char tmp[10];
// vector<string> szGenerated;


// int iColor = 255 - ((GetPixel( hDC, iX + x, iY + y )>>8)&0xFF); sprintf( tmp, "0x%.2X", iColor ); szResult += tmp; sFont.szFontData[iBaseOffset++] = iColor;
// if( ( y == (iHeight-1) ) && ( x == (iWidth-1)) ) { }
// else szResult += ",";
// else szResult += "\n\t\t";


/*
sFont.iHeight = 8;

// GetCharWidth32( hDC, 0, 255, sFont.iWidths );
int iBaseOffset = 0;

for( int i=0;i<256;i++ ) { sFont.iOffset[i] = iBaseOffset; iBaseOffset += sFont.iWidths[i]*sFont.iHeight; }
iBaseOffset = 0;

sprintf( szFontFileCPP, "%s.hff", szFONTNAME );
FILE *hFILECPP = fopen( szFontFileCPP, "wt" );
*/

// Appending characters width
/*
{ string szText = "const unsigned char ";
szText += szFONTNAME;
szText += "_Width[256] = {\n\t\t";

for( int i=0; i< 256; i++ )
{
sprintf( tmp, "%d", sFont.iWidths[i] );
szText += tmp;
if( i != 255 ) szText += ",";
if( i%16 == 15 ) szText += "\n\t\t";
}

szText += "};\n\n";
fwrite( szText.c_str(), szText.size(), 1, hFILECPP );
}
*/
/*
char szFont[100];
sprintf( szFont, "%s_Symbol_%.3d", szFONTNAME, iN );

string szResult = "char ";
szResult += szFont;
szResult += "[";
sprintf( tmp, "%d", iWidth * 8 ); szResult += tmp;
szResult += "] = {\n\t\t";

for(int y=0; y<iHeight;y++) {
for(int x=0; x<iWidth;x++) {
int iColor = 255 - ((GetPixel( hDC, iX + x, iY + y )>>8)&0xFF);
sprintf( tmp, "0x%.2X", iColor ); szResult += tmp;
sFont.szFontData[iBaseOffset++] = iColor;
if( ( y == (iHeight-1) ) && ( x == (iWidth-1)) ) { }
else szResult += ",";
}
szResult += "\n\t\t";
}

szResult += "\n};\n\n";
*/

// fwrite( szResult.c_str(), szResult.size(), 1, hFILECPP );
// szGenerated.push_back( szResult );


/*
// Char Marker
SetPixel( hDC, iX,iY, RGB(255,0,0) );
SetPixel( hDC, iX+iWidth,iY, RGB(255,0,0) );
SetPixel( hDC, iX+iWidth,iY+iHeight, RGB(255,0,0) );
SetPixel( hDC, iX,iY+iHeight, RGB(255,0,0) );
*/

// sprintf( tmp, "\nchar *%s_PTR[256] = {", szFONTNAME );
// fwrite( tmp, strlen(tmp), 1, hFILECPP );

// Generating cross-reference
/*
for( int i=0;i<256;i++)
{
sprintf( tmp, "\n\t\t(char *)&%s_Symbol_%.3d%s", szFONTNAME, i, (i==255)? "" : "," );
fwrite( tmp, strlen(tmp), 1, hFILECPP );
}

sprintf( tmp, "\n\t};\n\n" );
fwrite( tmp, strlen(tmp), 1, hFILECPP);

fclose( hFILECPP );
*/
