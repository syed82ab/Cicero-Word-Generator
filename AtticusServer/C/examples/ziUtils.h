// Copyright [2016] Zurich Instruments AG
//
// Note: The utility functions included in this header file are a preliminary
// version.  Function calls and parameters may change without notice.

#ifndef __ETC_EXAMPLES_ZIUTILS_H__
#define __ETC_EXAMPLES_ZIUTILS_H__

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
  #include <windows.h>
  #define PRsize_t "I"
  #define PRptrdiff_t "I"
  #define PRIu64      "llu"
  #define PRId64      "lld"
  #ifndef snprintf
    #define snprintf sprintf_s
  #endif
  #define strtok_r strtok_s
#else
  #include <inttypes.h>
  #define PRsize_t "z"
  #define PRptrdiff_t "t"
  #include <string.h>
  #include <unistd.h>
#endif
#include <stdexcept>

void sleep(int ms) {
#ifdef _WIN32
  Sleep(ms);
#else
  usleep(ms * 1000);
#endif
}

static inline bool isError(ZIResult_enum resultCode) {
  if (resultCode != ZI_INFO_SUCCESS) {
    char* message;
    ziAPIGetError(resultCode, &message, NULL);
    fprintf(stderr, "Error: %s\n", message);
    return true;
  }
  return false;
}

static inline void checkError(ZIResult_enum resultCode) {
  if (resultCode != ZI_INFO_SUCCESS) {
    char* message;
    ziAPIGetError(resultCode, &message, NULL);
    throw(std::runtime_error(message));
  }
}

static inline void checkLastError(ZIConnection conn) {
    char message[1000];
    message[0] = 0;
    ziAPIGetLastError(conn, message, 1000);
    if (strlen(message) > 0) {
        throw(std::runtime_error(message));
    } else {
        printf("No error!\n");
    }
}

/// Create a Data Server session for the device and connect it on a physical interface (if not previously connected).
/** This function is a helper function to create an API session for the specified device on an appropriate Data
    Server. It uses Zurich Instruments Device Discovery to find the specified device on the local area network and
    determine which Data Server may be used to connect to it. The API Level used for the connection is the minimum Level
    supported by the device and by the input argument maxSupportedApilevel.

    @param[in]  conn                  The initialised ::ZIConnection which will be associated with the created API
                                      session.
    @param[in]  deviceAddress         The device address for which to create the API session, e.g., dev2006 or
                                      UHF-DEV2006 (as displayed on the back panel of the instrument).
    @param[in]  maxSupportedApilevel  A valid API Level (ZIAPIVersion_enum) that specifies the maximum API Level
                                      supported by the client code that will work with the API session.
    @param[out] deviceId              The device's ID as reported by ::ziAPIDiscoveryFind.
 */
static inline uint8_t ziCreateAPISession(ZIConnection conn, char* deviceAddress, ZIAPIVersion_enum maxSupportedApilevel,
                                         const char** deviceId) {
  if (!isError(ziAPIDiscoveryFind(conn, deviceAddress, deviceId))) {
    char message[1024];
    try {
      const char *serveraddress;
      ZIIntegerData discoverable = 0;
      ZIIntegerData serverport = 0;
      ZIIntegerData apilevel;
      const char* connected;
      // First check that the device is discoverable on the network or another interface.
      checkError(ziAPIDiscoveryGetValueI(conn, *deviceId, "discoverable", &discoverable));
      if (discoverable != 1) {
        snprintf(message, sizeof(message), "`%s` is not discoverable.", *deviceId);
        ziAPIWriteDebugLog(4, message);
        return 1;
      } else {
        snprintf(message, sizeof(message), "Discovered device `%s`.", *deviceId);
        ziAPIWriteDebugLog(0, message);
      }
      // The device is discoverable - get the discovery properties required to
      // create a connnection via a Data Server.
      checkError(ziAPIDiscoveryGetValueS(conn, *deviceId, "serveraddress", &serveraddress));
      checkError(ziAPIDiscoveryGetValueI(conn, *deviceId, "serverport", &serverport));
      checkError(ziAPIDiscoveryGetValueI(conn, *deviceId, "apilevel", &apilevel));
      checkError(ziAPIDiscoveryGetValueS(conn, *deviceId, "connected", &connected));

      ZIIntegerData apilevelConnection = apilevel;
      if (maxSupportedApilevel < apilevel) {
        apilevelConnection = maxSupportedApilevel;
      }
      // Create an API Session to the Data Server reported by discovery.
      snprintf(message, sizeof(message), "Creating an API Session with the Data Server running on `%s` on port `%"
               PRIu64 "` with API Level `%" PRIu64 "`.", serveraddress, serverport, apilevel);
      ziAPIWriteDebugLog(0, message);
      checkError(ziAPIConnectEx(conn, serveraddress, static_cast<uint16_t>(serverport),
                                static_cast<ZIAPIVersion_enum>(apilevelConnection), NULL));

      // Try to connect the device to the Data Server if not already.
      if (strcmp(connected, "\0")) {
        snprintf(message, sizeof(message), "Device is already connected on interface `%s`.", connected);
        ziAPIWriteDebugLog(0, message);
      } else {
        const char* interfaces;
        checkError(ziAPIDiscoveryGetValueS(conn, *deviceId, "interfaces", &interfaces));
        snprintf(message, sizeof(message), "Device is not connected, available interfaces: `%s`.", interfaces);
        ziAPIWriteDebugLog(0, message);
        // Get the first available interface.
        char* saveptr;
        char* interfaceConnection = strtok_r((char*)interfaces, "\n", &saveptr);
        if (strcmp(interfaceConnection, "\0") == 0) {
          // This should not happen if the device is discoverable.
          snprintf(message, sizeof(message),
                   "Error: The device `%s` is not connected but could not read an available interface.\n", *deviceId);
          ziAPIWriteDebugLog(4, message);
          return 1;
        }
        snprintf(message, sizeof(message), "Will try to connect on: `%s`.\n", interfaceConnection);
        ziAPIWriteDebugLog(0, message);
        checkError(ziAPIConnectDevice(conn, *deviceId, interfaceConnection, NULL));
      }
      return 0;
    } catch (std::runtime_error& e) {
      char extErrorMessage[1024] = "";
      ziAPIGetLastError(conn, extErrorMessage, 1024);
      snprintf(message, sizeof(message),
               "Error whilst creating API Session and connecting device: `%s`.\nDetails: `%s`.",
               e.what(), extErrorMessage);
      ziAPIWriteDebugLog(4, message);
      return 1;
    }
  } else {
    return 1;
  }
}

/// Check the versions of the API and Data Server are the same.
/** Issue a warning and return 0 if the release version of the API used in the session (daq) does not have the same
    release version as the Data Server (that the API is connected to). If the versions match return 1.

    @param[in]   conn     The initialised ::ZIConnection representing an API session.
 */
static inline uint8_t ziApiServerVersionCheck(ZIConnection conn) {
  unsigned int apiRevision = 0;
  const char *apiVersion;
  ZIIntegerData serverRevision = 0;
  char serverVersion[1024];

  checkError(ziAPIGetRevision(&apiRevision));
  checkError(ziAPIGetVersion(&apiVersion));
  checkError(ziAPIGetValueI(conn, "/zi/about/revision", &serverRevision));
  const char path[] = "/zi/about/version";
  unsigned int length = 0;
  checkError(ziAPIGetValueString(conn, path, serverVersion, &length, 1024));

  if (strcmp(apiVersion, serverVersion) != 0) {
    printf("*******************************************************************************************************\n");
    printf("Warning: There is a mismatch between the versions of the API and Data Server. The API reports version `%s' (revision: "
           "%d) and Data Server `%s', (revision: %"  PRIu64 "). See the ``Compatibility'' Section in the LabOne Programming "
           "Manual for more information.\n", apiVersion, apiRevision, serverVersion, serverRevision);
    printf("*******************************************************************************************************\n");
    return 0;
  }

  return 1;
}

#endif  // __ETC_EXAMPLES_ZIUTILS_H__
