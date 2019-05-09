
/*!
 *  \file DibImage.h
 *  \brief CDibImage DIB and .bmp image manipulation and file access persistence.
 */


#ifndef _DIB_IMAGE_H
#define _DIB_IMAGE_H


//////////////////////////////////////////////////////////////////////////////
/*! 
 * \brief Reads and Saves .bmp file into a DIB bitmap, create DIB from HBITMAP, draw to window, scale or invert for<BR/>
 *        upside down format or bitmap.  NOTE: the bitmap m_hBitmap will not refer to data at buffer m_pData,
 *        unless it was read from file, created, or m_hBitmap was created using CreateDIBSection() for example.
 *
 */
//////////////////////////////////////////////////////////////////////////////
class CDibImage
{

	enum {IMAGE_MAX_WIDTH = 1024, IMAGE_MAX_HEIGHT = 768};		// chosen without particular reason
public:

	// Constructors, lots of nice ones.
	CDibImage ( );
	CDibImage(LPTSTR szFilePath);
	CDibImage(HINSTANCE hInstance,DWORD ID);
	CDibImage(int width, int height, LONG size);
	CDibImage(HBITMAP hBitmap);
	CDibImage ( const CDibImage& copy );
	virtual ~CDibImage ( );
	BOOL ReadFromFile (LPTSTR szFilePath);
	bool LoadAttachBitmap(HBITMAP hBitmap);
	bool LoadFromResource(HINSTANCE hInstance,DWORD ID);
	void CreateBitmap(HDC dc, int iWidth, int iHeight);		//!< create bitmap handle and memory with CreateDIBSection()
	bool Get(HDC dc,int x,int y,int w,int h);				//!< Get data from dc
	void Init(int width, int height, LONG size);
	BOOL SaveToFile(LPTSTR szFilePath);
	BOOL Draw(HDC hDC, int xPos, int yPos);					// draw onto hDC
	BOOL Draw(HWND hWnd, int xPos, int yPos,int frame_left,int frame_top);
	BOOL DrawAB(HWND hWnd, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha=255,int AF=AC_SRC_OVER);	// test
	BOOL DrawAB(HDC hDC, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha=255,int AF=AC_SRC_OVER);	// alpha blend
	BOOL DrawABGo(HDC hDCTo,HDC hDCFrom, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha=255,int AF=AC_SRC_OVER);  // preexisting DC and selected
	BOOL DrawABDib(HDC hDC, CDibImage &pDest,int xPos, int yPos,int iAlpha=255,int AF=AC_SRC_OVER);	// alpha blend this dib into another dib.
	void ResetDIBData ( );
	int	 ScaleDown(HWND hWnd, int scale);
	LPBYTE GetDataPtr() { return m_pData; }
	UINT32 *GetDataPtr32() { return m_iData; }
	bool	CreateData32(int w,int h);
	void	SetData32(int x,int y,UINT32 d);
	void	DeleteData();
	void	FlipData32();
	void	Clear();
	void	Fill32(UINT32 d);
	bool	IsData();
	HDC		MakeDC(HDC dcCompat);
	void	FreeupDC();


	
private:

	int getmax(int a,int b) { if (a >= b) return a; return b; };

	int getmin(int a,int b) { if (a <= b) return a; return b; };

// Accessors

public:

	inline int GetHeight ( ) const  
	{
		return m_Height; 
	}
	inline int GetWidth ( ) const   
	{
		return m_Width;  
	}
	inline LONG GetSize ( ) const  
	{
		return m_lSize;   
	}
	HBITMAP GetBitmapHandle ();
	inline void ClearBitmapHandle ()
	{
		if(m_hBitmap!=0)
		{
			DeleteObject(m_hBitmap);
			m_hBitmap= 0;
		}
	}
	void ShareData(BYTE *pData)
	{
		m_bShared = true;
		m_pData = pData;
		m_lSize = m_Width * m_Height * 4;		// 32 bit assumption!!!
	}
	void GetData8To32(BYTE *pData);

	inline void SetBitmapHandle (HBITMAP hBitmap) { m_hBitmap = hBitmap; }
	bool	IsBitmap()		{ return (m_hBitmap!=0);	}
// Attributes

protected:

	int			m_Width;		// Size of image - x in pixels
	int			m_Height;		// Size of image - y in pixels
	bool		m_bShared;		// do not delete when destroyed.
	LONG		m_lSize;		// Size of data in bytes
	HBITMAP		m_hBitmap;
	HDC			m_hDC;		// DANGER: only for temporary use, must be Deleted after Created!
	HGDIOBJ		m_hTmpObj;		// For use with m_hDC to Delete.
	BYTE		*m_pData;		// Data as type byte
	UINT32		*m_iData;		// Data as type UINT32
};

#endif // _DIB_IMAGE_H
