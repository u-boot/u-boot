/****************************************************************************
*
*                   SciTech Multi-platform Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Win32
*
* Description:  Win32 implementation for the SciTech cross platform
*               event library.
*
****************************************************************************/

#include "event.h"
#include "pmapi.h"
#include "win32/oshdr.h"
#include "nucleus/graphics.h"

/*---------------------------- Global Variables ---------------------------*/

/* Publicly accessible variables */

int                 _PM_deskX,_PM_deskY;/* Desktop dimentions           */
HWND                _PM_hwndConsole;    /* Window handle for console    */
#ifdef  __INTEL__
uint                _PM_cw_default;     /* Default FPU control word     */
#endif

/* Private internal variables */

static HINSTANCE    hInstApp = NULL;/* Application instance handle      */
static HWND         hwndUser = NULL;/* User window handle               */
static HINSTANCE    hInstDD = NULL; /* Handle to DirectDraw DLL         */
static LPDIRECTDRAW lpDD = NULL;    /* DirectDraw object                */
static LONG         oldWndStyle;    /* Info about old user window       */
static LONG         oldExWndStyle;  /* Info about old user window       */
static int          oldWinPosX;     /* Old window position X coordinate */
static int          oldWinPosY;     /* Old window pisition Y coordinate */
static int          oldWinSizeX;    /* Old window size X                */
static int          oldWinSizeY;    /* Old window size Y                */
static WNDPROC      oldWinProc = NULL;
static PM_saveState_cb suspendApp = NULL;
static ibool        waitActive = false;
static ibool        isFullScreen = false;
static ibool        backInGDI = false;

/* Internal strings */

static char *szWinClassName     = "SciTechDirectDrawWindow";
static char *szAutoPlayKey      = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
static char *szAutoPlayValue    = "NoDriveTypeAutoRun";

/* Dynalinks to DirectDraw functions */

static HRESULT (WINAPI *pDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

/*---------------------------- Implementation -----------------------------*/

/****************************************************************************
REMARKS:
Temporarily disables AutoPlay operation while we are running in fullscreen
graphics modes.
****************************************************************************/
static void DisableAutoPlay(void)
{
    DWORD   dwAutoPlay,dwSize = sizeof(dwAutoPlay);
    HKEY    hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,szAutoPlayKey,0,KEY_EXECUTE | KEY_WRITE,&hKey) == ERROR_SUCCESS) {
	RegQueryValueEx(hKey,szAutoPlayValue,NULL,NULL,(void*)&dwAutoPlay,&dwSize);
	dwAutoPlay |= AUTOPLAY_DRIVE_CDROM;
	RegSetValueEx(hKey,szAutoPlayValue,0,REG_DWORD,(void*)&dwAutoPlay,dwSize);
	RegCloseKey(hKey);
	}
}

/****************************************************************************
REMARKS:
Re-enables AutoPlay operation when we return to regular GDI mode.
****************************************************************************/
static void RestoreAutoPlay(void)
{
    DWORD   dwAutoPlay,dwSize = sizeof(dwAutoPlay);
    HKEY    hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,szAutoPlayKey,0,KEY_EXECUTE | KEY_WRITE,&hKey) == ERROR_SUCCESS) {
	RegQueryValueEx(hKey,szAutoPlayValue,NULL,NULL,(void*)&dwAutoPlay,&dwSize);
	dwAutoPlay &= ~AUTOPLAY_DRIVE_CDROM;
	RegSetValueEx(hKey,szAutoPlayValue,0,REG_DWORD,(void*)&dwAutoPlay,dwSize);
	RegCloseKey(hKey);
	}
}

/****************************************************************************
REMARKS:
Suspends the application by switching back to the GDI desktop, allowing
normal application code to be processed, and then waiting for the
application activate command to bring us back to fullscreen mode with our
window minimised.
****************************************************************************/
static void LeaveFullScreen(void)
{
    int retCode = PM_SUSPEND_APP;

    if (backInGDI)
	return;
    if (suspendApp)
	retCode = suspendApp(PM_DEACTIVATE);
    RestoreAutoPlay();
    backInGDI = true;

    /* Now process messages normally until we are re-activated */
    waitActive = true;
    if (retCode != PM_NO_SUSPEND_APP) {
	while (waitActive) {
	    _EVT_pumpMessages();
	    Sleep(200);
	    }
	}
}

/****************************************************************************
REMARKS:
Reactivate all the surfaces for DirectDraw and set the system back up for
fullscreen rendering.
****************************************************************************/
static void RestoreFullScreen(void)
{
    static ibool    firstTime = true;

    if (firstTime) {
	/* Clear the message queue while waiting for the surfaces to be
	 * restored.
	 */
	firstTime = false;
	while (1) {
	    /* Continue looping until out application has been restored
	     * and we have reset the display mode.
	     */
	    _EVT_pumpMessages();
	    if (GetActiveWindow() == _PM_hwndConsole) {
		if (suspendApp)
		    suspendApp(PM_REACTIVATE);
		DisableAutoPlay();
		backInGDI = false;
		waitActive = false;
		firstTime = true;
		return;
		}
	    Sleep(200);
	    }
	}
}

/****************************************************************************
REMARKS:
This function suspends the application by switching back to the GDI desktop,
allowing normal application code to be processed and then waiting for the
application activate command to bring us back to fullscreen mode with our
window minimised.

This version only gets called if we have not captured the screen switch in
our activate message loops and will occur if the DirectDraw drivers lose a
surface for some reason while rendering. This should not normally happen,
but it is included just to be sure (it can happen on WinNT/2000 if the user
hits the Ctrl-Alt-Del key combination). Note that this code will always
spin loop, and we cannot disable the spin looping from this version (ie:
if the user hits Ctrl-Alt-Del under WinNT/2000 the application main loop
will cease to be executed until the user switches back to the application).
****************************************************************************/
void PMAPI PM_doSuspendApp(void)
{
    static  ibool firstTime = true;

    /* Call system DLL version if found */
    if (_PM_imports.PM_doSuspendApp != PM_doSuspendApp) {
	_PM_imports.PM_doSuspendApp();
	return;
	}

    if (firstTime) {
	if (suspendApp)
	    suspendApp(PM_DEACTIVATE);
	RestoreAutoPlay();
	firstTime = false;
	backInGDI = true;
	}
    RestoreFullScreen();
    firstTime = true;
}

/****************************************************************************
REMARKS:
Main Window proc for the full screen DirectDraw Window that we create while
running in full screen mode. Here we capture all mouse and keyboard events
for the window and plug them into our event queue.
****************************************************************************/
static LONG CALLBACK PM_winProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
	case WM_SYSCHAR:
	    /* Stop Alt-Space from pausing our application */
	    return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	    if (HIWORD(lParam) & KF_REPEAT) {
		if (msg == WM_SYSKEYDOWN)
		    return 0;
		break;
		}
	    /* Fall through for keydown events */
	case WM_KEYUP:
	case WM_SYSKEYUP:
	    if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) {
		if ((HIWORD(lParam) & KF_ALTDOWN) && wParam == VK_RETURN)
		    break;
		/* We ignore the remainder of the system keys to stop the
		 * system menu from being activated from the keyboard and pausing
		 * our app while fullscreen (ie: pressing the Alt key).
		 */
		return 0;
		}
	    break;
	case WM_SYSCOMMAND:
	    switch (wParam & ~0x0F) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
		    /* Ignore screensaver requests in fullscreen modes */
		    return 0;
		}
	    break;
	case WM_SIZE:
	    if (waitActive && backInGDI && (wParam != SIZE_MINIMIZED)) {
		/* Start the re-activation process */
		PostMessage(hwnd,WM_DO_SUSPEND_APP,WM_PM_RESTORE_FULLSCREEN,0);
		}
	    else if (!waitActive && isFullScreen && !backInGDI && (wParam == SIZE_MINIMIZED)) {
		/* Start the de-activation process */
		PostMessage(hwnd,WM_DO_SUSPEND_APP,WM_PM_LEAVE_FULLSCREEN,0);
		}
	    break;
	case WM_DO_SUSPEND_APP:
	    switch (wParam) {
				case WM_PM_RESTORE_FULLSCREEN:
					RestoreFullScreen();
					break;
				case WM_PM_LEAVE_FULLSCREEN:
					LeaveFullScreen();
					break;
		}
	    return 0;
	}
    if (oldWinProc)
	return oldWinProc(hwnd,msg,wParam,lParam);
    return DefWindowProc(hwnd,msg,wParam,lParam);
}

/****************************************************************************
PARAMETERS:
hwnd    - User window to convert
width   - Window of the fullscreen window
height  - Height of the fullscreen window

RETURNS:
Handle to converted fullscreen Window.

REMARKS:
This function takes the original user window handle and modifies the size,
position and attributes for the window to convert it into a fullscreen
window that we can use.
****************************************************************************/
static PM_HWND _PM_convertUserWindow(
    HWND hwnd,
    int width,
    int height)
{
    RECT    window;

    GetWindowRect(hwnd,&window);
    oldWinPosX = window.left;
    oldWinPosY = window.top;
    oldWinSizeX = window.right - window.left;
    oldWinSizeY = window.bottom - window.top;
    oldWndStyle = SetWindowLong(hwnd,GWL_STYLE,WS_POPUP | WS_SYSMENU);
    oldExWndStyle = SetWindowLong(hwnd,GWL_EXSTYLE,WS_EX_APPWINDOW);
    ShowWindow(hwnd,SW_SHOW);
    MoveWindow(hwnd,0,0,width,height,TRUE);
    SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
    oldWinProc = (WNDPROC)SetWindowLong(hwnd,GWL_WNDPROC, (LPARAM)PM_winProc);
    return hwnd;
}

/****************************************************************************
PARAMETERS:
hwnd    - User window to restore

REMARKS:
This function restores the original attributes of the user window and put's
it back into it's original state before it was converted to a fullscreen
window.
****************************************************************************/
static void _PM_restoreUserWindow(
    HWND hwnd)
{
    SetWindowLong(hwnd,GWL_WNDPROC, (LPARAM)oldWinProc);
    SetWindowLong(hwnd,GWL_EXSTYLE,oldExWndStyle);
    SetWindowLong(hwnd,GWL_STYLE,oldWndStyle);
    SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(hwnd,SW_SHOW);
    MoveWindow(hwnd,oldWinPosX,oldWinPosY,oldWinSizeX,oldWinSizeY,TRUE);
    oldWinProc = NULL;
}

/****************************************************************************
PARAMETERS:
device  - Index of the device to load DirectDraw for (0 for primary)

REMARKS:
Attempts to dynamically load the DirectDraw DLL's and create the DirectDraw
objects that we need.
****************************************************************************/
void * PMAPI PM_loadDirectDraw(
    int device)
{
    HDC         hdc;
    int         bits;

    /* Call system DLL version if found */
    if (_PM_imports.PM_loadDirectDraw != PM_loadDirectDraw)
	return _PM_imports.PM_loadDirectDraw(device);

    /* TODO: Handle multi-monitor!! */
    if (device != 0)
	return NULL;

    /* Load the DirectDraw DLL if not presently loaded */
    GET_DEFAULT_CW();
    if (!hInstDD) {
	hdc = GetDC(NULL);
	bits = GetDeviceCaps(hdc,BITSPIXEL);
	ReleaseDC(NULL,hdc);
	if (bits < 8)
	    return NULL;
	if ((hInstDD = LoadLibrary("ddraw.dll")) == NULL)
	    return NULL;
	pDirectDrawCreate = (void*)GetProcAddress(hInstDD,"DirectDrawCreate");
	if (!pDirectDrawCreate)
	    return NULL;
	}

    /* Create the DirectDraw object */
    if (!lpDD && pDirectDrawCreate(NULL, &lpDD, NULL) != DD_OK) {
	lpDD = NULL;
	return NULL;
	}
    RESET_DEFAULT_CW();
    return lpDD;
}

/****************************************************************************
PARAMETERS:
device  - Index of the device to unload DirectDraw for (0 for primary)

REMARKS:
Frees any DirectDraw objects for the device. We never actually explicitly
unload the ddraw.dll library, since unloading and reloading it is
unnecessary since we only want to unload it when the application exits and
that happens automatically.
****************************************************************************/
void PMAPI PM_unloadDirectDraw(
    int device)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_unloadDirectDraw != PM_unloadDirectDraw) {
	_PM_imports.PM_unloadDirectDraw(device);
	return;
	}
    if (lpDD) {
	IDirectDraw_Release(lpDD);
	lpDD = NULL;
	}
    (void)device;
}

/****************************************************************************
REMARKS:
Open a console for output to the screen, creating the main event handling
window if necessary.
****************************************************************************/
PM_HWND PMAPI PM_openConsole(
    PM_HWND hWndUser,
    int device,
    int xRes,
    int yRes,
    int bpp,
    ibool fullScreen)
{
    WNDCLASS        cls;
    static ibool    classRegistered = false;

    /* Call system DLL version if found */
    GA_getSystemPMImports();
    if (_PM_imports.PM_openConsole != PM_openConsole) {
	if (fullScreen) {
	    _PM_deskX = xRes;
	    _PM_deskY = yRes;
	    }
	return _PM_imports.PM_openConsole(hWndUser,device,xRes,yRes,bpp,fullScreen);
	}

    /* Create the fullscreen window if necessary */
    hwndUser = hWndUser;
    if (fullScreen) {
	if (!classRegistered) {
	    /* Create a Window class for the fullscreen window in here, since
	     * we need to register one that will do all our event handling for
	     * us.
	     */
	    hInstApp            = GetModuleHandle(NULL);
	    cls.hCursor         = LoadCursor(NULL,IDC_ARROW);
	    cls.hIcon           = LoadIcon(hInstApp,MAKEINTRESOURCE(1));
	    cls.lpszMenuName    = NULL;
	    cls.lpszClassName   = szWinClassName;
	    cls.hbrBackground   = GetStockObject(BLACK_BRUSH);
	    cls.hInstance       = hInstApp;
	    cls.style           = CS_DBLCLKS;
	    cls.lpfnWndProc     = PM_winProc;
	    cls.cbWndExtra      = 0;
	    cls.cbClsExtra      = 0;
	    if (!RegisterClass(&cls))
		return NULL;
	    classRegistered = true;
	    }
	_PM_deskX = xRes;
	_PM_deskY = yRes;
	if (!hwndUser) {
	    char windowTitle[80];
	    if (LoadString(hInstApp,1,windowTitle,sizeof(windowTitle)) == 0)
		strcpy(windowTitle,"MGL Fullscreen Application");
	    _PM_hwndConsole = CreateWindowEx(WS_EX_APPWINDOW,szWinClassName,
		windowTitle,WS_POPUP | WS_SYSMENU,0,0,xRes,yRes,
		NULL,NULL,hInstApp,NULL);
	    }
	else {
	    _PM_hwndConsole = _PM_convertUserWindow(hwndUser,xRes,yRes);
	    }
	ShowCursor(false);
	isFullScreen = true;
	}
    else {
	_PM_hwndConsole = hwndUser;
	isFullScreen = false;
	}
    SetFocus(_PM_hwndConsole);
    SetForegroundWindow(_PM_hwndConsole);
    DisableAutoPlay();
    (void)bpp;
    return _PM_hwndConsole;
}

/****************************************************************************
REMARKS:
Find the size of the console state buffer.
****************************************************************************/
int PMAPI PM_getConsoleStateSize(void)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_getConsoleStateSize != PM_getConsoleStateSize)
	return _PM_imports.PM_getConsoleStateSize();

    /* Not used in Windows */
    return 1;
}

/****************************************************************************
REMARKS:
Save the state of the console.
****************************************************************************/
void PMAPI PM_saveConsoleState(
    void *stateBuf,
    PM_HWND hwndConsole)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_saveConsoleState != PM_saveConsoleState) {
	_PM_imports.PM_saveConsoleState(stateBuf,hwndConsole);
	return;
	}

    /* Not used in Windows */
    (void)stateBuf;
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Set the suspend application callback for the fullscreen console.
****************************************************************************/
void PMAPI PM_setSuspendAppCallback(
    PM_saveState_cb saveState)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_setSuspendAppCallback != PM_setSuspendAppCallback) {
	_PM_imports.PM_setSuspendAppCallback(saveState);
	return;
	}
    suspendApp = saveState;
}

/****************************************************************************
REMARKS:
Restore the console state.
****************************************************************************/
void PMAPI PM_restoreConsoleState(
    const void *stateBuf,
    PM_HWND hwndConsole)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_restoreConsoleState != PM_restoreConsoleState) {
	_PM_imports.PM_restoreConsoleState(stateBuf,hwndConsole);
	return;
	}

    /* Not used in Windows */
    (void)stateBuf;
    (void)hwndConsole;
}

/****************************************************************************
REMARKS:
Close the fullscreen console.
****************************************************************************/
void PMAPI PM_closeConsole(
    PM_HWND hwndConsole)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_closeConsole != PM_closeConsole) {
	_PM_imports.PM_closeConsole(hwndConsole);
	return;
	}
    ShowCursor(true);
    RestoreAutoPlay();
    if (hwndUser)
	_PM_restoreUserWindow(hwndConsole);
    else
	DestroyWindow(hwndConsole);
    hwndUser = NULL;
    _PM_hwndConsole = NULL;
}

/****************************************************************************
REMARKS:
Return the DirectDraw window handle used by the application.
****************************************************************************/
PM_HWND PMAPI PM_getDirectDrawWindow(void)
{
    /* Call system DLL version if found */
    if (_PM_imports.PM_getDirectDrawWindow != PM_getDirectDrawWindow)
	return _PM_imports.PM_getDirectDrawWindow();
    return _PM_hwndConsole;
}
