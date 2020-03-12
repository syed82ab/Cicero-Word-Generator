// Copyright [2016] Zurich Instruments AG
#include <stdlib.h>
#include <stdio.h>

#include "ziAPI.h"
#define NAG_CALL __stdcall
#define NAG_DLL_EXPIMP __declspec(dllexport)
/*NAG_DLL_EXPIMP void NAG_CALL SetPtPID(ZIDoubleData value) {
	ZIResult_enum retVal;
	ZIConnection conn;
	char* errBuffer;
	const char serverAddress[] = "localhost";
	//ZIDoubleData value=0.00010;
	// Initialize ZIConnection.
	retVal = ziAPIInit(&conn);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't init Connection: %s\n", errBuffer);
		return 1;
	}
	else
	{
		fprintf(stderr, "Connection initialised: %d\n", retVal);
	}

	// Connect to the Data Server: Use port 8005 for the HF2 Data Server, use
	// 8004 for the UHF and MF Data Servers. HF2 only support ZI_API_VERSION_1,
	// see the LabOne Programming Manual for an explanation of API Levels.
	retVal = ziAPIConnect(conn, serverAddress, 8005);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Error, can't connect to the Data Server: `%s`.\n", errBuffer);
	}
	else {
		// Set all demodulator rates of device dev1046 to 150 Hz
		retVal = ziAPISetValueD(conn, "/dev574/pids/0/setpoint", value);
		if (retVal != ZI_INFO_SUCCESS) {
			ziAPIGetError(retVal, &errBuffer, NULL);
			fprintf(stderr, "Can't set parameter: %s\n", errBuffer);
		}
		else
		{
			fprintf(stderr, "supposed do have set pids/0/setpoint to %f\n", value);
		}

		// Disconnect from the Data Server. Since ZIAPIDisconnect always returns
		// ZI_INFO_SUCCESS no error handling is required.
		ziAPIDisconnect(conn);
	}

	// Destroy the ZIConnection. Since ZIAPIDestroy always returns
	// ZI_INFO_SUCCESS, no error handling is required.
	ziAPIDestroy(conn);

	return;
}
*/
ZIResult_enum retVal;
ZIResult_enum retVal2;
ZIConnection conn;
char* errBuffer;
const char serverAddress[] = "localhost";
NAG_DLL_EXPIMP int NAG_CALL ziInit(ZIConnection* conn) {
	retVal = ziAPIInit(conn);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't init Connection: %s\n", errBuffer);
		return 1;
	}
	else
	{
		fprintf(stderr, "Connection initialised: %d\n", retVal);
	}
	return retVal;
}

// Connect to the Data Server: Use port 8005 for the HF2 Data Server, use
// 8004 for the UHF and MF Data Servers. HF2 only support ZI_API_VERSION_1,
// see the LabOne Programming Manual for an explanation of API Levels.
NAG_DLL_EXPIMP int NAG_CALL ziConnect(ZIConnection conn) {

	retVal = ziAPIConnect(conn, serverAddress, 8005);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Error, can't connect to the Data Server: `%s`.\n", errBuffer);
	}
	else {
		fprintf(stderr, "Connected: %d\n", retVal);
	}
	return retVal;
}

NAG_DLL_EXPIMP int NAG_CALL ziSetValueD(ZIConnection conn, ZIDoubleData value) {

	retVal = ziAPISetValueD(conn, "/dev574/pids/0/setpoint", value);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't set parameter: %s\n", errBuffer);
	}
	else
	{
		fprintf(stderr, "supposed do have set /dev574/pids/0/setpoint to %f\n", value);
	}
	return retVal;
}

NAG_DLL_EXPIMP int NAG_CALL ziSyncSetValueD(ZIConnection conn, ZIDoubleData value) {

	retVal = ziAPISyncSetValueD(conn, "/dev574/pids/0/setpoint", &value);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't set parameter: %s\n", errBuffer);
	}
	else
	{
		fprintf(stderr, "supposed do have set /dev574/pids/0/setpoint to %f\n", value);
	}
	return retVal;
}

NAG_DLL_EXPIMP int NAG_CALL ziTogglePID1(ZIConnection conn, ZIIntegerData value) {

	retVal = ziAPISyncSetValueI(conn, "/dev574/pids/0/enable", &value);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't toggle loek: %s\n", errBuffer);
	}
	else
	{
		fprintf(stderr, "supposed do toggle lock\n");
	}
	return retVal;
}
/*
NAG_DLL_EXPIMP int NAG_CALL ziEnablePID1(ZIConnection conn) {

	retVal = ziAPISyncSetValueD(conn, "/dev574/pids/0/enable", 1);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't enable locek: %s\n", errBuffer);
	}
	else
	{
		fprintf(stderr, "supposed do enable lock\n");
	}
	return retVal;
}
*/
NAG_DLL_EXPIMP void NAG_CALL ziDisconnect(ZIConnection conn) {

	// Disconnect from the Data Server. Since ZIAPIDisconnect always returns
	// ZI_INFO_SUCCESS no error handling is required.
	ziAPIDisconnect(conn);
	return;
}
NAG_DLL_EXPIMP void NAG_CALL ziDestroy(ZIConnection conn) {
	// Destroy the ZIConnection. Since ZIAPIDestroy always returns
	// ZI_INFO_SUCCESS, no error handling is required.
	ziAPIDestroy(conn);

	return;
}
NAG_DLL_EXPIMP int NAG_CALL ziGetPID1(ZIConnection conn, ZIDoubleData* value) {

	retVal = ziAPIGetValueD(conn, "/dev574/pids/0/setpoint", value);
	if (retVal != ZI_INFO_SUCCESS) {
		ziAPIGetError(retVal, &errBuffer, NULL);
		fprintf(stderr, "Can't get parameter: %s\n", errBuffer);
	}
	else
	{
		fprintf(stderr, "value of /dev574/pids/0/setpoint is %f\n", *value);
	}
		return retVal;
}