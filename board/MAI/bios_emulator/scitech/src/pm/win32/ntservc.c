/****************************************************************************
*
*                         SciTech Display Doctor
*
*               Copyright (C) 1991-2001 SciTech Software, Inc.
*                            All rights reserved.
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
* Language:     ANSI C
* Environment:  Windows NT, Windows 2K or Windows XP.
*
* Description:  Main module to do the installation of the SDD and GLDirect
*               device driver components under Windows NT/2K/XP.
*
****************************************************************************/

#include "pmapi.h"
#include "win32/oshdr.h"

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
PARAMETERS:
szDriverName    - Actual name of the driver to install in the system
szServiceName   - Name of the service to create
szLoadGroup     - Load group for the driver (NULL for normal drivers)
dwServiceType   - Service type to create

RETURNS:
True on success, false on failure.

REMARKS:
This function does all the work to install the driver into the system.
The driver is not however activated; for that you must use the Start_SddFilt
function.
****************************************************************************/
ulong PMAPI PM_installService(
    const char *szDriverName,
    const char *szServiceName,
    const char *szLoadGroup,
    ulong dwServiceType)
{
    SC_HANDLE   scmHandle;
    SC_HANDLE   driverHandle;
    char        szDriverPath[MAX_PATH];
    HKEY        key;
    char        keyPath[MAX_PATH];
    ulong       status;

    /* Obtain a handle to the service control manager requesting all access */
    if ((scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	return GetLastError();

    /* Find the path to the driver in system directory */
    GetSystemDirectory(szDriverPath, sizeof(szDriverPath));
    strcat(szDriverPath, "\\drivers\\");
    strcat(szDriverPath, szDriverName);

    /* Create the service with the Service Control Manager. */
    driverHandle = CreateService(scmHandle,
				 szServiceName,
				 szServiceName,
				 SERVICE_ALL_ACCESS,
				 dwServiceType,
				 SERVICE_BOOT_START,
				 SERVICE_ERROR_NORMAL,
				 szDriverPath,
				 szLoadGroup,
				 NULL,
				 NULL,
				 NULL,
				 NULL);

    /* Check to see if the driver could actually be installed. */
    if (!driverHandle) {
	status = GetLastError();
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Get a handle to the key for driver so that it can be altered in the */
    /* next step. */
    strcpy(keyPath, "SYSTEM\\CurrentControlSet\\Services\\");
    strcat(keyPath, szServiceName);
    if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,keyPath,0,KEY_ALL_ACCESS,&key)) != ERROR_SUCCESS) {
	/* A problem has occured. Delete the service so that it is not installed. */
	status = GetLastError();
	DeleteService(driverHandle);
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Delete the ImagePath value in the newly created key so that the */
    /* system looks for the driver in the normal location. */
    if ((status = RegDeleteValue(key, "ImagePath")) != ERROR_SUCCESS) {
	/* A problem has occurred. Delete the service so that it is not */
	/* installed and will not try to start. */
	RegCloseKey(key);
	DeleteService(driverHandle);
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Clean up and exit */
    RegCloseKey(key);
    CloseServiceHandle(driverHandle);
    CloseServiceHandle(scmHandle);
    return ERROR_SUCCESS;
}

/****************************************************************************
PARAMETERS:
szServiceName   - Name of the service to start

RETURNS:
True on success, false on failure.

REMARKS:
This function is used to start the specified service and make it active.
****************************************************************************/
ulong PMAPI PM_startService(
    const char *szServiceName)
{
    SC_HANDLE       scmHandle;
    SC_HANDLE       driverHandle;
    SERVICE_STATUS	serviceStatus;
    ulong           status;

    /* Obtain a handle to the service control manager requesting all access */
    if ((scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	return GetLastError();

    /* Open the service with the Service Control Manager. */
    if ((driverHandle = OpenService(scmHandle,szServiceName,SERVICE_ALL_ACCESS)) == NULL) {
	status = GetLastError();
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Start the service */
    if (!StartService(driverHandle,0,NULL)) {
	status = GetLastError();
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Query the service to make sure it is there */
    if (!QueryServiceStatus(driverHandle,&serviceStatus)) {
	status = GetLastError();
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}
    CloseServiceHandle(driverHandle);
    CloseServiceHandle(scmHandle);
    return ERROR_SUCCESS;
}

/****************************************************************************
PARAMETERS:
szServiceName   - Name of the service to stop

RETURNS:
True on success, false on failure.

REMARKS:
This function is used to stop the specified service and disable it.
****************************************************************************/
ulong PMAPI PM_stopService(
    const char *szServiceName)
{
    SC_HANDLE       scmHandle;
    SC_HANDLE       driverHandle;
    SERVICE_STATUS	serviceStatus;
    ulong           status;

    /* Obtain a handle to the service control manager requesting all access */
    if ((scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	return GetLastError();

    /* Open the service with the Service Control Manager. */
    if ((driverHandle = OpenService(scmHandle,szServiceName,SERVICE_ALL_ACCESS)) == NULL) {
	status = GetLastError();
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Stop the service from running */
    if (!ControlService(driverHandle, SERVICE_CONTROL_STOP, &serviceStatus)) {
	status = GetLastError();
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}
    CloseServiceHandle(driverHandle);
    CloseServiceHandle(scmHandle);
    return ERROR_SUCCESS;
}

/****************************************************************************
PARAMETERS:
szServiceName   - Name of the service to remove

RETURNS:
True on success, false on failure.

REMARKS:
This function is used to remove a service completely from the system.
****************************************************************************/
ulong PMAPI PM_removeService(
    const char *szServiceName)
{
    SC_HANDLE   scmHandle;
    SC_HANDLE   driverHandle;
    ulong       status;

    /* Obtain a handle to the service control manager requesting all access */
    if ((scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	return GetLastError();

    /* Open the service with the Service Control Manager. */
    if ((driverHandle = OpenService(scmHandle,szServiceName,SERVICE_ALL_ACCESS)) == NULL) {
	status = GetLastError();
	CloseServiceHandle(scmHandle);
	return status;
	}

    /* Remove the service */
    if (!DeleteService(driverHandle)) {
	status = GetLastError();
	CloseServiceHandle(driverHandle);
	CloseServiceHandle(scmHandle);
	return status;
	}
    CloseServiceHandle(driverHandle);
    CloseServiceHandle(scmHandle);
    return ERROR_SUCCESS;
}
