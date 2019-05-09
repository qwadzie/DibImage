
/*!
 *  \file DibImage.cpp
 *  \brief CDibImage DIB and .bmp image manipulation and file access persistence.
 */


#include "stdafx.h"

#include "DibImage.h"

#include "Process.h"

////////////////////////////////////////////////////////////////////////////////
//
// CDibImage()
//
// Constructor
//
////////////////////////////////////////////////////////////////////////////////
CDibImage::CDibImage() : 
			m_Width(0)
			,m_Height(0)
			,m_hBitmap(0)
			,m_pData(0)
			,m_lSize(0)
			,m_bShared(false)
			,m_hDC(0)
{
}
CDibImage::CDibImage(LPTSTR szFilePath): 
			m_Width(0)
			,m_Height(0)
			,m_hBitmap(0)
			,m_pData(0)
			,m_lSize(0)
			,m_bShared(false)
			,m_hDC(0)
{
	ReadFromFile(szFilePath);
}
CDibImage::CDibImage(HINSTANCE hInstance,DWORD ID): 
			m_Width(0)
			,m_Height(0)
			,m_hBitmap(0)
			,m_pData(0)
			,m_lSize(0)
			,m_bShared(false)
			,m_hDC(0)
{
	LoadFromResource(hInstance,ID);
}

////////////////////////////////////////////////////////////////////////////////
//
// CDibImage(int height, int width, LONG size)
//
// Constructor
//
////////////////////////////////////////////////////////////////////////////////
CDibImage::CDibImage(int height, int width, LONG size): 
			m_hBitmap(0)
			,m_pData(0)
			,m_bShared(false)
			,m_hDC(0)
{
	Init(height,width,size);
}


////////////////////////////////////////////////////////////////////////////////
//
// CDibImage(HBITMAP hBitmap)
//
// Constructor
//
////////////////////////////////////////////////////////////////////////////////
CDibImage::CDibImage(HBITMAP hBitmap): 
			m_Width(0)
			,m_Height(0)
			,m_hBitmap(0)
			,m_pData(0)
			,m_lSize(0)
			,m_bShared(false)
			,m_hDC(0)
{
	LoadAttachBitmap(hBitmap);
}

void CDibImage::Init(int width, int height, LONG size)
{
	DeleteData();
	m_Width = width;
	m_Height = height;
	m_bShared = false;
	if (size>0)
		m_pData = new BYTE[size];
	else
		m_pData = 0;
	m_lSize = size;

}

HBITMAP CDibImage::GetBitmapHandle ()
{
	return m_hBitmap;
}


bool CDibImage::LoadAttachBitmap(HBITMAP hBitmap)
{
	BITMAP bmp;

	// get bitmap information from HBITMAP
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	m_Height = bmp.bmHeight;
	m_Width = bmp.bmWidth;
	m_lSize = bmp.bmHeight * bmp.bmWidthBytes;
	m_hBitmap = hBitmap;

	// allocate memory for image byte array
	m_pData = new BYTE[m_lSize];
	if (!m_pData)
		return false;
	// fill in m_pData
	memcpy(m_pData, bmp.bmBits, m_lSize);
	return true;
}

//
// NOTE to read HINSTANCE ATL::_AtlBaseModule.GetResourceInstance(); or for older system _Module.GetResourceInstance()
//
bool CDibImage::LoadFromResource(HINSTANCE hInstance,DWORD ID)
{
	HBITMAP hBitmap;
	hBitmap = (HBITMAP) LoadImage(hInstance,   // handle to instance
								MAKEINTRESOURCE(ID),			// name or identifier of the image .say"C:\\NewFolder\\1.bmp"
								IMAGE_BITMAP,       // image types
								0,     // desired width
								0,     // desired height
								LR_CREATEDIBSECTION);
	if (hBitmap == 0)
		return false;

	if (!LoadAttachBitmap(hBitmap))
		return false;

	return true;

}
////////////////////////////////////////////////////////////////////////////////
//
// ~CDibImage()
//
// Destructor
//
////////////////////////////////////////////////////////////////////////////////
CDibImage::~CDibImage()
{
	DeleteData();
	if (m_hDC!=0)
		FreeupDC();
}
bool CDibImage::IsData()
{
	if (m_hBitmap)
		return true;
	if (m_pData)
		return true;
	return false;
}

void CDibImage::DeleteData()
{
	// free allocated memory
	if(m_hBitmap != 0)
		DeleteObject(m_hBitmap);

	if (!m_bShared && m_pData)
		delete [] m_pData;

	m_Width		= 0;
	m_Height	= 0;
	m_lSize		= 0;
	m_hBitmap	= 0;
	m_pData		= 0;
	m_bShared	= false;
}

//
// The data that is saved into a bitmap is 32 bit data,
// so create a 32 bit array to hold the new data to be saved into a bitmap.
//
bool CDibImage::CreateData32(int w,int h)
{
	if (m_pData)
	{
		if ( (w == m_Width) && (h == m_Height))
			return true;
	}
	DeleteData();
	m_Width  = w;
	m_Height = h;
	m_lSize = w*h*sizeof(UINT32);
	m_pData = new BYTE[m_lSize];
	m_iData = reinterpret_cast<UINT32 *>(m_pData);
	if (m_pData == 0)
		return false;
	return true;

}
void CDibImage::Clear()
{
	if ( (m_lSize>0) && (m_pData))
		memset(m_pData,0,m_lSize);
}
void CDibImage::Fill32(UINT32 data)
{
	if (!m_pData)
		return;
	if (m_lSize != m_Width*m_Height*sizeof(UINT32))
		return;
	m_iData = reinterpret_cast<UINT32 *>(m_pData);
	for (int x=0; x<m_Width; x++)
		for (int y=0; y<m_Height; y++)
			m_iData[y*m_Width + x] = data;
}
void CDibImage::SetData32(int x,int y,UINT32 d)
{
	m_iData[y*m_Width + x] = d;
}

void CDibImage::FlipData32()
{
	UINT32 temp;
	long	i,j;
	long	posA,posB;	
	for(i=0; i < m_Height/2; i++)  //loop on number of lines
	{
		posA = i*m_Width;
		posB = ((m_Height-1)-i)*m_Width;

		for(j=0; j<m_Width; j++)  // loop on number of columns
		{
			temp = m_iData[posA + j];
			m_iData[posA + j] = m_iData[posB + j];
			m_iData[posB + j] = temp;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// CDibImage ( const CDibImage& copy )
//
// Copy Constructor
//
////////////////////////////////////////////////////////////////////////////////
CDibImage::CDibImage( const CDibImage& copy )
{
	m_Width = copy.m_Width;
	m_Height = copy.m_Height;
	m_lSize = copy.m_lSize;
	m_hBitmap = copy.m_hBitmap;

	m_pData = new BYTE[m_lSize];
}


//////////////////////////////////////////////////////////////////////////////////
//
// SaveToFile(path)
//
// Saves DIB to file (.bmp)
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CDibImage::SaveToFile(LPTSTR szFilePath)
{
	DWORD			dwByteCount;
	HANDLE			hFile;
	BOOL			bSuccess = FALSE;
	BITMAPINFO		bmi;


	hFile = CreateFile(szFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, _T("Error saving image file."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	try
	{
		BITMAPFILEHEADER	bfhBMP;

		memset(&bfhBMP, 0, sizeof(bfhBMP));

		// fill out the BMP file header
		bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth		= m_Width;
		bmi.bmiHeader.biHeight		= m_Height;
		bmi.bmiHeader.biPlanes		= 1;
		bmi.bmiHeader.biBitCount	= 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage	= 0;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;
		bmi.bmiHeader.biClrUsed		  = 0;
		bmi.bmiHeader.biClrImportant  = 0;

		bfhBMP.bfType = 0x4d42;		// "BM" prefix
		bfhBMP.bfSize = m_Width * m_Height * sizeof(DWORD) + sizeof(BITMAPFILEHEADER);
		bfhBMP.bfOffBits = sizeof(BITMAPFILEHEADER) + bmi.bmiHeader.biSize;

		// save header, info and image bits
		if(!WriteFile(hFile, &bfhBMP, sizeof(bfhBMP), &dwByteCount, NULL))
			throw 0;

		if(!WriteFile(hFile, &bmi.bmiHeader, sizeof(bmi.bmiHeader), &dwByteCount, NULL))
			throw 0;

		if(!WriteFile(hFile, m_pData, m_lSize, &dwByteCount, NULL))
			throw 0;

		bSuccess = TRUE;

	}

	catch(...)
	{
		MessageBox(NULL, _T("Error saving image file."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	CloseHandle(hFile);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////
//
// ReadFromFile(path)
//
// Loads bitmap from file
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CDibImage::ReadFromFile(LPTSTR szFilePath)
{
	DIBSECTION ds;

	TCHAR buf1[1024],buf2[1024];
//	_wgetcwd(buf1,1000);
//	wsprintf(buf2,_T("Reading: %s"),szFilePath);

//	::MessageBox(NULL, buf2, buf1, MB_OK);

	m_hBitmap = (HBITMAP) LoadImage(NULL,   // handle to instance
									szFilePath,			// name or identifier of the image .say"C:\\NewFolder\\1.bmp"
									IMAGE_BITMAP,       // image types
									0,     // desired width
									0,     // desired height
									LR_LOADFROMFILE | LR_CREATEDIBSECTION);
//	wsprintf(buf2,_T("hBitmap:%d"),m_hBitmap);
//	::MessageBox(NULL, buf2, buf1, MB_OK);

	if( m_hBitmap == NULL )
	{
		DWORD dwError = GetLastError();
		TCHAR buf1[1024],buf2[1024];
#ifdef UNICODE
		_wgetcwd(buf1,1000);
		wsprintf(buf2,_T("%d %s Load Image Failed"),dwError,szFilePath);
#else
		sprintf(buf1,"dib error");
		sprintf(buf2,"dib error");
#endif
		MessageBox(NULL, buf2, buf1, MB_OK| MB_ICONEXCLAMATION);
	}
	else
	{
		// get width and height

		GetObject(m_hBitmap, sizeof(DIBSECTION), &ds);

		m_Height = ds.dsBm.bmHeight;
		m_Width = ds.dsBm.bmWidth;
		m_lSize = m_Height * ds.dsBm.bmWidthBytes; // includes padding

		if(m_pData)
		{
			// free previously allocated memory
			delete(m_pData);
			m_pData = 0;
		}
		m_pData = new BYTE[m_lSize];

		memcpy(m_pData, ds.dsBm.bmBits, m_lSize);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
//
// GetMatrixForm()
//
// get the Matrix form of a bitmap
//
//	NOTE: Bitmap format has (0,0) at bottom left
//		  and (height-1, width-1) at top right.
//		  Our matrix format has (0,0) at top left and
//		  (height-1, width-1) at bottom right.
//		  So, we neet to flip the image data vertically
//		  to get to one format from the other.
//
//////////////////////////////////////////////////////////////////////////////////
// CMatrix Removed from CDibImage. Should not be part of class but used side-by-side if necessary (is not necessary)
// for ImacorGUI
// ...........Keep for reference only............
//////////////////////////////////////////////////////////////////////////////////

#if 00

BOOL CDibImage::GetMatrixForm (CMatrix *pMatrix) const
{
	BITMAP		bmp;
	WORD		bitsPerPixel;
	BOOL		bSuccess = FALSE;
	BYTE		*pData = NULL;
	CMatrix		temp(m_Height,m_Width);

	GetObject(m_hBitmap, sizeof(BITMAP), &bmp);

	// NOTE: image bits are in m_pData
	// the size of the Bit Block is m_Size bytes
	// for each pixel we use bmBitsPixel bits
	bitsPerPixel = bmp.bmBitsPixel;

	// NOTE: bmp.bmWidthBytes gives padding information
	// if the bmBitsPixel is 24 (3 bytes) and the width of the image is 50,
	// the width in bytes is 50*3 = 150 bytes. However bmWidthBytes is 152
	// in this case! So there are 2 bytes of padding at the end of each scan
	// line. We must filter the padding bytes out when we create the matrix.

	pData = &m_pData[0];

	// copy bits from DIB into Matrix
	for(int i=0; i<m_Height; i++)  //loop on number of lines
	{
		for(int j=0; j<m_Width; j++)  // loop on number of columns
		{
			temp(i,j) = static_cast<double>(*pData); // test with 3 bytes per pixel
			pData += 3;
		}
		// we're at the end of a scan line
		// check for padding
		pData += (bmp.bmWidthBytes - m_Width*bmp.bmBitsPixel/8);
	}

	// perform vertical flip
	for(int r=0; r<m_Height; r++)  //loop on number of lines
	{
		for(int c=0; c<m_Width; c++)  // loop on number of columns
		{
			(*pMatrix)(r,c) = temp(m_Height-1 - r, c);
		}
	}


	return (bSuccess);
}


//////////////////////////////////////////////////////////////////////////////////
//
// SetDIBFromMatrix()
//
// get the DIB form of an Image matrix
//
//	NOTE: Bitmap format has (0,0) at bottom left
//		  and (height-1, width-1) at top right.
//		  Our matrix format has (0,0) at top left and
//		  (height-1, width-1) at bottom right.
//		  So, we neet to flip the image data vertically
//		  to get to one format from the other.
//
//////////////////////////////////////////////////////////////////////////////////
// NOT IN CLASS ANYMORE (kept as reference)
//
BOOL CDibImage::SetDIBFromMatrix ( HWND hWnd, const CMatrix& inMatrix )
{
	BOOL		bSuccess = TRUE;
	BITMAPINFO	bmi;
	HDC			hDC, hDCtemp;
	LPDWORD		pDibData = NULL;
	BYTE		*pData;
	int			mod;
	CMatrix		temp(m_Height, m_Width);  // temporary matrix for vertical flip


	// perform vertical flip
	for(int r=0; r<m_Height; r++)  //loop on number of lines
	{
		for(int c=0; c<m_Width; c++)  // loop on number of columns
		{
			temp(r,c) = inMatrix(m_Height-1 - r, c);
		}
	}


	// NOTE: we need to create a bitmap from its matrix form
	// the result is a HBITMAP that we store in m_hBitmap
	// and also the image bits that we save in m_pData.

	pData = m_pData;
	// copy image data (bits) from matrix
	// do not forget possible padding at the end of each scan line
	// use bmWidthBytes for that.
	for(int i=0; i<m_Height; i++)  //loop on number of lines
	{
		for(int j=0; j<m_Width; j++)  // loop on number of columns
		{
			// we assume 24-bit colors per pixel and grey scale image
			*pData = static_cast<BYTE>(temp(i,j));
			pData++;
			*pData = static_cast<BYTE>(temp(i,j));
			pData++;
			*pData = static_cast<BYTE>(temp(i,j));
			pData++;
		}
		int tmp = m_Width*3 / 4;
		tmp = (tmp+1)*4;
		mod = tmp - m_Width*3;
		mod = mod % 4;

		for(int k=0; k<mod; k++)
		{
			// add padding bytes
			*pData = 0;
			pData++;
		}
	}

	// create DIB with CreateDibSection
	// this will return a HBITMAP
	hDC = GetDC(hWnd);
	hDCtemp = CreateCompatibleDC(hDC);
	ReleaseDC(hWnd, hDC);

	bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth		= m_Width;
	bmi.bmiHeader.biHeight		= m_Height;
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage	= 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed		  = 0;
	bmi.bmiHeader.biClrImportant  = 0;

	ClearBitmapHandle();
	m_hBitmap = CreateDIBSection(hDCtemp, &bmi, DIB_RGB_COLORS, (void **)&pDibData, NULL, 0);

	// copy image bits into dib section
	memcpy(pDibData, m_pData, m_lSize);

	// free temporary device context
	DeleteDC(hDCtemp);


	return (bSuccess);
}


#endif


///////////////////////////////////////////////////////////////////////////////
/*!
 * \brief Simple blit, of the image, no special functions. NOTE: See Draw(HWND hWnd...._) for centering code, etc.
 *        Also see DrawAB(..) for alpha blending draw.
 */
///////////////////////////////////////////////////////////////////////////////
BOOL CDibImage::Draw(HDC hDC, int xPos, int yPos)
{
	BOOL bSuccess = FALSE;

	if(m_hBitmap)
	{
		// draw the image on image device context
		HDC dcBmp = CreateCompatibleDC(hDC);
		HGDIOBJ TmpObj = SelectObject(dcBmp, m_hBitmap);

		// block transfer of image
		bSuccess = BitBlt(hDC, xPos, yPos, m_Width, m_Height,  dcBmp, 0, 0, SRCCOPY);

		// restore previous selected object
		SelectObject(dcBmp,TmpObj);

		// delete image device context
		DeleteDC(dcBmp);

	}
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////////
//
// Draw(hWnd,x,y,imageX,imageY)
//
// Draws bitmap onto screen
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CDibImage::Draw(HWND hWnd, int xPos, int yPos,int frame_left,int frame_top)
{
	BOOL bSuccess = TRUE;

/*
	int xPos, yPos, frame_left, frame_top;

	if(iFrameId == 0)
	{
		frame_left = FRAME_1_LEFT;
		frame_top = FRAME_1_TOP;
	}
	else
	{
		frame_left = FRAME_2_LEFT;
		frame_top = FRAME_2_TOP;
	}
*/

	if(m_hBitmap)
	{
		HDC hDC = ::GetDC(hWnd);

		// draw the image on image device context
		HDC dcBmp = CreateCompatibleDC(hDC);
		HGDIOBJ TmpObj = SelectObject(dcBmp, m_hBitmap);

		// center image if smaller than frame size
		if(m_Width <= IMAGE_MAX_WIDTH)
			xPos = (frame_left + 1) + (IMAGE_MAX_WIDTH-m_Width)/2;
		else
			xPos = frame_left + 1;

		if(m_Height<=IMAGE_MAX_HEIGHT)
			yPos = (frame_top + 1) + (IMAGE_MAX_HEIGHT-m_Height)/2;
		else
			yPos = frame_top + 1;

		// block transfer of image
		BitBlt(hDC, xPos, yPos, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT, dcBmp, 0, 0, SRCCOPY);

		// restore previous selected object
		SelectObject(dcBmp,TmpObj);

		// delete image device context
		::DeleteDC(dcBmp);

		::ReleaseDC(hWnd, hDC);
	}

	return bSuccess;

}

BOOL CDibImage::DrawABDib(HDC hDC, CDibImage &pDest,int xPos, int yPos,int iAlpha,int AF)	// alpha blend this dib into another dib.
{
	// draw the image on image device context
	HDC dcDestBmp = CreateCompatibleDC(hDC);
	if (dcDestBmp == 0)
		return FALSE;

	HGDIOBJ TmpDestObj = SelectObject(dcDestBmp, pDest.GetBitmapHandle());
	BOOL bRes = DrawAB(dcDestBmp,xPos,yPos,0,m_Width,m_Height,iAlpha,AF);

	// restore previous selected object
	SelectObject(dcDestBmp,TmpDestObj);

	// delete destination image device context
	DeleteDC(dcDestBmp);

	return bRes;
}

BOOL CDibImage::DrawABGo(HDC hDCTo,HDC hDCFrom, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha,int AF)
{
	if ((hDCTo==0) || (hDCFrom==0) )
		return FALSE;
	BLENDFUNCTION bf = {AC_SRC_OVER,0,iAlpha,AF};	//AC_SRC_OVER};		//AC_SRC_ALPHA};

	// block transfer of image
	// NOTE: AlphaBlend will stretch if WH of dest and source differ.
	return AlphaBlend(hDCTo,xPos,yPos,iWidth,iHeight,hDCFrom,iOffset,0,iWidth,iHeight,bf);

	//BitBlt(hDC, xPos, yPos, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT, dcBmp, 0, 0, SRCCOPY);
}
BOOL CDibImage::DrawAB(HDC hDC, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha,int AF)
{
	// draw the image on image device context
	HDC dcBmp = CreateCompatibleDC(hDC);
	if (dcBmp == 0)
		return FALSE;

	HGDIOBJ TmpObj = SelectObject(dcBmp, m_hBitmap);

	BOOL bRes = DrawABGo(hDC,dcBmp,xPos,yPos,iOffset,iWidth,iHeight,iAlpha,AF);

	// restore previous selected object
	SelectObject(dcBmp,TmpObj);

	// delete (source) image device context
	DeleteDC(dcBmp);

	return bRes;
}
//////////////////////////////////////////////////////////////////////////////////
//
// DrawAB(hWnd,x,y,imageX,imageY)
//
// Draw with Alpha Blend (4th byte of color) bitmap onto Window
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CDibImage::DrawAB(HWND hWnd, int xPos, int yPos,int iOffset,int iWidth,int iHeight,int iAlpha,int AF)
{
	BOOL bSuccess = TRUE;

	if(m_hBitmap)
	{
		HDC hDC = ::GetDC(hWnd);

		bSuccess = DrawAB(hDC,xPos,yPos,iOffset,iWidth,iHeight,iAlpha,AF);

		::ReleaseDC(hWnd, hDC);
	}
	return bSuccess;
}







//////////////////////////////////////////////////////////////////////////////////
//
// ResetDIBData()
//
// free allocated memory and reset DIB variables
//
//////////////////////////////////////////////////////////////////////////////////

void CDibImage::ResetDIBData()
{
	if(m_pData)
		delete(m_pData);
	m_pData = 0;

	m_Width = 0;
    m_Height = 0;
    m_lSize = 0;
    m_hBitmap = NULL;

}






//////////////////////////////////////////////////////////////////////////////////
//
// ScaleDown(int scale)
//
// scale down the image
//
//////////////////////////////////////////////////////////////////////////////////

int CDibImage::ScaleDown(HWND hWnd, int scale)
{
	BITMAP		bmp;
	BITMAPINFO	bmi;
	HDC			hDC, hDCtemp;
	LPDWORD		pDibData = NULL;
	BYTE		*pData;
	BYTE		*pNewData;
	BYTE		*pNewDataStart;
	int			rc = 0;
	int			new_height, new_width;
	int			value;
	int			sampleCount;

    if (m_hBitmap == NULL) // nothing to do
        return 0;

    if (scale < 1) // no zoomin in this function
        return -1;

	// get original bitmap information
	GetObject(m_hBitmap, sizeof(BITMAP), &bmp);

	// calculate length of a row in original image
	int rowLength = bmp.bmWidthBytes;

	// resize data array
	new_height = (int) (m_Height / scale + 0.5);
	new_width = (int) (m_Width / scale + 0.5);

	// allocate memory for new image data array
	int mod;
	int tmp = new_width*3 / 4;  // 3 bytes per pixel
	tmp = (tmp+1)*4;
	mod = tmp - new_width*3;
	mod = mod % 4;
	LONG new_size = new_height * (new_width*3 + mod); // takes care of padding
	pNewData = new BYTE[new_size];
	pNewDataStart = pNewData; // save start position of new image buffer
	pData = &m_pData[0];


	for(int i=0; i<new_height; i++)  // loop on new number of rows
	{
		for(int j=0; j<new_width; j++)  // loop on new number of columns
		{
			value = 0;
			sampleCount = 0;

			// accumulate rows from original image
			for(int r= getmax(i*scale - scale/2, 0); r<= getmin(i*scale + scale/2, m_Height-1); r++)
			{
				// accumulate columns from original image
				for(int c= getmax(j*scale - scale/2, 0); c<= getmin(j*scale + scale/2, m_Width-1); c++)
				{
					// accumulator
					value += pData[r*(rowLength) + c*bmp.bmBitsPixel/8];  // bmp.bmBitsPixel/8 = 3
					sampleCount++;
				}
			}
			// average value over number of samples used
			value = (value / sampleCount);
			*pNewData = static_cast<BYTE>(value);
			pNewData++;
			*pNewData = static_cast<BYTE>(value);
			pNewData++;
			*pNewData = static_cast<BYTE>(value);
			pNewData++;
		}
		for(int k=0; k<mod; k++)
		{
			// add padding bytes
			*pNewData = 0;
			pNewData++;
		}
	}

	// clear old image data array
	delete [] m_pData;
	m_pData = 0;

	// set new image
	m_Height = new_height;
	m_Width = new_width;
	m_lSize = new_size;
	m_pData = pNewDataStart;


	// create DIB with CreateDibSection
	// this will return a HBITMAP
	hDC = ::GetDC(hWnd);
	hDCtemp = CreateCompatibleDC(hDC);
	::ReleaseDC(hWnd, hDC);

	bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth		= (LONG) (m_Width);
	bmi.bmiHeader.biHeight		= (LONG) (m_Height);
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage	= 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed		  = 0;
	bmi.bmiHeader.biClrImportant  = 0;

	ClearBitmapHandle();
	m_hBitmap = CreateDIBSection(hDCtemp, &bmi, DIB_RGB_COLORS, (void **)&pDibData, NULL, 0);

	// copy image bits into dib section
	memcpy(pDibData, m_pData, m_lSize);

	// delete temp device context
	DeleteDC(hDCtemp);

    return rc;
}
/*
	example from DisplayData.cpp
	char buf[80];
	sprintf(buf,"BModeImage%d.bmp",i);
	POINT dims = GetDisplayDims();
	CDibImage dibImage(dims.x,dims.y,0);
	dibImage.GetData8To32(pBuffer);
	dibImage.SaveToFile(buf);
*/
// Convert 8 bit image to the output pointer *pData passed as parameter (will be converted to UINT32)

void CDibImage::GetData8To32(BYTE *pData)		// MUST BE TRANSFER EDIT. In-Place Conversion is NOT IMPLEMENTED yet.
{
	Init(m_Width,m_Height,m_Width*m_Height*4);		// Create new 32 bit buffer in memory.
	UINT32 *p32 = (UINT32 *)m_pData;
	for(int i=0; i<m_Height; i++)  // loop on new number of rows
	{
		for(int j=0; j<m_Width; j++)  // loop on new number of columns
		{
			p32[(i*m_Width) + j] = pData[(i*m_Width) + j] + ((UINT32(pData[(i*m_Width) + j]))<<8) + ((UINT32(pData[(i*m_Width) + j]))<<16);
		}
	}
}

void CDibImage::CreateBitmap(HDC dc, int iWidth, int iHeight)
{
	BYTE *pDibData = NULL;

	DeleteData();
	// set new image
	m_Height = iHeight;
	m_Width = iWidth;
	m_lSize = 4*iWidth*iHeight;

	// create DIB with CreateDibSection
	// this will return a HBITMAP
	BITMAPINFO	bmi;
	memset(&bmi,0,sizeof(bmi));
	bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth		= (LONG) (m_Width);
	bmi.bmiHeader.biHeight		= (LONG) (m_Height);
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage	= 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed		  = 0;
	bmi.bmiHeader.biClrImportant  = 0;

	ClearBitmapHandle();
	m_hBitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void **)&pDibData, NULL, 0);

	// copy image bits into dib section
	m_bShared = true;		// deleting the bitmap handle will delete the memory
	m_pData = pDibData;
	m_iData = reinterpret_cast<UINT32 *>(m_pData);
}

// Get From dc input at x,y for width,height, into our bitmap
bool CDibImage::Get(HDC dc, int x, int y, int iWidth, int iHeight)
{
		HDC dcBmp = CreateCompatibleDC(dc);
		HGDIOBJ TmpObj = SelectObject(dcBmp, m_hBitmap);

		// block transfer of image
		//bSuccess = BitBlt(hDC, xPos, yPos, m_Width, m_Height,  dcBmp, 0, 0, SRCCOPY);
		bool bSuccess = BitBlt(dcBmp, 0, 0, iWidth, iHeight,  dc, x, y, SRCCOPY);

		// restore previous selected object
		SelectObject(dcBmp,TmpObj);

		// delete image device context
		DeleteDC(dcBmp);
		return bSuccess;
}

HDC	CDibImage::MakeDC(HDC dcCompat)
{
	if (m_hDC!=0)
		FreeupDC();
	m_hDC = CreateCompatibleDC(dcCompat);
	if (m_hDC==0)
		return 0;
	m_hTmpObj = SelectObject(m_hDC,m_hBitmap);
	return m_hDC;
}
void CDibImage::FreeupDC()
{
	SelectObject(m_hDC,m_hTmpObj);
	DeleteDC(m_hDC);
	m_hDC = 0;
}
//
// End of DibImage.cpp
//