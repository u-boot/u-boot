/****************************************************************************
*
*                  Display Doctor Windows Interface Code
*
*  ======================================================================
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  |                                                                    |
*  |This copyrighted computer code is a proprietary trade secret of     |
*  |SciTech Software, Inc., located at 505 Wall Street, Chico, CA 95928 |
*  |USA (www.scitechsoft.com).  ANY UNAUTHORIZED POSSESSION, USE,       |
*  |VIEWING, COPYING, MODIFICATION OR DISSEMINATION OF THIS CODE IS     |
*  |STRICTLY PROHIBITED BY LAW.  Unless you have current, express       |
*  |written authorization from SciTech to possess or use this code, you |
*  |may be subject to civil and/or criminal penalties.                  |
*  |                                                                    |
*  |If you received this code in error or you would like to report      |
*  |improper use, please immediately contact SciTech Software, Inc. at  |
*  |530-894-8400.                                                       |
*  |                                                                    |
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  ======================================================================
*
* Language:     C++ 3.0
* Environment:  Win16
*
* Description:  Dialog driven configuration program for UniVBE and
*               WinDirect Professional products.
*
****************************************************************************/

#include "center.h"

/*------------------------------ Implementation ---------------------------*/

void _EXPORT CenterWindow(HWND hWndCenter, HWND parent, BOOL repaint)
/****************************************************************************
*
* Function:     CenterWindow
* Parameters:   hWndCenter  - Window to center
*               parent      - Handle for parent window
*               repaint     - true if window should be re-painted
*
* Description:  Centers the specified window within the bounds of the
*               specified parent window. If the parent window is NULL, then
*               we center it using the Desktop window.
*
****************************************************************************/
{
    HWND    hWndParent = (parent ? parent : GetDesktopWindow());
    RECT    RectParent;
    RECT    RectCenter;
    int     CenterX,CenterY,Height,Width;

    GetWindowRect(hWndParent, &RectParent);
    GetWindowRect(hWndCenter, &RectCenter);

    Width = (RectCenter.right - RectCenter.left);
    Height = (RectCenter.bottom - RectCenter.top);
    CenterX = ((RectParent.right - RectParent.left) - Width) / 2;
    CenterY = ((RectParent.bottom - RectParent.top) - Height) / 2;

    if ((CenterX < 0) || (CenterY < 0)) {
	/* The Center Window is smaller than the parent window. */
	if (hWndParent != GetDesktopWindow()) {
	    /* If the parent window is not the desktop use the desktop size. */
	    CenterX = (GetSystemMetrics(SM_CXSCREEN) - Width) / 2;
	    CenterY = (GetSystemMetrics(SM_CYSCREEN) - Height) / 2;
	    }
	CenterX = (CenterX < 0) ? 0: CenterX;
	CenterY = (CenterY < 0) ? 0: CenterY;
	}
    else {
	CenterX += RectParent.left;
	CenterY += RectParent.top;
	}

    /* Copy the values into RectCenter */
    RectCenter.left = CenterX;
    RectCenter.right = CenterX + Width;
    RectCenter.top = CenterY;
    RectCenter.bottom = CenterY + Height;

    /* Move the window to the new location */
    MoveWindow(hWndCenter, RectCenter.left, RectCenter.top,
	    (RectCenter.right - RectCenter.left),
	    (RectCenter.bottom - RectCenter.top), repaint);
}

void _EXPORT CenterLogo(HWND hWndLogo, HWND hWndParent, int CenterY)
/****************************************************************************
*
* Function:     CenterLogo
* Parameters:   hWndLogo    - Window to center
*               hWndParent  - Handle for parent window
*               CenterY     - Top coordinate for logo
*
* Description:  Centers the specified window within the bounds of the
*               specified parent window in the horizontal direction only.
*
****************************************************************************/
{
    RECT    RectParent;
    RECT    RectCenter;
    int     CenterX,Height,Width;

    GetWindowRect(hWndParent, &RectParent);
    GetWindowRect(hWndLogo, &RectCenter);
    Width = (RectCenter.right - RectCenter.left);
    Height = (RectCenter.bottom - RectCenter.top);
    CenterX = ((RectParent.right - RectParent.left) - Width) / 2;

    /* Copy the values into RectCenter */
    RectCenter.left = CenterX;
    RectCenter.right = CenterX + Width;
    RectCenter.top = CenterY;
    RectCenter.bottom = CenterY + Height;

    /* Move the window to the new location */
    MoveWindow(hWndLogo, RectCenter.left, RectCenter.top,
	    (RectCenter.right - RectCenter.left),
	    (RectCenter.bottom - RectCenter.top), false);
}
