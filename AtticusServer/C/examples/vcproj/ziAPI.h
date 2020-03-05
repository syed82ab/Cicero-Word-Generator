// Copyright [2016] Zurich Instruments AG

/**
@file  ziAPI.h
@brief Header File for the LabOne C/C++ API

ziAPI provides all functionality to establish a connection with the Data Server and to communicate with it. It has
functions for setting and getting parameters in a single call as well as an event-framework with which the user may
subscribe the parameter tree and receive the events which occur when values change.
- All functions do not check passed pointers if they're NULL pointers. In that case a segmentation fault will occur.
- The ZIConnection is not thread-safe. One connection can only be used in one thread. If you want to use the ziAPI
  in a multi-threaded program you will have to use one ZIConnection for each thread that is communicating or implement
  a mutual exclusion.
- The Data Server is able to handle connections from threads simultaneously. The Data Server takes over
  the synchronization.
*/

#ifndef __ZIAPI_H__
#define __ZIAPI_H__

// Portable fixed-width integer types.
// In case <stdint.h> is not available on your system, define ZI_CUSTOM_STDINT_H and set it to the
// name of the custom (or specific to your system) implementation.
#ifdef ZI_CUSTOM_STDINT_H
#include ZI_CUSTOM_STDINT_H
#elif defined(_MSC_VER) && _MSC_VER < 1600
// <stdint.h> is not available on MSVC++ below 2010 (10.0)
typedef   signed __int8    int8_t;
typedef   signed __int16   int16_t;
typedef   signed __int32   int32_t;
typedef   signed __int64   int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#else
#include <stdint.h>
#endif

#include <wchar.h>

#ifdef _MSC_VER
  #if defined(ZI_API_DLL_EXPORTING)
    #define ZI_EXPORT __declspec(dllexport)
  #elif defined(ZI_API_INTERNAL)
    #define ZI_EXPORT
  #else
    #define ZI_EXPORT __declspec(dllimport)
  #endif
  #ifndef __cplusplus
    #define inline __inline
  #endif
#else
  #define ZI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// The maximum length that has to be used for passing paths to functions (including terminating zero)
#define MAX_PATH_LEN      256
/// The maximum size of an event's data block
#define MAX_EVENT_SIZE    0x400000
/// The maximum length of the node name (in tree change event)
#define MAX_NAME_LEN      32

// deprecated symbols
// ******************
#ifdef __GNUC__
  #define DEPRECATED(decl) decl __attribute__ ((deprecated))
  #define DEPRECATED_ENUM(decl) __attribute__ ((deprecated)) decl
#elif defined(_MSC_VER)
  #define DEPRECATED(decl) __declspec(deprecated) decl
  #define DEPRECATED_ENUM(decl) decl
#else
  #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
  #define DEPRECATED(decl) decl
  #define DEPRECATED_ENUM(decl) decl
#endif

// We don't want to get the deprecated warnings for the ziAPI.h itself
#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 6
// Only supported by GCC >= 4.6
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
// Visual C++ specific
#pragma warning(push)
// 4996 - declared deprecated
#pragma warning(disable:4996)
#endif

/// Defines return value for all ziAPI functions. Divided into 3 regions: info, warning and error.
enum ZIResult_enum {
  // *********** Infos
  ZI_INFO_BASE = 0x0000,
  /// Success (no error)
  ZI_INFO_SUCCESS = 0x0000,

  ZI_INFO_MAX,

  // *********** Warnings
  ZI_WARNING_BASE = 0x4000,
  /// Warning (general);
  ZI_WARNING_GENERAL = 0x4000,

  /// FIFO Underrun
  ZI_WARNING_UNDERRUN = 0x4001,
  /// FIFO Overflow
  ZI_WARNING_OVERFLOW = 0x4002,

  /// Value or Node not found
  ZI_WARNING_NOTFOUND = 0x4003,

  /// Async command executed in sync mode (will be no async reply)
  ZI_WARNING_NO_ASYNC = 0x4004,

  ZI_WARNING_MAX,

  // *********** Errors
  ZI_ERROR_BASE = 0x8000,
  /// Error (general)
  ZI_ERROR_GENERAL = 0x8000,

  /// USB Communication failed
  ZI_ERROR_USB = 0x8001,

  /// Memory allocation failed
  ZI_ERROR_MALLOC = 0x8002,

  /// Unable to initialize mutex
  ZI_ERROR_MUTEX_INIT = 0x8003,
  /// Unable to destroy mutex
  ZI_ERROR_MUTEX_DESTROY = 0x8004,
  /// Unable to lock mutex
  ZI_ERROR_MUTEX_LOCK = 0x8005,
  /// Unable to unlock mutex
  ZI_ERROR_MUTEX_UNLOCK = 0x8006,

  /// Unable to start thread
  ZI_ERROR_THREAD_START = 0x8007,
  /// Unable to join thread
  ZI_ERROR_THREAD_JOIN = 0x8008,

  /// Can't initialize socket
  ZI_ERROR_SOCKET_INIT = 0x8009,
  /// Unable to connect socket
  ZI_ERROR_SOCKET_CONNECT = 0x800a,
  /// Hostname not found
  ZI_ERROR_HOSTNAME = 0x800b,

  /// Connection invalid
  ZI_ERROR_CONNECTION = 0x800c,
  /// Command timed out
  ZI_ERROR_TIMEOUT = 0x800d,
  /// Command internally failed
  ZI_ERROR_COMMAND = 0x800e,
  /// Command failed in server
  ZI_ERROR_SERVER_INTERNAL = 0x800f,

  /// Provided Buffer length is too small
  ZI_ERROR_LENGTH = 0x8010,

  /// Can't open file or read from it
  ZI_ERROR_FILE = 0x8011,

  /// There is already a similar entry
  ZI_ERROR_DUPLICATE = 0x8012,

  /// Attempt to set a read-only node
  ZI_ERROR_READONLY = 0x8013,

  /// Device is not visible to the server
  ZI_ERROR_DEVICE_NOT_VISIBLE = 0x8014,
  /// Device is already connected by a different server
  ZI_ERROR_DEVICE_IN_USE = 0x8015,
  /// Device does currently not support the specified interface
  ZI_ERROR_DEVICE_INTERFACE = 0x8016,
  /// Device connection timeout
  ZI_ERROR_DEVICE_CONNECTION_TIMEOUT = 0x8017,
  /// Device already connected over a different Interface
  ZI_ERROR_DEVICE_DIFFERENT_INTERFACE = 0x8018,
  /// Device needs FW upgrade
  ZI_ERROR_DEVICE_NEEDS_FW_UPGRADE = 0x8019,
  /// Trying to get data from a poll event with wrong target data type.
  ZI_ERROR_ZIEVENT_DATATYPE_MISMATCH = 0x801A,
  /// Device not found
  ZI_ERROR_DEVICE_NOT_FOUND = 0x801B,

  /// Provided arguments are not supported for the command
  ZI_ERROR_NOT_SUPPORTED = 0x801C,

  /// Connection invalid
  ZI_ERROR_TOO_MANY_CONNECTIONS = 0x801D,

  /// Command not supported on HF2
  ZI_ERROR_NOT_ON_HF2 = 0x801E,

  ZI_ERROR_MAX

  /// @cond deprecated
  // **********
#ifdef _MSC_VER
  ,
  // Success (no error)
  ZI_SUCCESS = 0x0000,

  ZI_MAX_INFO,

  // Warning (general)
  ZI_WARNING = 0x4000,

  // FIFO Underrun
  ZI_UNDERRUN,
  // FIFO Overflow
  ZI_OVERFLOW,

  // Value or Node not found
  ZI_NOTFOUND,

  ZI_MAX_WARNING,

  // Error (general)
  ZI_ERROR = 0x8000,

  // USB Communication failed
  ZI_USB,

  // Memory allocation failed
  ZI_MALLOC,

  // Unable to initialize mutex
  ZI_MUTEX_INIT,
  // Unable to destroy mutex
  ZI_MUTEX_DESTROY,
  // Unable to lock mutex
  ZI_MUTEX_LOCK,
  // Unable to unlock mutex
  ZI_MUTEX_UNLOCK,

  // Unable to start thread
  ZI_THREAD_START,
  // Unable to join thread
  ZI_THREAD_JOIN,

  // Can't initialize socket
  ZI_SOCKET_INIT,
  // Unable to connect socket
  ZI_SOCKET_CONNECT,
  // Hostname not found
  ZI_HOSTNAME,

  // Connection invalid
  ZI_CONNECTION,
  // Command timed out
  ZI_TIMEOUT,
  // Command internally failed
  ZI_COMMAND,
  // Command failed in server
  ZI_SERVER_INTERNAL,

  // Provided Buffer length doesn't reach
  ZI_LENGTH,

  // Can't open file or read from it
  ZI_FILE,

  // There is already a similar entry
  ZI_DUPLICATE,

  // Attempt to set a read-only node
  ZI_READONLY,

  ZI_MAX_ERROR,
#endif
  /// @endcond
};

#ifndef __cplusplus
typedef enum ZIResult_enum ZIResult_enum;
#endif

#ifdef _MSC_VER
#pragma deprecated(ZI_SUCCESS)
#pragma deprecated(ZI_MAX_INFO)
#pragma deprecated(ZI_WARNING)
#pragma deprecated(ZI_UNDERRUN)
#pragma deprecated(ZI_OVERFLOW)
#pragma deprecated(ZI_NOTFOUND)
#pragma deprecated(ZI_MAX_WARNING)
#pragma deprecated(ZI_ERROR)
#pragma deprecated(ZI_USB)
#pragma deprecated(ZI_MALLOC)
#pragma deprecated(ZI_MUTEX_INIT)
#pragma deprecated(ZI_MUTEX_DESTROY)
#pragma deprecated(ZI_MUTEX_LOCK)
#pragma deprecated(ZI_MUTEX_UNLOCK)
#pragma deprecated(ZI_THREAD_START)
#pragma deprecated(ZI_THREAD_JOIN)
#pragma deprecated(ZI_SOCKET_INIT)
#pragma deprecated(ZI_SOCKET_CONNECT)
#pragma deprecated(ZI_HOSTNAME)
#pragma deprecated(ZI_CONNECTION)
#pragma deprecated(ZI_TIMEOUT)
#pragma deprecated(ZI_COMMAND)
#pragma deprecated(ZI_SERVER_INTERNAL)
#pragma deprecated(ZI_LENGTH)
#pragma deprecated(ZI_FILE)
#pragma deprecated(ZI_DUPLICATE)
#pragma deprecated(ZI_READONLY)
#pragma deprecated(ZI_MAX_ERROR)
#else

#ifdef __cplusplus
#define ZIResult_enum_cast(val) ZIResult_enum(val)
#else
#define ZIResult_enum_cast(val) val
#endif

DEPRECATED_ENUM(const ZIResult_enum ZI_SUCCESS         = ZIResult_enum_cast(0x0000));
DEPRECATED_ENUM(const ZIResult_enum ZI_MAX_INFO        = ZIResult_enum_cast(0x0001));
DEPRECATED_ENUM(const ZIResult_enum ZI_WARNING         = ZIResult_enum_cast(0x4000));
DEPRECATED_ENUM(const ZIResult_enum ZI_UNDERRUN        = ZIResult_enum_cast(0x4001));
DEPRECATED_ENUM(const ZIResult_enum ZI_OVERFLOW        = ZIResult_enum_cast(0x4002));
DEPRECATED_ENUM(const ZIResult_enum ZI_NOTFOUND        = ZIResult_enum_cast(0x4003));
DEPRECATED_ENUM(const ZIResult_enum ZI_MAX_WARNING     = ZIResult_enum_cast(0x4004));
DEPRECATED_ENUM(const ZIResult_enum ZI_ERROR           = ZIResult_enum_cast(0x8000));
DEPRECATED_ENUM(const ZIResult_enum ZI_USB             = ZIResult_enum_cast(0x8001));
DEPRECATED_ENUM(const ZIResult_enum ZI_MALLOC          = ZIResult_enum_cast(0x8002));
DEPRECATED_ENUM(const ZIResult_enum ZI_MUTEX_INIT      = ZIResult_enum_cast(0x8003));
DEPRECATED_ENUM(const ZIResult_enum ZI_MUTEX_DESTROY   = ZIResult_enum_cast(0x8004));
DEPRECATED_ENUM(const ZIResult_enum ZI_MUTEX_LOCK      = ZIResult_enum_cast(0x8005));
DEPRECATED_ENUM(const ZIResult_enum ZI_MUTEX_UNLOCK    = ZIResult_enum_cast(0x8006));
DEPRECATED_ENUM(const ZIResult_enum ZI_THREAD_START    = ZIResult_enum_cast(0x8007));
DEPRECATED_ENUM(const ZIResult_enum ZI_THREAD_JOIN     = ZIResult_enum_cast(0x8008));
DEPRECATED_ENUM(const ZIResult_enum ZI_SOCKET_INIT     = ZIResult_enum_cast(0x8009));
DEPRECATED_ENUM(const ZIResult_enum ZI_SOCKET_CONNECT  = ZIResult_enum_cast(0x800a));
DEPRECATED_ENUM(const ZIResult_enum ZI_HOSTNAME        = ZIResult_enum_cast(0x800b));
DEPRECATED_ENUM(const ZIResult_enum ZI_CONNECTION      = ZIResult_enum_cast(0x800c));
DEPRECATED_ENUM(const ZIResult_enum ZI_TIMEOUT         = ZIResult_enum_cast(0x800d));
DEPRECATED_ENUM(const ZIResult_enum ZI_COMMAND         = ZIResult_enum_cast(0x800e));
DEPRECATED_ENUM(const ZIResult_enum ZI_SERVER_INTERNAL = ZIResult_enum_cast(0x800f));
DEPRECATED_ENUM(const ZIResult_enum ZI_LENGTH          = ZIResult_enum_cast(0x8010));
DEPRECATED_ENUM(const ZIResult_enum ZI_FILE            = ZIResult_enum_cast(0x8011));
DEPRECATED_ENUM(const ZIResult_enum ZI_DUPLICATE       = ZIResult_enum_cast(0x8012));
DEPRECATED_ENUM(const ZIResult_enum ZI_READONLY        = ZIResult_enum_cast(0x8013));
DEPRECATED_ENUM(const ZIResult_enum ZI_MAX_ERROR       = ZIResult_enum_cast(0x8014));
#endif

/// Enumerates all types that data in a ZIEvent may have
enum ZIValueType_enum {
  /// No data type, event is invalid.
  ZI_VALUE_TYPE_NONE = 0,

  /// ZIDoubleData type. Use the ZIEvent.value.doubleData pointer to read the data of the event.
  ZI_VALUE_TYPE_DOUBLE_DATA = 1,

  /// ZIIntegerData type. Use the ZIEvent.value.integerData pointer to read the data of the event.
  ZI_VALUE_TYPE_INTEGER_DATA = 2,

  /// ZIDemodSample type. Use the ZIEvent.value.demodSample pointer to read the data of the event.
  ZI_VALUE_TYPE_DEMOD_SAMPLE = 3,

  /// ScopeWave type, used in v1 compatibility mode. use the ZIEvent.value.scopeWaveOld pointer
  /// to read the data of the event.
  ZI_VALUE_TYPE_SCOPE_WAVE_OLD = 4,

  /// ZIAuxInSample type. Use the ZIEvent.value.auxInSample pointer to read the data of the event.
  ZI_VALUE_TYPE_AUXIN_SAMPLE = 5,

  /// ZIDIOSample type. Use the ZIEvent.value.dioSample pointer to read the data of the event.
  ZI_VALUE_TYPE_DIO_SAMPLE = 6,

  /// ZIByteArray type. Use the ZIEvent.value.byteArray pointer to read the data of the event.
  ZI_VALUE_TYPE_BYTE_ARRAY = 7,

  /// ZIPWAWave type. Use the ZIEvent.value.pwaWave pointer to read the data of the event.
  ZI_VALUE_TYPE_PWA_WAVE = 8,

  /// TreeChange type - a list of added or removed nodes, used in v1 compatibility mode. Use the
  /// ZIEvent.value.treeChangeDataOld pointer to read the data of the event.
  ZI_VALUE_TYPE_TREE_CHANGE_DATA_OLD = 16,

  /// ZIDoubleDataTS type. Use the ZIEvent.value.doubleDataTS pointer to read the data of the event.
  ZI_VALUE_TYPE_DOUBLE_DATA_TS = 32,

  /// ZIIntegerDataTS type. Use the ZIEvent.value.integerDataTS pointer to read the data of the event.
  ZI_VALUE_TYPE_INTEGER_DATA_TS = 33,

  /// ZIComplexData type. Use the ZIEvent.value.complexData pointer to read the data of the event.
  ZI_VALUE_TYPE_COMPLEX_DATA = 34,

  /// ZIScopeWave type. Use the ZIEvent.value.scopeWave pointer to read the data of the event.
  ZI_VALUE_TYPE_SCOPE_WAVE = 35,

  /// ZIScopeWaveEx type. Use the ZIEvent.value.scopeWaveEx pointer to read the data of the event.
  ZI_VALUE_TYPE_SCOPE_WAVE_EX = 36,

  /// ZIByteArrayTS type. Use the ZIEvent.value.byteArrayTS pointer to read the data of the event.
  ZI_VALUE_TYPE_BYTE_ARRAY_TS = 38,

  /// ZICntSample type. Use the ZIEvent.value.cntSample pointer to read the data of the event.
  ZI_VALUE_TYPE_CNT_SAMPLE = 46,

  /// ZITrigSample type. Use the ZIEvent.value.trigSample pointer to read the data of the event.
  ZI_VALUE_TYPE_TRIG_SAMPLE = 47,

  /// ZITreeChangeData type - a list of added or removed nodes. Use the ZIEvent.value.treeChangeData
  /// pointer to read the data of the event.
  ZI_VALUE_TYPE_TREE_CHANGE_DATA = 48,

  /// ZIAsyncReply type. Use the ZIEvent.value.asyncReply pointer to read the data of the event.
  ZI_VALUE_TYPE_ASYNC_REPLY = 50,

  /// ZISweeperWave type. Use the ZIEvent.value.sweeperWave pointer to read the data of the event.
  ZI_VALUE_TYPE_SWEEPER_WAVE = 64,

  /// ZISpectrumWave type. Use the ZIEvent.value.spectrumWave pointer to read the data of the event.
  ZI_VALUE_TYPE_SPECTRUM_WAVE = 65,

  /// ZIAdvisorWave type. Use the ZIEvent.value.advisorWave pointer to read the data of the event.
  ZI_VALUE_TYPE_ADVISOR_WAVE = 66,

  /// ZIVectorData type. Use the ZIEvent.value.vectorData pointer to access the data of the event.
  ZI_VALUE_TYPE_VECTOR_DATA = 67,

  /// ZIImpedanceSample type. Use the ZIEvent.value.impedanceSample pointer to access the data of the event.
  ZI_VALUE_TYPE_IMPEDANCE_SAMPLE = 68

  /// @cond deprecated
  // **********
#ifdef _MSC_VER
  ,
  // no data type. the ziEvent is invalid.
  ZI_DATA_NONE = 0,

  // double data type. use the ziEvent::Val.Double Pointer to read the data of the event.
  ZI_DATA_DOUBLE = 1,
  // integer data type. use the ziEvent::Val.Integer Pointer to read the data of the event.
  ZI_DATA_INTEGER = 2,
  // DemodSample data type. use the ziEvent::Val.Sample Pointer to read the data of the event.
  ZI_DATA_DEMODSAMPLE = 3,
  // ScopeWave data type. use the ziEvent::Val.Wave Pointer to read the data of the event.
  ZI_DATA_SCOPEWAVE = 4,
  // MiscADValue data type. use the ziEvent::Val.ADValue Pointer to read the data of the event.
  ZI_DATA_AUXINSAMPLE = 5,
  // DIOValue data type. use the ziEvent::Val.DIOValue Pointer to read the data of the event.
  ZI_DATA_DIOSAMPLE = 6,
  // ByteArray data type. use the ziEvent::Val.ByteArray Pointer to read the data of the event.
  ZI_DATA_BYTEARRAY = 7,

  // a list of added or removed trees. use the ziEvent::Val.Tree Pointer to read the data of the event.
  ZI_DATA_TREE_CHANGED = 16
#endif
  /// @endcond
};

#ifndef __cplusplus
typedef enum ZIValueType_enum ZIValueType_enum;
#endif

#ifdef _MSC_VER
#pragma deprecated(ZI_DATA_NONE)
#pragma deprecated(ZI_DATA_DOUBLE)
#pragma deprecated(ZI_DATA_INTEGER)
#pragma deprecated(ZI_DATA_DEMODSAMPLE)
#pragma deprecated(ZI_DATA_SCOPEWAVE)
#pragma deprecated(ZI_DATA_AUXINSAMPLE)
#pragma deprecated(ZI_DATA_DIOSAMPLE)
#pragma deprecated(ZI_DATA_BYTEARRAY)
#pragma deprecated(ZI_DATA_TREE_CHANGED)
#else

#ifdef __cplusplus
#define ZIValueType_enum_cast(val) ZIValueType_enum(val)
#else
#define ZIValueType_enum_cast(val) val
#endif

DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_NONE         = ZIValueType_enum_cast(0));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_DOUBLE       = ZIValueType_enum_cast(1));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_INTEGER      = ZIValueType_enum_cast(2));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_DEMODSAMPLE  = ZIValueType_enum_cast(3));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_SCOPEWAVE    = ZIValueType_enum_cast(4));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_AUXINSAMPLE  = ZIValueType_enum_cast(5));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_DIOSAMPLE    = ZIValueType_enum_cast(6));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_BYTEARRAY    = ZIValueType_enum_cast(7));
DEPRECATED_ENUM(const ZIValueType_enum ZI_DATA_TREE_CHANGED = ZIValueType_enum_cast(16));
#endif

typedef uint64_t ZITimeStamp;
typedef double ZIDoubleData;
typedef int64_t ZIIntegerData;
typedef uint32_t ZIAsyncTag;
/// A handle with which to reference an instance of a ziCore Module created with ::ziAPIModCreate.
typedef uint64_t ZIModuleHandle;

/// The structure used to hold a single IEEE double value. Same as ZIDoubleData, but with timestamp.
struct ZIDoubleDataTS {
  /// Time stamp at which the value has changed
  ZITimeStamp timeStamp;
  ZIDoubleData value;
};

#ifndef __cplusplus
typedef struct ZIDoubleDataTS ZIDoubleDataTS;
#endif

/// The structure used to hold a single 64bit signed integer value. Same as ZIIntegerData, but with timestamp.
struct ZIIntegerDataTS {
  /// Time stamp at which the value has changed
  ZITimeStamp timeStamp;
  ZIIntegerData value;
};

#ifndef __cplusplus
typedef struct ZIIntegerDataTS ZIIntegerDataTS;
#endif

/// The structure used to hold a complex double value.
struct ZIComplexData {
  /// Time stamp at which the value has changed
  ZITimeStamp timeStamp;
  ZIDoubleData real;
  ZIDoubleData imag;
};

#  ifndef __cplusplus
typedef struct ZIComplexData ZIComplexData;
#  endif

/// Defines the actions that are performed on a tree, as returned in
/// the ZITreeChangeData::action or ZITreeChangeDataOld::action.
enum ZITreeAction_enum {
  /// A node has been removed.
  ZI_TREE_ACTION_REMOVE = 0,

  /// A node has been added.
  ZI_TREE_ACTION_ADD = 1,

  /// A node has been changed.
  ZI_TREE_ACTION_CHANGE = 2
};

/// The struct is holding info about added or removed nodes.
struct ZITreeChangeData {
  /// Time stamp at which the data was updated.
  ZITimeStamp timeStamp;
  /// field indicating which action occurred on the tree. A value of the ZITreeAction_enum.
  uint32_t action;
  /// Name of the Path that has been added, removed or changed.
  char name[MAX_NAME_LEN];
};

#ifndef __cplusplus
typedef struct ZITreeChangeData ZITreeChangeData;
#endif

/// The structure used to hold info about added or removed nodes. This is the version without timestamp
/// used in API v1 compatibility mode.
struct TreeChange {
  /// field indicating which action occurred on the tree. A value of the ::ZITreeAction_enum (TREE_ACTION) enum.
  uint32_t Action;
  /// Name of the Path that has been added, removed or changed
  char Name[MAX_NAME_LEN];
};

#ifndef __cplusplus
typedef struct TreeChange TreeChange;
#endif


/// The structure used to hold data for a single demodulator sample
struct ZIDemodSample {
  /// The timestamp at which the sample has been measured.
  ZITimeStamp timeStamp;

  /// X part of the sample.
  double x;
  /// Y part of the sample.
  double y;

  /// oscillator frequency at that sample.
  double frequency;
  /// oscillator phase at that sample.
  double phase;

  /// the current bits of the DIO.
  uint32_t dioBits;

  /// trigger bits
  uint32_t trigger;

  /// value of Aux input 0.
  double auxIn0;
  /// value of Aux input 1.
  double auxIn1;
};

#ifndef __cplusplus
typedef struct ZIDemodSample ZIDemodSample;
#endif

/// The structure used to hold data for a single auxiliary inputs sample.
struct ZIAuxInSample {
  /// The timestamp at which the values have been measured.
  ZITimeStamp timeStamp;

  /// Channel 0 voltage.
  double ch0;
  /// Channel 1 voltage.
  double ch1;
};

#ifndef __cplusplus
typedef struct ZIAuxInSample ZIAuxInSample;
#endif

/// The structure used to hold data for a single digital I/O sample
struct ZIDIOSample {
  /// The timestamp at which the values have been measured.
  ZITimeStamp timeStamp;

  /// The digital I/O values.
  uint32_t bits;

  /// Filler to keep 8 bytes alignment in the array of ZIDIOSample structures.
  uint32_t reserved;
};

#ifndef __cplusplus
typedef struct ZIDIOSample ZIDIOSample;
#endif

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(push)
#pragma warning(disable:4200)
#endif

/// The structure used to hold an arbitrary array of bytes. This is the version without timestamp
/// used in API Level 1 compatibility mode.
struct ZIByteArray {
  /// Length of the data readable from the Bytes field.
  uint32_t length;
  /// The data itself. The array has the size given in length.
  uint8_t bytes[0];
};

#ifndef __cplusplus
typedef struct ZIByteArray ZIByteArray;
#endif

/// The structure used to hold an arbitrary array of bytes. This is the same as ZIByteArray, but with timestamp.
struct ZIByteArrayTS {
  /// Time stamp at which the data was updated
  ZITimeStamp timeStamp;
  /// length of the data readable from the bytes field
  uint32_t length;
  /// the data itself. The array has the size given in length
  uint8_t bytes[0];
};

#ifndef __cplusplus
typedef struct ZIByteArrayTS ZIByteArrayTS;
#endif

/// The structure used to hold data for a single counter sample.
struct ZICntSample {
  /// The timestamp at which the values have been measured.
  ZITimeStamp timeStamp;
  /// Counter value
  int32_t counter;
  /// Trigger bits
  uint32_t trigger;
};

#ifndef __cplusplus
typedef struct ZICntSample ZICntSample;
#endif

/// The structure used to hold data for a single counter sample.
struct ZITrigSample {
  /// The timestamp at which the values have been measured.
  ZITimeStamp timeStamp;
  /// The sample tick at which the values have been measured.
  ZITimeStamp sampleTick;
  /// Trigger bits
  uint32_t trigger;
  /// Missed trigger bits
  uint32_t missedTriggers;
  /// AWG trigger values at the time of trigger
  uint32_t awgTrigger;
  /// Dio values at the time of trigger
  uint32_t dio;
  /// AWG sequencer index at the time of trigger
  uint32_t sequenceIndex;
};

#ifndef __cplusplus
typedef struct ZITrigSample ZITrigSample;
#endif

/// The structure used to hold a single scope shot (API Level 1). If the client is connected to the Data Server using
/// API Level 4 (recommended if supported by your device class) please see ::ZIScopeWave instead (::ZIScopeWaveEx for
/// API Level 5 and above).
struct ScopeWave {
  /// Time difference between samples
  double dt;

  /// Scope channel of the represented data
  uint32_t ScopeChannel;
  /// Trigger channel of the represented data
  uint32_t TriggerChannel;

  /// Bandwidth-limit flag
  uint32_t BWLimit;

  /// Count of samples
  uint32_t Count;
  /// First wave data
  int16_t Data[0];
};

#ifndef __cplusplus
typedef struct ScopeWave ScopeWave;
#endif

/// The structure used to hold scope data (when using API Level 4). Note that ZIScopeWave does not contain the structure
/// member channelOffset, whereas ::ZIScopeWaveEx does. The data may be formatted differently, depending on
/// settings. See the description of the structure members for details.
struct ZIScopeWave {
// --- 8 bytes border
    /// The timestamp of the last sample in this data block
    ZITimeStamp timeStamp;

// --- 8 bytes border
    /// The timestamp of the trigger (may also fall between samples and in another block)
    ZITimeStamp triggerTimeStamp;

// --- 8 bytes border
    /// Time difference between samples in seconds
    double dt;

// --- 8 bytes border
    /// Up to four channels: if channel is enabled, corresponding element is non-zero.
    uint8_t channelEnable[4];

    /// Specifies the input source for each of the scope four channels.
    ///
    /// Value of channelInput and corresponding input source:
    ///  - 0 = Signal Input 1,
    ///  - 1 = Signal Input 2,
    ///  - 2 = Trigger Input 1,
    ///  - 3 = Trigger Input 2,
    ///  - 4 = Aux Output 1,
    ///  - 5 = Aux Output 2,
    ///  - 6 = Aux Output 3,
    ///  - 7 = Aux Output 4,
    ///  - 8 = Aux Input 1,
    ///  - 9 = Aux Input 2.
    uint8_t channelInput[4];

// --- 8 bytes border
    /// Non-zero if trigger is enabled:
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Trigger on rising edge,
    ///   - Bit (1): 1 = Trigger on falling edge.
    uint8_t triggerEnable;

    /// Trigger source (same values as for channel input)
    uint8_t triggerInput;

    uint8_t reserved0[2];

    /// Bandwidth-limit flag, per channel.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Enable bandwidth limiting.
    ///   - Bit (7...1): Reserved
    uint8_t channelBWLimit[4];

// --- 8 bytes border
    /// Enable/disable math operations such as averaging or FFT.
    ///
    /// Bit encoded:
    ///   - Bit(0): 1 = Perform averaging,
    ///   - Bit(1): 1 = Perform FFT,
    ///   - Bit(7...2): Reserved
    uint8_t channelMath[4];

    /// Data scaling factors for up to 4 channels
    float channelScaling[4];

    /// Current scope shot sequence number. Identifies a scope shot.
    uint32_t sequenceNumber;

// --- 8 bytes border
    /// Current segment number.
    uint32_t segmentNumber;

    /// Current block number from the beginning of a scope shot.
    /// Large scope shots are split into blocks, which need to be concatenated to obtain the complete scope shot.
    uint32_t blockNumber;

// --- 8 bytes border
    /// Total number of samples in one channel in the current scope shot, same for all channels
    uint64_t totalSamples;

// --- 8 bytes border
    /// Data transfer mode
    ///
    /// Value and the corresponding data transfer mode:
    ///  - 0 - SingleTransfer,
    ///  - 1 - BlockTransfer,
    ///  - 3 - ContinuousTransfer.
    /// Other values are reserved.
    uint8_t dataTransferMode;

    /// Block marker:
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = End marker for continuous or multi-block transfer,
    ///   - Bit (7..0): Reserved.
    uint8_t blockMarker;


    /// Indicator Flags.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Data loss detected (samples are 0),
    ///   - Bit (1): 1 = Missed trigger,
    ///   - Bit (2): 1 = Transfer failure (corrupted data).
    uint8_t flags;

    /// Data format of samples:
    ///
    /// Value and the corresponding data format used:
    ///   - 0 - Int16,
    ///   - 1 - Int32,
    ///   - 2 - Float,
    ///   - 4 - Int16Interleaved,
    ///   - 5 - Int32Interleaved,
    ///   - 6 - FloatInterleaved.
    uint8_t sampleFormat;

    /// Number of samples in one channel in the current block, same for all channels
    uint32_t sampleCount;

// --- 8 bytes border
    /// Wave data, access via union member dataInt16, dataInt32 or dataFloat depending on sampleFormat.
    /// Indexing scheme also depends on sampleFormat.
    ///
    /// Example for interleaved int16 wave, 4096 samples, 2 channels:
    ///   - data.dataInt16[0]    - sample 0 of channel 0,
    ///   - data.dataInt16[1]    - sample 0 of channel 1,
    ///   - data.dataInt16[2]    - sample 1 of channel 0,
    ///   - data.dataInt16[3]    - sample 1 of channel 1,
    ///   - ...
    ///   - data.dataInt16[8190] - sample 4095 of channel 0,
    ///   - data.dataInt16[8191] - sample 4095 of channel 1.
    ///
    /// Example for non-interleaved int16 wave, 4096 samples, 2 channels:
    ///   - data.dataInt16[0]    - sample 0 of channel 0,
    ///   - data.dataInt16[1]    - sample 1 of channel 0,
    ///   - ..                   - ...
    ///   - data.dataInt16[4095] - sample 4095 of channel 0,
    ///   - data.dataInt16[4096] - sample 0 of channel 1,
    ///   - data.dataInt16[4097] - sample 1 of channel 1,
    ///   - ...
    ///   - data.dataInt16[8191] - sample 4095 of channel 1.
    union {
      /// Wave data when sampleFormat==0 or sampleFormat==4
      int16_t dataInt16[0];
      /// Wave data when sampleFormat==1 or sampleFormat==5
      int32_t dataInt32[0];
      /// Wave data when sampleFormat==2 or sampleFormat==6
      float dataFloat[0];
    } data;

};

#ifndef __cplusplus
typedef struct ZIScopeWave ZIScopeWave;
#endif

/// The structure used to hold scope data (extended, when using API Level 5). Note that ZIScopeWaveEx contains the
/// structure member channelOffset; ::ZIScopeWave does not. The data may be formatted differently, depending on
/// settings.  See the description of the structure members for details.
struct ZIScopeWaveEx {
// --- 8 bytes border
    /// The timestamp of the last sample in this data block.
    ZITimeStamp timeStamp;

// --- 8 bytes border
    /// The Timestamp of the trigger (may also fall between samples and in another block).
    ZITimeStamp triggerTimeStamp;

// --- 8 bytes border
    /// Time difference between samples in seconds.
    double dt;

// --- 8 bytes border
    /// Up to four channels: If channel is enabled, the corresponding element is non-zero.
    uint8_t channelEnable[4];

    /// Specifies the input source for each of the scope four channels.
    ///
    /// Value of channelInput and corresponding input source:
    ///   - 0 = Signal Input 1,
    ///   - 1 = Signal Input 2,
    ///   - 2 = Trigger Input 1,
    ///   - 3 = Trigger Input 2,
    ///   - 4 = Aux Output 1,
    ///   - 5 = Aux Output 2,
    ///   - 6 = Aux Output 3,
    ///   - 7 = Aux Output 4,
    ///   - 8 = Aux Input 1,
    ///   - 9 = Aux Input 2.
    uint8_t channelInput[4];

// --- 8 bytes border
    /// Non-zero if trigger is enabled.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Trigger on rising edge,
    ///   - Bit (1): 1 = Trigger on falling edge.
    uint8_t triggerEnable;

    /// Trigger source (same values as for channel input)
    uint8_t triggerInput;

    uint8_t reserved0[2];

    /// Bandwidth-limit flag, per channel.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Enable bandwidth limiting.
    ///   - Bit (7...1): Reserved
    uint8_t channelBWLimit[4];

// --- 8 bytes border
    /// Enable/disable math operations such as averaging or FFT.
    ///
    /// Bit encoded:
    ///   - Bit(0): 1 = Perform averaging,
    ///   - Bit(1): 1 = Perform FFT,
    ///   - Bit(7...2): Reserved
    uint8_t channelMath[4];

    /// Data scaling factors for up to 4 channels
    float channelScaling[4];

    /// Current scope shot sequence number. Identifies a scope shot.
    uint32_t sequenceNumber;

// --- 8 bytes border
    /// Current segment number.
    uint32_t segmentNumber;

    /// Current block number from the beginning of a scope shot.
    /// Large scope shots are split into blocks, which need to be concatenated to obtain the complete scope shot.
    uint32_t blockNumber;

// --- 8 bytes border
    /// Total number of samples in one channel in the current scope shot, same for all channels
    uint64_t totalSamples;

// --- 8 bytes border
    /// Data transfer mode.
    ///
    /// Value and the corresponding data transfer mode:
    ///  - 0 - SingleTransfer,
    ///  - 1 - BlockTransfer,
    ///  - 3 - ContinuousTransfer.
    /// Other values are reserved.
    uint8_t dataTransferMode;

    /// Block marker providing additional information about the current block.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = End marker for continuous or multi-block transfer,
    ///   - Bit (7..0): Reserved.
    uint8_t blockMarker;


    /// Indicator Flags.
    ///
    /// Bit encoded:
    ///   - Bit (0): 1 = Data loss detected (samples are 0),
    ///   - Bit (1): 1 = Missed trigger,
    ///   - Bit (2): 1 = Transfer failure (corrupted data).
    ///   - Bit (3): 1 = Assembled scope recording. 'sampleCount' will be set to 0, use 'totalSamples' instead.
    ///   - Bit (7...4): Reserved.
    uint8_t flags;

    /// Data format of samples.
    ///
    /// Value and the corresponding data format used:
    ///   - 0 - Int16,
    ///   - 1 - Int32,
    ///   - 2 - Float,
    ///   - 4 - Int16Interleaved,
    ///   - 5 - Int32Interleaved,
    ///   - 6 - FloatInterleaved.
    uint8_t sampleFormat;

    /// Number of samples in one channel in the current block, same for all channels.
    uint32_t sampleCount;

// --- 8 bytes border
    /// Data offset (scaled) for up to 4 channels.
    double channelOffset[4];

// --- 8 bytes border
    /// Number of segments in the recording. Only valid if 'flags' bit (3) is set.
    uint32_t totalSegments;

    uint32_t reserved1;

// --- 8 bytes border
    uint64_t reserved2[31];

// --- 8 bytes border
    /// Wave data, access via union member dataInt16, dataInt32 or dataFloat depending on sampleFormat.
    /// Indexing scheme also depends on sampleFormat.
    ///
    /// Example for interleaved int16 wave, 4096 samples, 2 channels:
    ///   - data.dataInt16[0]    - sample 0 of channel 0,
    ///   - data.dataInt16[1]    - sample 0 of channel 1,
    ///   - data.dataInt16[2]    - sample 1 of channel 0,
    ///   - data.dataInt16[3]    - sample 1 of channel 1,
    ///   - ...
    ///   - data.dataInt16[8190] - sample 4095 of channel 0,
    ///   - data.dataInt16[8191] - sample 4095 of channel 1.
    ///
    /// Example for non-interleaved int16 wave, 4096 samples, 2 channels:
    ///   - data.dataInt16[0]    - sample 0 of channel 0,
    ///   - data.dataInt16[1]    - sample 1 of channel 0,
    ///   - ..                   - ...
    ///   - data.dataInt16[4095] - sample 4095 of channel 0,
    ///   - data.dataInt16[4096] - sample 0 of channel 1,
    ///   - data.dataInt16[4097] - sample 1 of channel 1,
    ///   - ...
    ///   - data.dataInt16[8191] - sample 4095 of channel 1.
    union {
      /// Wave data when sampleFormat==0 or sampleFormat==4
      int16_t dataInt16[0];
      /// Wave data when sampleFormat==1 or sampleFormat==5
      int32_t dataInt32[0];
      /// Wave data when sampleFormat==2 or sampleFormat==6
      float dataFloat[0];
    } data;

};

#ifndef __cplusplus
typedef struct ZIScopeWaveEx ZIScopeWaveEx;
#endif

/// Single PWA sample value
struct ZIPWASample {
  /// Phase position of each bin
  double binPhase;
  /// Real PWA result or X component of a demod PWA
  double x;
  /// Y component of the demod PWA
  double y;
  /// Number of events per bin
  uint32_t countBin;
  /// Reserved
  uint32_t reserved;
};

#ifndef __cplusplus
typedef struct ZIPWASample ZIPWASample;
#endif

/// PWA Wave
struct ZIPWAWave {
  /// Time stamp at which the data was updated
  ZITimeStamp timeStamp;

  /// Total sample count considered for PWA
  uint64_t sampleCount;

  /// Input selection used for the PWA
  uint32_t inputSelect;
  /// Oscillator used for the PWA
  uint32_t oscSelect;
  /// Harmonic setting
  uint32_t harmonic;
  /// Bin count of the PWA
  uint32_t binCount;

  /// Frequency during PWA accumulation
  double frequency;

  /// Type of the PWA
  uint8_t pwaType;
  /// PWA Mode [0: zoom PWA, 1: harmonic PWA]
  uint8_t mode;
  /// Overflow indicators.
  /// overflow[0]: Data accumulator overflow,
  /// overflow[1]: Counter at limit,
  /// overflow[6..2]: Reserved,
  /// overflow[7]: Invalid (missing frames).
  uint8_t overflow;
  /// Commensurability of the data
  uint8_t commensurable;
  /// Reserved 32bit
  uint32_t reservedUInt;

  /// PWA data vector
  ZIPWASample data[0];
};

#ifndef __cplusplus
typedef struct ZIPWAWave ZIPWAWave;
#endif

/// Enumerates the bits set in an ::ZIImpedanceSample's flags.
enum ZIImpFlags_enum {
  ZI_IMP_FLAGS_NONE               = 0x00000000,
  /// Internal calibration is applied
  ZI_IMP_FLAGS_VALID_INTERNAL     = 0x00000001,
  /// User compensation is applied
  ZI_IMP_FLAGS_VALID_USER         = 0x00000002,
  /// Reserved for future use.
  ZI_IMP_FLAGS_AUTORANGE_GATING   = 0x00000004,
  /// Overflow on voltage input
  ZI_IMP_FLAGS_OVERFLOW_VOLTAGE   = 0x00000010,
  /// Overflow on current input
  ZI_IMP_FLAGS_OVERFLOW_CURRENT   = 0x00000020,
  /// Underflow on voltage input
  ZI_IMP_FLAGS_UNDERFLOW_VOLTAGE  = 0x00000040,
  /// Underflow on current input
  ZI_IMP_FLAGS_UNDERFLOW_CURRENT  = 0x00000080,
  /// Reserved for future use.
  ZI_IMP_FLAGS_FREQ_EXACT         = 0x00000100,
  /// Reserved for future use.
  ZI_IMP_FLAGS_FREQ_INTERPOLATION = 0x00000200,
  /// Reserved for future use.
  ZI_IMP_FLAGS_FREQ_EXTRAPOLATION = 0x00000400,
  /// LowDUT impedance detected
  ZI_IMP_FLAGS_LOWDUT2T           = 0x00000800,
  /// Suppression of first parameter PARAM0
  ZI_IMP_FLAGS_SUPPRESSION_PARAM0   = 0x00001000,
  /// Suppression of second parameter PARAM1
  ZI_IMP_FLAGS_SUPPRESSION_PARAM1   = 0x00002000,
  /// Reserved for future use
  ZI_IMP_FLAGS_FREQLIMIT_RANGE_VOLTAGE  = 0x00004000,
  /// Frequency bigger than the frequency limit of active current input range
  ZI_IMP_FLAGS_FREQLIMIT_RANGE_CURRENT  = 0x00008000,
  /// Strong compensation detected on PARAM0
  ZI_IMP_FLAGS_STRONGCOMPENSATION_PARAM0 = 0x00010000,
  /// Strong compensation detected on PARAM1
  ZI_IMP_FLAGS_STRONGCOMPENSATION_PARAM1 = 0x00020000,
  /// Non-reasonable values for Q/D measurement
  ZI_IMP_FLAGS_NEGATIVE_QFACTOR = 0x00040000,
  /// Reserved for future use.
  ZI_IMP_FLAGS_BWC_BIT0           = 0x00100000,
  /// Reserved for future use.
  ZI_IMP_FLAGS_BWC_BIT1           = 0x00200000,
  /// Reserved for future use.
  ZI_IMP_FLAGS_BWC_BIT2           = 0x00400000,
  /// Reserved for future use.
  ZI_IMP_FLAGS_BWC_BIT3           = 0x00800000,
  /// Reserved for future use.
  ZI_IMP_FLAGS_BWC_MASK           = 0x00f00000,
  /// Open detected on 4T measurement
  ZI_IMP_FLAGS_OPEN_DETECTION     = 0x01000000,
  /// Overflow on sigin0
  ZI_IMP_FLAGS_OVERFLOW_SIGIN0    = 0x04000000,
  /// Overflow on sigin1
  ZI_IMP_FLAGS_OVERFLOW_SIGIN1    = 0x08000000,
  /// Model selected for the measurement
  ZI_IMP_FLAGS_MODEL_MASK         = 0xf0000000
};

#ifndef __cplusplus
typedef enum ZIImpFlags_enum ZIImpFlags_enum;
#endif

/// The structure used to hold data for a single impedance sample.
struct ZIImpedanceSample {
  /// Timestamp at which the sample has been measured
  ZITimeStamp timeStamp;

  /// Real part of the impedance sample
  double realz;
  /// Imaginary part of the impedance sample
  double imagz;

  /// Frequency at that sample
  double frequency;
  /// Phase at that sample
  double phase;

  /// Flags (see ::ZIImpFlags_enum)
  uint32_t flags;

  /// Trigger bits
  uint32_t trigger;

  /// Value of model parameter 0
  double param0;
  /// Value of model parameter 1
  double param1;

  /// Drive amplitude
  double drive;

  /// Bias voltage
  double bias;
};

#ifndef __cplusplus
typedef struct ZIImpedanceSample ZIImpedanceSample;
#endif

struct ZIStatisticSample {
  /// Average value or single value
  double avg;

  /// Standard deviation
  double stddev;

  /// Power value
  double pwr;
};

#ifndef __cplusplus
typedef struct ZIStatisticSample ZIStatisticSample;
#endif

struct ZISweeperDoubleSample {
  /// Grid value (x-axis)
  double grid;

  /// Bandwidth
  double bandwidth;

  /// Sample count used for statistic calculation
  uint64_t count;

  /// Result value (y-axis)
  ZIStatisticSample value;
};

#ifndef __cplusplus
typedef struct ZISweeperDoubleSample ZISweeperDoubleSample;
#endif

struct ZISweeperDemodSample {
  /// Grid value (x-axis)
  double grid;

  /// Demodulator bandwidth used for the specific sweep point
  double bandwidth;

  /// Sample count used for statistic calculation
  uint64_t count;

  /// Time constant calculated for the specific sweep point
  double tc;

  /// Time constant used by the device
  double tcMeas;

  /// Settling time (s) used to wait until averaging operation is started
  double settling;

  /// Time stamp when the grid value was set on the device
  ZITimeStamp setTimeStamp;

  /// Time stamp when the first statistic value was recorded
  ZITimeStamp nextTimeStamp;

  /// Sweep point statistic result of X
  ZIStatisticSample x;

  /// Sweep point statistic result of Y
  ZIStatisticSample y;

  /// Sweep point statistic result of R
  ZIStatisticSample r;

  /// Sweep point statistic result of phase
  ZIStatisticSample phase;

  /// Sweep point statistic result of frequency
  ZIStatisticSample frequency;

  /// Sweep point statistic result of auxin0
  ZIStatisticSample auxin0;

  /// Sweep point statistic result of auxin1
  ZIStatisticSample auxin1;
};

#ifndef __cplusplus
typedef struct ZISweeperDemodSample ZISweeperDemodSample;
#endif

struct ZISweeperImpedanceSample {
  /// Grid value (x-axis)
  double grid;

  /// Demodulator bandwidth used for the specific sweep point
  double bandwidth;

  /// Sample count used for statistic calculation
  uint64_t count;

  /// Time constant calculated for the specific sweep point
  double tc;

  /// Time constant used by the device
  double tcMeas;

  /// Settling time (s) used to wait until averaging operation is started
  double settling;

  /// Time stamp when the grid value was set on the device
  ZITimeStamp setTimeStamp;

  /// Time stamp when the first statistic value was recorded
  ZITimeStamp nextTimeStamp;

  /// Sweep point statistic result of X
  ZIStatisticSample realz;

  /// Sweep point statistic result of Y
  ZIStatisticSample imagz;

  /// Sweep point statistic result of R
  ZIStatisticSample absz;

  /// Sweep point statistic result of phase
  ZIStatisticSample phasez;

  /// Sweep point statistic result of frequency
  ZIStatisticSample frequency;

  /// Sweep point statistic result of param0
  ZIStatisticSample param0;

  /// Sweep point statistic result of param1
  ZIStatisticSample param1;

  /// Sweep point statistic result of drive amplitude
  ZIStatisticSample drive;

  /// Sweep point statistic result of bias
  ZIStatisticSample bias;
};

#ifndef __cplusplus
typedef struct ZISweeperImpedanceSample ZISweeperImpedanceSample;
#endif

struct ZISweeperHeader {
  /// Total sample count considered for sweeper
  uint64_t sampleCount;

// --- 8 bytes border
  /// Flags
  ///   Bit 0: Phase unwrap
  ///   Bit 1: Sinc filter
  uint8_t flags;

  /// Sample format
  /// Double = 0, Demodulator = 1, Impedance = 2
  uint8_t sampleFormat;

  /// Sweep mode
  /// Sequential = 0, Binary = 1, Bidirectional = 2, Reverse = 3
  uint8_t sweepMode;

  /// Bandwidth mode
  /// Manual = 0, Fixed = 1, Auto = 2
  uint8_t bandwidthMode;

  /// Reserved space for future use
  uint8_t reserved0[4];

// --- 8 bytes border
  /// Reserved space for future use
  uint8_t reserved1[8];
};

#ifndef __cplusplus
typedef struct ZISweeperHeader ZISweeperHeader;
#endif

struct ZISweeperWave {
  /// Time stamp at which the data was updated
  ZITimeStamp timeStamp;

// --- 8 bytes border
  ZISweeperHeader header;

// --- 8 bytes border
  /// Sweeper data vector
  union {
    ZISweeperDoubleSample dataDouble[0];
    ZISweeperDemodSample dataDemod[0];
    ZISweeperImpedanceSample dataImpedance[0];
  } data;
};

#ifndef __cplusplus
typedef struct ZISweeperWave ZISweeperWave;
#endif

struct ZISpectrumDemodSample {
  /// Grid
  double grid;

  /// Filter strength at the specific grid point
  double filter;

  /// X
  double x;

  /// Y
  double y;

  /// R
  double r;
};

#ifndef __cplusplus
typedef struct ZISpectrumDemodSample ZISpectrumDemodSample;
#endif

struct ZISpectrumHeader {
  /// Total sample count considered for spectrum
  uint64_t sampleCount;

// --- 8 bytes border
  /// Flags
  ///   Bit 0: Power
  ///   Bit 1: Spectral density
  ///   Bit 2: Absolute frequency
  ///   Bit 3: Full span
  uint8_t flags;

  /// Sample format
  /// Demodulator = 0.
  uint8_t sampleFormat;

  /// Spectrum mode
  /// FFT(x+iy) = 0, FFT(r) = 1, FFT(theta) = 2, FFT(freq) = 3, FFT(dtheta/dt)/2pi = 4
  uint8_t spectrumMode;

  /// Window
  /// Rectangular = 0, Hann = 1, Hamming = 2, Blackman Harris = 3,
  /// Exponential = 16 (ring-down), Cosine = 17 (ring-down), Cosine squared = 18 (ring-down)
  uint8_t window;

  /// Reserved space for future use
  uint8_t reserved0[4];

// --- 8 bytes border
  /// Reserved space for future use
  uint8_t reserved1[8];

// --- 8 bytes border
  /// Filter bandwidth
  double bandwidth;

// --- 8 bytes border
  /// Rate of the sampled data
  double rate;

// --- 8 bytes border
  /// FFT center value
  double center;

// --- 8 bytes border
  /// FFT bin resolution
  double resolution;

// --- 8 bytes border
  /// Aliasing reject (dB)
  double aliasingReject;

// --- 8 bytes border
  /// Correction factor for the used window when calculating spectral density
  double nenbw;

// --- 8 bytes border
  /// FFT overlap [0 .. 1[
  double overlap;
};

#ifndef __cplusplus
typedef struct ZISpectrumHeader ZISpectrumHeader;
#endif

struct ZISpectrumWave {
  /// Time stamp at which the data was updated
  ZITimeStamp timeStamp;

// --- 8 bytes border
  ZISpectrumHeader header;

// --- 8 bytes border
  /// Spectrum data vector
  union {
    ZISpectrumDemodSample dataDemod[0];
  } data;
};

#ifndef __cplusplus
typedef struct ZISpectrumWave ZISpectrumWave;
#endif

struct ZIAdvisorSample {
  /// Grid
  double grid;

  /// X
  double x;

  /// Y
  double y;
};

#ifndef __cplusplus
typedef struct ZIAdvisorSample ZIAdvisorSample;
#endif

struct ZIAdvisorHeader {
  /// Total sample count considered for advisor
  uint64_t sampleCount;

// --- 8 bytes border
  /// Flags
  uint8_t flags;

  /// Sample format
  /// Bode = 0, Step = 1, Impulse = 2.
  uint8_t sampleFormat;

  /// Reserved space for future use
  uint8_t reserved0[6];

// --- 8 bytes border
  /// Reserved space for future use
  uint8_t reserved1[8];

};

#ifndef __cplusplus
typedef struct ZIAdvisorHeader ZIAdvisorHeader;
#endif

struct ZIAdvisorWave {
  /// Time stamp at which the data was updated
  ZITimeStamp timeStamp;

// --- 8 bytes border
  ZIAdvisorHeader header;

// --- 8 bytes border
  /// Advisor data vector
  union {
    ZIAdvisorSample data[0];
  } data;
};

#ifndef __cplusplus
typedef struct ZIAdvisorWave ZIAdvisorWave;
#endif

/// Enumerates all the types that a ZIVectorData::elementType may have.
enum ZIVectorElementType_enum {
  ZI_VECTOR_ELEMENT_TYPE_UINT8  = 0,
  ZI_VECTOR_ELEMENT_TYPE_UINT16 = 1,
  ZI_VECTOR_ELEMENT_TYPE_UINT32 = 2,
  ZI_VECTOR_ELEMENT_TYPE_UINT64 = 3,
  ZI_VECTOR_ELEMENT_TYPE_FLOAT  = 4,
  ZI_VECTOR_ELEMENT_TYPE_DOUBLE = 5,

  /// NULL-terminated string
  ZI_VECTOR_ELEMENT_TYPE_ASCIIZ = 6
};

/// The structure used to hold vector data block. See the description of the structure members for details.
struct ZIVectorData {
// --- 8 bytes border
    /// Time stamp of this array data block
    ZITimeStamp timeStamp;

// --- 8 bytes border
    /// Current array transfer sequence number. Incremented for each new transfer. Stays same for all blocks of
    /// a single array transfer.
    uint32_t sequenceNumber;

    /// Current block number from the beginning of an array transfer.
    /// Large array transfers are split into blocks, which need to be concatenated to obtain the complete array.
    uint32_t blockNumber;

// --- 8 bytes border
    /// Total number of elements in the array
    uint64_t totalElements;

// --- 8 bytes border
    /// Offset of the current block first element from the beginning of the array
    uint64_t blockOffset;

// --- 8 bytes border
    /// Number of elements in the current block
    uint32_t blockElements;

    /// Block marker:
    ///   Bit (0): 1 = End marker for multi-block transfer
    ///   Bit (1): 1 = Transfer failure
    ///   Bit (7..2): Reserved
    uint8_t flags;

    /// Vector element type, see ::ZIVectorElementType_enum
    uint8_t elementType;

    uint8_t reserved0[2];

// --- 8 bytes border
    uint64_t reserved1[32];

// --- 8 bytes border
    /// First data element of the current block
    union {
      uint8_t dataUInt8[0];
      uint16_t dataUInt16[0];
      uint32_t dataUInt32[0];
      uint64_t dataUInt64[0];
      int8_t dataInt8[0];
      int16_t dataInt16[0];
      int32_t dataInt32[0];
      int64_t dataInt64[0];
      double dataDouble[0];
      float dataFloat[0];
    } data;

};

#ifndef __cplusplus
typedef struct ZIVectorData ZIVectorData;
#endif

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(pop)
#endif


struct ZIAsyncReply {
// --- 8 bytes border
  /// Time stamp of the reply (server clock)
  ZITimeStamp timeStamp;

// --- 8 bytes border
  /// Time stamp of the target node sample, to which the reply belongs
  ZITimeStamp sampleTimeStamp;

// --- 8 bytes border
  /// Command:
  ///   1 - ziAPIAsyncSetDoubleData
  ///   2 - ziAPIAsyncSetIntegerData
  ///   3 - ziAPIAsyncSetByteArray
  ///   4 - ziAPIAsyncSubscribe
  ///   5 - ziAPIAsyncUnSubscribe
  ///   6 - ziAPIAsyncGetValueAsPollData
  uint16_t command;

  /// Command result code (cast to ZIResult_enum)
  uint16_t resultCode;

  /// Tag sent along with the async command
  ZIAsyncTag tag;
// --- 8 bytes border
};

#ifndef __cplusplus
typedef struct ZIAsyncReply ZIAsyncReply;
#endif


/** @defgroup Connection Connecting to Data Server
 *  @brief This section describes how to initialize the ZIConnection and establish a connection to Data Server as well
 *         as how to disconnect after all data handling is done and cleanup the ZIConnection.
 *  @include ExampleConnection.c
 *  @{
 */

enum ZIAPIVersion_enum {
  ZI_API_VERSION_0 = 0,
  ZI_API_VERSION_1 = 1,
  ZI_API_VERSION_4 = 4,
  ZI_API_VERSION_5 = 5,
  ZI_API_VERSION_6 = 6,
  ZI_API_VERSION_MAX = ZI_API_VERSION_6
};

#ifndef __cplusplus
typedef enum ZIAPIVersion_enum ZIAPIVersion_enum;
#endif


#ifdef __cplusplus
struct ZIConnectionProxy;
#else
typedef void ZIConnectionProxy;
#endif

/// The ZIConnection is a connection reference; it holds information and helper variables about a connection to
/// the Data Server. There is nothing in this reference which the user user may use, so it is hidden and instead
/// a dummy pointer is used.  See ::ziAPIInit for how to create a ZIConnection.
/** This struct is intentionally left blank so that the ZIConnection does not
   have to be defined as void pointer. If it would be defined as void* the
   compiler won't give a warning, when a void** is given to a funtction since
   a void** is also a void*. Blank struct however is not supported by C89
   standard, therefore in this case for compatibility ZIConnection is defined
   as plain void*.
 */
typedef ZIConnectionProxy* ZIConnection;

/// Initializes a ::ZIConnection structure.
/** This function initializes the structure so that it is ready to connect to Data Server. It allocates memory and sets
    up the infrastructure needed.

    @param[out]  conn  Pointer to ::ZIConnection that is to be initialized
    @return
    - ZI_INFO_SUCCESS  on success
    - ZI_ERROR_MALLOC  on memory allocation failure
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link Connection Connection @endlink for an example
    @sa ziAPIDestroy, ziAPIConnect, ziAPIDisconnect
 */
ZI_EXPORT ZIResult_enum ziAPIInit(ZIConnection* conn);

/// Destroys a ::ZIConnection structure.
/** This function frees all memory that has been allocated by ::ziAPIInit. If it is called with an uninitialized
    ::ZIConnection struct it may result in segmentation faults as well when it is called with a struct for which
    ZIAPIDestroy already has been called.

    @param[in] conn Pointer to ::ZIConnection struct that has to be destroyed
    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link Connection Connection @endlink for an example
    @sa ziAPIInit, ziAPIConnect, ziAPIDisconnect
 */
ZI_EXPORT ZIResult_enum ziAPIDestroy(ZIConnection conn);

/// Connects the ZIConnection to Data Server.
/** Connects to Data Server using a ::ZIConnection and prepares for data exchange. For most cases it is enough to just
    give a reference to the connection and give NULL for hostname and 0 for the port, so it connects to localhost
    on the default port.

    @param[in]   conn      Pointer to ::ZIConnection with which the connection should be established
    @param[in]   hostname  Name of the Host to which it should be connected, if NULL "localhost" will be used as
                           default
    @param[in]   port      The Number of the port to connect to. If 0, default port of the local Data Server will be
                           used (8005)
    @return
    - ZI_INFO_SUCCESS          on success
    - ZI_ERROR_HOSTNAME        if the given host name could not be found
    - ZI_ERROR_SOCKET_CONNECT  if no connection could be established
    - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
    - ZI_ERROR_SOCKET_INIT     if initialization of the socket failed
    - ZI_ERROR_CONNECTION      when the Data Server didn't return the correct answer
    - ZI_ERROR_TIMEOUT         when initial communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link Connection Connection @endlink for an example
    @sa ziAPIDisconnect, ziAPIInit, ziAPIDestroy
 */
ZI_EXPORT ZIResult_enum ziAPIConnect(ZIConnection conn, const char* hostname, uint16_t port);

/// Disconnects an established connection.
/** Disconnects from Data Server. If the connection has not been established and the function is called it returns
    without doing anything.

    @param[in]   conn  Pointer to ZIConnection to be disconnected
    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link Connection Connection @endlink for an example
    @sa ziAPIConnect, ziAPIInit, ziAPIDestroy
 */
ZI_EXPORT ZIResult_enum ziAPIDisconnect(ZIConnection conn);

/// Returns the list of supported implementations
/**
    Returned names are defined by implementations in the linked library and may change depending on software version.

     @param[out] implementations  Pointer to a buffer receiving a newline-delimited list of the names of all
                                  the supported ziAPI implementations. The string is zero-terminated.
     @param[in]  bufferSize       The size of the buffer assigned to the implementations parameter
     @return
     - ZI_INFO_SUCCESS            on success
     - ZI_ERROR_LENGTH            if the length of the char-buffer given by MaxLen is too small for all elements
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIConnectEx
 */
ZI_EXPORT ZIResult_enum ziAPIListImplementations(char* implementations, uint32_t bufferSize);

/// Connects to Data Server and enables extended ziAPI.
/**
    With apiLevel=ZI_API_VERSION_1 and implementation=NULL, this call is equivalent to plain ::ziAPIConnect.
    With other version and implementation values enables corresponding ziAPI extension and connection using
    different implementation.

     @param[in]  conn            Pointer to the ZIConnection with which the connection should be established
     @param[in]  hostname        Name of the host to which it should be connected, if NULL "localhost" will be used
                                 as default
     @param[in]  port            The number of the port to connect to. If 0 the port of the local Data Server
                                 will be used
     @param[in]  apiLevel        Specifies the ziAPI compatibility level to use for this connection (1 or 4).
     @param[in]  implementation  Specifies implementation to use for a connection, must be one of the returned by
                                 ziAPIListImplementations or NULL to select default implementation
     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_HOSTNAME         if the given host name could not be found
     - ZI_ERROR_SOCKET_CONNECT   if no connection could be established
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_SOCKET_INIT      if initialization of the socket failed
     - ZI_ERROR_CONNECTION       when the Data Server didn't return the correct answer or requested implementation
                                 is not found or doesn't support requested ziAPI level
     - ZI_ERROR_TIMEOUT          when initial communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     See @link Connection Connection @endlink for an example
     @sa ziAPIListImplementations, ziAPIConnect, ziAPIDisconnect, ziAPIInit, ziAPIDestroy, ziAPIGetConnectionVersion
 */
ZI_EXPORT ZIResult_enum ziAPIConnectEx(ZIConnection conn, const char* hostname, uint16_t port,
                                       ZIAPIVersion_enum apiLevel, const char* implementation);

/// Returns ziAPI level used for the connection conn.
/**
     @param[in]  conn       Pointer to ZIConnection
     @param[out] apiLevel   Pointer to preallocated ZIAPIVersion_enum, receiving the ziAPI level
     @return
     - ZI_INFO_SUCCESS      on success
     - ZI_ERROR_CONNECTION  if level can not be determined due to conn is not connected

     @sa ziAPIConnectEx, ziAPIGetVersion, ziAPIGetRevision
 */
ZI_EXPORT ZIResult_enum ziAPIGetConnectionAPILevel(ZIConnection conn, ZIAPIVersion_enum* apiLevel);

/// Retrieves the release version of ziAPI
/** Sets the passed pointer to point to the null-terminated release version string of ziAPI.

    @param[in]   version  Pointer to const char pointer.
    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIConnectEx, ziAPIGetRevision, ziAPIGetConnectionAPILevel
*/
ZI_EXPORT ZIResult_enum ziAPIGetVersion(const char** version);

/// Retrieves the revision of ziAPI
/** Sets an unsigned int with the revision (build number) of the ziAPI you are using.

    @param[in]   revision  Pointer to an unsigned int to fill up with the revision.
    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIConnectEx, ziAPIGetVersion, ziAPIGetConnectionAPILevel
 */
ZI_EXPORT ZIResult_enum ziAPIGetRevision(unsigned int* revision);


/** @} */  // end of group Connection


/** @defgroup TreeListing Tree
 *  @brief All parameters and streams are organized in a tree. You can list the whole tree, parts of it or single items
 *         using ::ziAPIListNodes or you may update the tree with nodes of newly connected devices by using
 *         ::ziAPIUpdateDevices.
 *  @include ExampleListNodes.c
 *  @{
 */

/// Defines the values of the flags used in ::ziAPIListNodes
enum ZIListNodes_enum {
  /// Default, return a simple listing of the given node immediate descendants.
  ZI_LIST_NODES_NONE = 0x00,
  /// List the nodes recursively
  ZI_LIST_NODES_RECURSIVE = 0x01,
  /// Return absolute paths
  ZI_LIST_NODES_ABSOLUTE = 0x02,
  /// Return only leaf nodes, which means the nodes at the outermost level of the tree
  ZI_LIST_NODES_LEAFSONLY = 0x04,
  /// Return only nodes which are marked as setting
  ZI_LIST_NODES_SETTINGSONLY = 0x08,
  /// Return only streaming nodes (nodes that can be pushed from the device at a high data rate)
  ZI_LIST_NODES_STREAMINGONLY = 0x10,
  /// Return only nodes that are subscribed to in the API session
  ZI_LIST_NODES_SUBSCRIBEDONLY = 0x20,
  // Return only one instance of a node in case of multiple channels
  ZI_LIST_NODES_BASECHANNEL = 0x40,
  // Return only nodes which can be used with the get command
  ZI_LIST_NODES_GETONLY = 0x80
  /// @cond deprecated
  // **********
#ifdef _MSC_VER
  ,
  // Default, return a simple listing of the given node immediate descendants.
  ZI_LIST_NONE           = 0x00,
  // List the nodes recursively
  ZI_LIST_RECURSIVE      = 0x01,
  // Return absolute paths
  ZI_LIST_ABSOLUTE       = 0x02,
  // Return only leaf nodes, which means the nodes at the outermost level of the tree
  ZI_LIST_LEAFSONLY      = 0x04,
  // Return only nodes which are marked as setting
  ZI_LIST_SETTINGSONLY   = 0x08
#endif
  /// @endcond
};

#ifndef __cplusplus
typedef enum ZIListNodes_enum ZIListNodes_enum;
#endif

#ifdef _MSC_VER
#pragma deprecated(ZI_LIST_NONE)
#pragma deprecated(ZI_LIST_RECURSIVE)
#pragma deprecated(ZI_LIST_ABSOLUTE)
#pragma deprecated(ZI_LIST_LEAFSONLY)
#pragma deprecated(ZI_LIST_SETTINGSONLY)
#else

#ifdef __cplusplus
#define ZIListNodes_enum_cast(val) ZIListNodes_enum(val)
#else
#define ZIListNodes_enum_cast(val) val
#endif

DEPRECATED_ENUM(const ZIListNodes_enum ZI_LIST_NONE           = ZIListNodes_enum_cast(0x00));
DEPRECATED_ENUM(const ZIListNodes_enum ZI_LIST_RECURSIVE      = ZIListNodes_enum_cast(0x01));
DEPRECATED_ENUM(const ZIListNodes_enum ZI_LIST_ABSOLUTE       = ZIListNodes_enum_cast(0x02));
DEPRECATED_ENUM(const ZIListNodes_enum ZI_LIST_LEAFSONLY      = ZIListNodes_enum_cast(0x04));
DEPRECATED_ENUM(const ZIListNodes_enum ZI_LIST_SETTINGSONLY   = ZIListNodes_enum_cast(0x08));
#endif

/// Returns all child nodes found at the specified path
/** This function returns a list of node names found at the specified path. The path may contain wildcards so that
    the returned nodes do not necessarily have to have the same parents. The list is returned in a null-terminated
    char-buffer, each element delimited by a newline. If the maximum length of the buffer (bufferSize) is not
    sufficient for all elements, nothing will be returned and the return value will be ZIResult_enum::ZI_LENGTH.

    @param[in]   conn        Pointer to the ZIConnection for which the node names should be retrieved.
    @param[in]   path        Path for which all children will be returned. The path may contain wildcard characters.
    @param[out]  nodes       Upon call filled with newline-delimited list of the names of all the children found.
                             The string is zero-terminated.
    @param[in]   bufferSize  The length of the buffer used for the nodes output parameter.
    @param[in]   flags       A combination of flags (applied bitwise) as defined in ::ZIListNodes_enum.

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds ::MAX_PATH_LEN or the length of the char-buffer for the
                                nodes given by bufferSize is too small for all elements
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link TreeListing Tree Listing @endlink for an example

    @sa ziAPIUpdate
 */
ZI_EXPORT ZIResult_enum ziAPIListNodes(ZIConnection conn, const char* path, char* nodes, uint32_t bufferSize,
                                       uint32_t flags);

/// Returns all child nodes found at the specified path
/** This function returns a list of node names found at the specified path, formatted as JSON. The path may
    contain wildcards so that the returned nodes do not necessarily have to have the same parents.
    The list is returned in a null-terminated char-buffer. If the maximum length of the buffer (bufferSize) is not
    sufficient for all elements, nothing will be returned and the return value will be ZIResult_enum::ZI_LENGTH.

    @param[in]   conn        Pointer to the ZIConnection for which the node names should be retrieved.
    @param[in]   path        Path for which all children will be returned. The path may contain wildcard characters.
    @param[out]  nodes       Upon call filled with JSON-formatted list of the names of all the children found.
                             The string is zero-terminated.
    @param[in]   bufferSize  The length of the buffer used for the nodes output parameter.
    @param[in]   flags       A combination of flags (applied bitwise) as defined in ::ZIListNodes_enum.

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds ::MAX_PATH_LEN or the length of the char-buffer for the
                                nodes given by bufferSize is too small for all elements
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    See @link TreeListing Tree Listing @endlink for an example

    @sa ziAPIUpdate
 */
ZI_EXPORT ZIResult_enum ziAPIListNodesJSON(ZIConnection conn, const char* path, char* nodes, uint32_t bufferSize,
                                           uint32_t flags);

/** @} */  // end of group TreeListing

/// Search for the newly connected devices and update the tree
/** This function forces the Data Server to search for newly connected devices and to connect to run them

    @param[in]   conn  Pointer to ZIConnection
    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIListNodes
 */
ZI_EXPORT ZIResult_enum ziAPIUpdateDevices(ZIConnection conn);

/// Connect a device to the server.
/** This function connects a device with deviceSerial via the specified deviceInterface for use with the server.

    @param[in]  conn            Pointer to the ZIConnection with which the connection should be established
    @param[in]  deviceSerial    The serial of the device to connect to, e.g., dev2100
    @param[in]  deviceInterface The interface to use for the connection, e.g., USB|1GbE
    @param[in]  interfaceParams Parameters for interface configuration (currently reserved for future use,
                                NULL may be specified).

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDisconnectDevice, ziAPIConnect, ziAPIDisconnect, ziAPIInit
 */
ZI_EXPORT ZIResult_enum ziAPIConnectDevice(ZIConnection conn, const char* deviceSerial,
                                           const char* deviceInterface, const char* interfaceParams);

/// Disconnect a device from the server.
/** This function disconnects a device specified by deviceSerial from the server.

    @param[in]  conn            Pointer to the ZIConnection with which the connection should be established
    @param[in]  deviceSerial    The serial of the device to connect to, e.g., dev2100

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIConnectDevice, ziAPIConnect, ziAPIDisconnect, ziAPIInit
 */
ZI_EXPORT ZIResult_enum ziAPIDisconnectDevice(ZIConnection conn, const char* deviceSerial);

/** @defgroup Parameters Set and Get Parameters
 *  @brief This section describes several functions for getting and setting parameters of different datatypes.
 *  @{
 */

/// gets the double-type value of the specified node
/** This function retrieves the numerical value of the specified node as an double-type value. The value first found
    is returned if more than one value is available (a wildcard is used in the path).

    @param[in]   conn   Pointer to ZIConnection with which the value should be retrieved
    @param[in]   path   Path to the node holding the value
    @param[out]  value  Pointer to a double in which the value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @include ExampleSetGet.cpp

    @sa ziAPISetValueD, ziAPIGetValueAsPollData
 */

ZI_EXPORT ZIResult_enum ziAPIGetValueD(ZIConnection conn, const char* path, ZIDoubleData* value);

/// gets the complex double-type value of the specified node
/** This function retrieves the numerical value of the specified node as an complex double-type value.
    The value first found is returned if more than one value is available (a wildcard is used in the path).

    @param[in]   conn   Pointer to ZIConnection with which the value should be retrieved
    @param[in]   path   Path to the node holding the value
    @param[out]  real   Pointer to a double in which the real value should be written
    @param[out]  imag   Pointer to a double in which the imaginary value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @include ExampleSetGet.cpp

    @sa ziAPISetComplexData, ziAPIGetValueAsPollData
 */

ZI_EXPORT ZIResult_enum ziAPIGetComplexData(ZIConnection conn, const char* path, ZIDoubleData* real, ZIDoubleData* imag);


/// gets the integer-type value of the specified node
/** This function retrieves the numerical value of the specified node as an integer-type value. The value first
    found is returned if more than one value is available (a wildcard is used in the path).

    @param[in]   conn   Pointer to ZIConnection with which the value should be retrieved
    @param[in]   path   Path to the node holding the value
    @param[out]  value  Pointer to an 64bit integer in which the value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPISetValueI, ziAPIGetValueAsPollData
 */
ZI_EXPORT ZIResult_enum ziAPIGetValueI(ZIConnection conn, const char* path, ZIIntegerData* value);

/// Gets the demodulator sample value of the specified node
/** This function retrieves the value of the specified node as an DemodSample struct. The value first found is
    returned if more than one value is available (a wildcard is used in the path). This function is only applicable
    to paths matching DEMODS/[0-9]+/SAMPLE.

    @param[in]   conn   Pointer to ZIConnection with which the value should be retrieved
    @param[in]   path   Path to the node holding the value
    @param[out]  value  Pointer to a ZIDemodSample struct in which the value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @include ExampleGetS.c

    @sa ziAPIGetValueAsPollData
 */
ZI_EXPORT ZIResult_enum ziAPIGetDemodSample(ZIConnection conn, const char* path, ZIDemodSample* value);

/// Gets the Digital I/O sample of the specified node
/** This function retrieves the newest available DIO sample from the specified node. The value first found
    is returned if more than one value is available (a wildcard is used in the path). This function is only
    applicable to nodes ending in "/DIOS/[0-9]+/INPUT".

    @param[in]   conn   Pointer to the ZIConnection with which the value should be retrieved
    @param[in]   path   Path to the node holding the value
    @param[out]  value  Pointer to a ZIDIOSample struct in which the value should be written
    @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH           if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for
                                 the nodes given by MaxLen is too small for all elements
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_COMMAND          on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @include ExampleGetDIO.c

    @sa ziAPIGetValueAsPollData
 */
ZI_EXPORT ZIResult_enum ziAPIGetDIOSample(ZIConnection conn, const char* path, ZIDIOSample* value);

/// gets the AuxIn sample of the specified node
/** This function retrieves the newest available AuxIn sample from the specified node. The value first found
    is returned if more than one value is available (a wildcard is used in the path). This function is only
    applicable to nodes ending in "/AUXINS/[0-9]+/SAMPLE".

    @param[in] conn Pointer to the ziConnection with which the Value should be retrieved
    @param[in] path Path to the Node holding the value
    @param[out] value Pointer to an ZIAuxInSample struct in which the value should be written
    @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH           if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for the
                                 nodes given by MaxLen is too small for all elements
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_COMMAND          on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @include ExampleGetAuxIn.c

    @sa ziAPIGetValueAsPollData
 */
ZI_EXPORT ZIResult_enum ziAPIGetAuxInSample(ZIConnection conn, const char* path, ZIAuxInSample* value);

  /// gets the Bytearray value of the specified node
  /**
     This function retrieves the newest available DIO sample from the specified node. The value first found
     is returned if more than one value is available (a wildcard is used in the path).

     @param[in]  conn           Pointer to the ziConnection with which the value should be retrieved
     @param[in]  path           Path to the Node holding the value
     @param[out] buffer         Pointer to a buffer to store the retrieved data in
     @param[out] length         Pointer to an unsigned int to store the length of data in. if an error occurred or
                                the length of the passed buffer is insufficient, a zero will be returned
     @param[in]  bufferSize     The length of the passed buffer

     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for
                                the nodes given by MaxLen is too small for all elements.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @include ExampleGetB.c

     @sa ziAPISetValueB, ziAPIGetValueAsPollData

  */
ZI_EXPORT ZIResult_enum ziAPIGetValueB(ZIConnection conn, const char* path, unsigned char* buffer,
                                       unsigned int* length, unsigned int bufferSize);

  /// gets a null-terminated string value of the specified node
  /**
     This function retrieves the newest string value for the specified node. The value first found
     is returned if more than one value is available (a wildcard is used in the path).

     @param[in]  conn           Pointer to the ziConnection with which the value should be retrieved
     @param[in]  path           Path to the Node holding the value
     @param[out] buffer         Pointer to a buffer to store the retrieved null-terminated string
     @param[out] length         Pointer to an unsigned int to store the length of the string in
                                (including the null terminator). If an error occurred or
                                the length of the passed buffer is insufficient, a zero will be returned
     @param[in]  bufferSize     The length of the passed buffer

     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for
                                the nodes given by MaxLen is too small for all elements.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPISetValueString, ziAPIGetValueAsPollData

  */
ZI_EXPORT ZIResult_enum ziAPIGetValueString(ZIConnection conn, const char* path, char* buffer,
                                            unsigned int* length, unsigned int bufferSize);

  /// gets a null-terminated string value of the specified node
  /**
     This function retrieves the newest unicode string value for the specified node. The value first found
     is returned if more than one value is available (a wildcard is used in the path).

     @param[in]  conn           Pointer to the ziConnection with which the value should be retrieved
     @param[in]  path           Path to the Node holding the value
     @param[out] wbuffer        Pointer to a buffer to store the retrieved null-terminated string
     @param[out] length         Pointer to an unsigned int to store the length of the string in
                                (including the null terminator). If an error occurred or
                                the length of the passed buffer is insufficient, a zero will be returned
     @param[in]  bufferSize     The length of the passed buffer

     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for
                                the nodes given by MaxLen is too small for all elements.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPISetValueStringUnicode, ziAPIGetValueAsPollData

  */
ZI_EXPORT ZIResult_enum ziAPIGetValueStringUnicode(ZIConnection conn, const char* path, wchar_t* wbuffer,
                                                   unsigned int* length, unsigned int bufferSize);

  /// asynchronously sets a double-type value to one or more nodes specified in the path
  /**
     This function sets the values of the nodes specified in path to Value. More than one value can be set if
     a wildcard is used. The function sets the value asynchronously which means that after the function returns
     you have no security to which value it is finally set nor at what point in time it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set.
     @param[in] path            Path to the Node(s) for which the value(s) will be set to Value.
     @param[in] value           The double-type value that will be written to the node(s).
     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
     - ZI_ERROR_READONLY        on attempt to set a read-only node.
     - ZI_ERROR_COMMAND         on an incorrect answer of the server.
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out.
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueD. ziAPISyncSetValueD
  */
ZI_EXPORT ZIResult_enum ziAPISetValueD(ZIConnection conn, const char* path, ZIDoubleData value);

  /// asynchronously sets a double-type complex value to one or more nodes specified in the path
/**
   This function sets the values of the nodes specified in path to the complex value (real, imag).
   More than one value can be set if a wildcard is used. The function sets the value asynchronously
   which means that after the function returns you have no security to which value it is finally
   set nor at what point in time it is set. If the node does not support complex values only the
   real value will be updated.

   @param[in] conn            Pointer to the ziConnection for which the value(s) will be set.
   @param[in] path            Path to the Node(s) for which the value(s) will be set to Value.
   @param[in] real            The real value that will be written to the node(s).
   @param[in] imag            The imag value that will be written to the node(s).
   @return
   - ZI_INFO_SUCCESS          on success.
   - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
   - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
   - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
   - ZI_ERROR_READONLY        on attempt to set a read-only node.
   - ZI_ERROR_COMMAND         on an incorrect answer of the server.
   - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
   - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
   - ZI_ERROR_TIMEOUT         when communication timed out.
   - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

   @sa ziAPIGetComplexData. ziAPISyncSetComplexData
*/
ZI_EXPORT ZIResult_enum ziAPISetComplexData(ZIConnection conn, const char* path, ZIDoubleData real, ZIDoubleData imag);


  /// asynchronously sets an integer-type value to one or more nodes specified in a path
  /**
     This function sets the values of the nodes specified in path to Value. More than one value can be set if
     a wildcard is used. The function sets the value asynchronously which means that after the function returns
     you have no security to which value it is finally set nor at what point in time it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the Node(s) for which the value(s) will be set
     @param[in] value           The int-type value that will be written to the node(s)
     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
     - ZI_ERROR_READONLY        on attempt to set a read-only node.
     - ZI_ERROR_COMMAND         on an incorrect answer of the server.
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out.
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueI. ziAPISyncSetValueI

  */
ZI_EXPORT ZIResult_enum ziAPISetValueI(ZIConnection conn, const char* path, ZIIntegerData value);

  /// asynchronously sets the binary-type value of one or more nodes specified in the path
  /**
     This function sets the values at the nodes specified in a path. More than one value can be set if
     a wildcard is used. The function sets the value asynchronously which means that after the function
     returns you have no security to which value it is finally set nor at what point in time it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the Node(s) for which the value(s) will be set
     @param[in] buffer          Pointer to the byte array with the data
     @param[in] length          Length of the data in the buffer
     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
     - ZI_ERROR_READONLY        on attempt to set a read-only node.
     - ZI_ERROR_COMMAND         on an incorrect answer of the server.
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able
                                to hold values.
     - ZI_ERROR_TIMEOUT         when communication timed out.
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @include ExampleSetB.c

     @sa ziAPIGetValueB. ziAPISyncSetValueB

  */
ZI_EXPORT ZIResult_enum ziAPISetValueB(ZIConnection conn, const char* path, unsigned char* buffer, unsigned int length);

  /// asynchronously sets a string value of one or more nodes specified in the path
  /**
     This function sets the values at the nodes specified in a path. More than one value can be set if
     a wildcard is used. The function sets the value asynchronously which means that after the function
     returns you have no security to which value it is finally set nor at what point in time it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the Node(s) for which the value(s) will be set
     @param[in] str             Pointer to a null terminated string (max 64k characters)
     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
     - ZI_ERROR_READONLY        on attempt to set a read-only node.
     - ZI_ERROR_COMMAND         on an incorrect answer of the server.
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able
                                to hold values.
     - ZI_ERROR_TIMEOUT         when communication timed out.
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueString. ziAPISyncSetValueString

  */
ZI_EXPORT ZIResult_enum ziAPISetValueString(ZIConnection conn, const char* path, const char* str);

  /// asynchronously sets a unicode encoded string value of one or more nodes specified in the path
  /**
     This function sets the values at the nodes specified in a path. More than one value can be set if
     a wildcard is used. The function sets the value asynchronously which means that after the function
     returns you have no security to which value it is finally set nor at what point in time it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the Node(s) for which the value(s) will be set
     @param[in] wstr            Pointer to a null terminated unicode string (max 64k characters)
     @return
     - ZI_INFO_SUCCESS          on success.
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred.
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN.
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred.
     - ZI_ERROR_READONLY        on attempt to set a read-only node.
     - ZI_ERROR_COMMAND         on an incorrect answer of the server.
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server.
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able
                                to hold values.
     - ZI_ERROR_TIMEOUT         when communication timed out.
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueStringUnicode. ziAPISyncSetValueStringUnicode

  */
ZI_EXPORT ZIResult_enum ziAPISetValueStringUnicode(ZIConnection conn, const char* path, const wchar_t* wstr);

  /// synchronously sets a double-type value to one or more nodes specified in the path
  /**
     This function sets the values of the nodes specified in path to Value. More than one value can be set if
     a wildcard is used. The function sets the value synchronously. After returning you know that it is set and
     to which value it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the Node(s) for which the value(s) will be set to value
     @param[in] value           Pointer to a double-type containing the value to be written. When the function returns
                                value holds the effectively written value.
     @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_READONLY        on attempt to set a read-only node
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueD, ziAPISetValueD
  */
ZI_EXPORT ZIResult_enum ziAPISyncSetValueD(ZIConnection conn, const char* path, ZIDoubleData* value);

  /// synchronously sets an integer-type value to one or more nodes specified in a path
  /**
     This function sets the values of the nodes specified in path to value. More than one value can be set if
     a wildcard is used. The function sets the value synchronously. After returning you know that it is set and
     to which value it is set.

     @param[in] conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in] path            Path to the node(s) for which the value(s) will be set
     @param[in] value           Pointer to a int-type containing then value to be written. when the function returns
                                value holds the effectively written value.

     @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_READONLY        on attempt to set a read-only node
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueI, ziAPISetValueI

  */
ZI_EXPORT ZIResult_enum ziAPISyncSetValueI(ZIConnection conn, const char* path, ZIIntegerData* value);

/// Synchronously sets the binary-type value of one ore more nodes specified in the path
/** This function sets the values at the nodes specified in a path. More than one value can be set if a wildcard
    is used. This function sets the value synchronously. After returning you know that it is set and to which
    value it is set.

     @param[in]  conn            Pointer to the ziConnection for which the value(s) will be set
     @param[in]  path            Path to the Node(s) for which the value(s) will be set
     @param[in]  buffer          Pointer to the byte array with the data
     @param[in]  length          Length of the data in the buffer
     @param[in]  bufferSize      Length of the data in the buffer
     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH           if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_READONLY         on attempt to set a read-only node
     - ZI_ERROR_COMMAND          on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no node given by path is able to hold
                                 values
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueB, ziAPISetValueB
*/
ZI_EXPORT ZIResult_enum ziAPISyncSetValueB(ZIConnection conn, const char* path, uint8_t* buffer, uint32_t* length,
                                           uint32_t bufferSize);

/// Synchronously sets a string value of one or more nodes specified in the path
/** This function sets the values at the nodes specified in a path. More than one value can be set if a wildcard
    is used. This function sets the value synchronously. After returning you know that it is set.

     @param[in]     conn         Pointer to the ziConnection for which the value(s) will be set
     @param[in]     path         Path to the Node(s) for which the value(s) will be set
     @param[inout]  str          Pointer to a null terminated string (max 64k characters)
     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH           if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_READONLY         on attempt to set a read-only node
     - ZI_ERROR_COMMAND          on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no node given by path is able to hold
                                 values
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueString, ziAPISetValueString
*/
ZI_EXPORT ZIResult_enum ziAPISyncSetValueString(ZIConnection conn, const char* path, const char* str);

/// Synchronously sets a unicode string value of one or more nodes specified in the path
/** This function sets the values at the nodes specified in a path. More than one value can be set if a wildcard
    is used. This function sets the value synchronously. After returning you know that it is set.

     @param[in]     conn         Pointer to the ziConnection for which the value(s) will be set
     @param[in]     path         Path to the Node(s) for which the value(s) will be set
     @param[inout]  wstr         Pointer to a null terminated unicode string (max 64k characters)
     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH           if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
     - ZI_ERROR_READONLY         on attempt to set a read-only node
     - ZI_ERROR_COMMAND          on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no node given by path is able to hold
                                 values
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     @sa ziAPIGetValueStringUnicode, ziAPISetValueStringUnicode
*/
ZI_EXPORT ZIResult_enum ziAPISyncSetValueStringUnicode(ZIConnection conn, const char* path, const wchar_t* wstr);

/// Synchronizes the session by dropping all pending data.
/** This function drops any data that is pending for transfer. Any data (including poll data)
    retrieved afterwards is guaranteed to be produced not earlier than the call to ziAPISync. This ensures in
    particular that any settings made prior to the call to ziAPISync have been propagated to the device, and the
    data retrieved afterwards is produced with the new settings already set to the hardware. Note, however, that
    this does not include any required settling time.

     @param[in]  conn            Pointer to the ZIConnection that is to be synchronized

     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

 */
ZI_EXPORT ZIResult_enum ziAPISync(ZIConnection conn);

/// Sends an echo command to a device and blocks until answer is received.
/** This is useful to flush all buffers between API and device to enforce that further code is only executed after
    the device executed a previous command.
    Per device echo is only implemented for HF2. For other device types it is a synonym to ziAPISync, and
    deviceSerial parameter is ignored.

     @param[in]  conn            Pointer to the ZIConnection that is to be synchronized
     @param[in]  deviceSerial    The serial of the device to get the echo from, e.g., dev2100


     @return
     - ZI_INFO_SUCCESS           on success
     - ZI_ERROR_TIMEOUT          when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

 */
ZI_EXPORT ZIResult_enum ziAPIEchoDevice(ZIConnection conn, const char* deviceSerial);

/** @} */  // end of group Parameters


/** @defgroup DataHandling Data Streaming
 *  @brief This section describes how to perform data streaming. It allows for recording at high data rates without
 *         sample loss.
 *  @include ExampleSubscription.c
 *  @{
 */

/// This struct holds event data forwarded by the Data Server
/**
    ZIEvent is used to give out events like value changes or errors to the user. Event handling functionality is
    provided by ziAPISubscribe and ziAPIUnSubscribe as well as ziAPIPollDataEx.

    @include ExampleProcessEvent.c

    @sa ziAPISubscribe, ziAPIUnSubscribe, ziAPIPollDataEx
 */
struct ZIEvent {
// --- 8 bytes border
  /// Specifies the type of the data held by the ZIEvent, see ::ZIValueType_enum
  uint32_t valueType;

  /// Number of values available in this event
  uint32_t count;

// --- 8 bytes border
  /// The path to the node from which the event originates
  uint8_t path[MAX_PATH_LEN];

// --- 8 bytes border
  /// Convenience pointer to allow for access to the first entry in Data using the correct type
  /// according to ZIEvent.valueType field.
  union {
    /// For convenience. The void field doesn't have a corresponding data type.
    void* untyped;

    /// when valueType == ZI_VALUE_TYPE_DOUBLE_DATA
    ZIDoubleData* doubleData;

    /// when valueType == ZI_VALUE_TYPE_DOUBLE_DATA_TS
    ZIDoubleDataTS* doubleDataTS;

    /// when valueType == ZI_VALUE_TYPE_INTEGER_DATA
    ZIIntegerData* integerData;

    /// when valueType == ZI_VALUE_TYPE_INTEGER_DATA_TS
    ZIIntegerDataTS* integerDataTS;

    /// when valueType == ZI_VALUE_TYPE_COMPLEX_DATA
    ZIComplexData* complexData;

    /// when valueType == ZI_VALUE_TYPE_BYTE_ARRAY
    ZIByteArray* byteArray;

    /// when valueType == ZI_VALUE_TYPE_BYTE_ARRAY_TS
    ZIByteArrayTS* byteArrayTS;

    /// when valueType == ZI_VALUE_TYPE_CNT_SAMPLE
    ZICntSample* cntSample;

    /// when valueType == ZI_VALUE_TYPE_TRIG_SAMPLE
    ZITrigSample* trigSample;

    /// when valueType == ZI_VALUE_TYPE_TREE_CHANGE_DATA
    ZITreeChangeData* treeChangeData;

    /// when valueType == ZI_VALUE_TYPE_TREE_CHANGE_DATA_OLD
    TreeChange* treeChangeDataOld;

    /// when valueType == ZI_VALUE_TYPE_DEMOD_SAMPLE
    ZIDemodSample* demodSample;

    /// when valueType == ZI_VALUE_TYPE_AUXIN_SAMPLE
    ZIAuxInSample* auxInSample;

    /// when valueType == ZI_VALUE_TYPE_DIO_SAMPLE
    ZIDIOSample* dioSample;

    /// when valueType == ZI_VALUE_TYPE_SCOPE_WAVE
    ZIScopeWave* scopeWave;

    /// when valueType == ZI_VALUE_TYPE_SCOPE_WAVE_EX
    ZIScopeWaveEx* scopeWaveEx;

    /// when valueType == ZI_VALUE_TYPE_SCOPE_WAVE_OLD
    ScopeWave* scopeWaveOld;

    /// when valueType == ZI_VALUE_TYPE_PWA_WAVE
    ZIPWAWave* pwaWave;

    /// when valueType == ZI_VALUE_TYPE_SWEEPER_WAVE
    ZISweeperWave* sweeperWave;

    /// when valueType == ZI_VALUE_TYPE_SPECTRUM_WAVE
    ZISpectrumWave* spectrumWave;

    /// when valueType == ZI_VALUE_TYPE_ADVISOR_WAVE
    ZIAdvisorWave* advisorWave;

    /// when valueType == ZI_VALUE_TYPE_ASYNC_REPLY
    ZIAsyncReply* asyncReply;

    /// when valueType == ZI_VALUE_TYPE_VECTOR_DATA
    ZIVectorData* vectorData;

    /// when valueType == ZI_VALUE_TYPE_IMPEDANCE_SAMPLE
    ZIImpedanceSample* impedanceSample;

    /// ensure union size is 8 bytes
    uint64_t alignment;
  } value;

// --- 8 bytes border
  /// The raw value data.
  uint8_t data[MAX_EVENT_SIZE];
};

#ifndef __cplusplus
typedef struct ZIEvent ZIEvent;
#endif

/// Allocates ZIEvent structure and returns the pointer to it. Attention!!! It is the client code responsibility
/// to deallocate the structure by calling ziAPIDeallocateEventEx!
/** This function allocates a ZIEvent structure and returns the pointer to it. Free the memory
    using ziAPIDeallocateEventEx.

    @sa ziAPIDeallocateEventEx
*/
ZI_EXPORT ZIEvent* ziAPIAllocateEventEx();

/// Deallocates ZIEvent structure created with ziAPIAllocateEventEx().
/** This function is the compliment to ziAPIAllocateEventEx()
    @param[in] ev Pointer to ZIEvent structure to be deallocated..

    @sa ziAPIAllocateEventEx
*/
ZI_EXPORT void ziAPIDeallocateEventEx(ZIEvent* ev);

/// subscribes the nodes given by path for ::ziAPIPollDataEx
/** This function subscribes to nodes so that whenever the value of the node changes the new value can be polled
    using ::ziAPIPollDataEx. By using wildcards or by using a path that is not a leaf node but contains sub nodes,
    more than one leaf can be subscribed to with one function call.

    @param[in] conn             Pointer to the ziConnection for which to subscribe for
    @param[in] path             Path to the nodes to subscribe
    @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

   See @link DataHandling Data Handling @endlink for an example

   @sa ziAPIUnSubscribe, ziAPIPollDataEx, ziAPIGetValueAsPollData
 */
ZI_EXPORT ZIResult_enum ziAPISubscribe(ZIConnection conn, const char* path);

  /// unsubscribes to the nodes given by path
  /**
      This function is the complement to ::ziAPISubscribe. By using wildcards or by using a path that is not a leaf
      node but contains sub nodes, more than one node can be unsubscribed with one function call.

      @param[in] conn           Pointer to the ziConnection for which to unsubscribe for
      @param[in] path           Path to the Nodes to unsubscribe
      @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no node given by path is able to hold values
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     See @link DataHandling Data Handling @endlink for an example

     @sa ziAPISubscribe, ziAPIPollDataEx, ziAPIGetValueAsPollData

  */
ZI_EXPORT ZIResult_enum ziAPIUnSubscribe(ZIConnection conn, const char* path);
  /// checks if an event is available to read
  /**
     This function returns immediately if an event is pending. Otherwise it waits for an event for up to
     timeOutMilliseconds. All value changes that occur in nodes that have been subscribed to or in children of nodes
     that have been subscribed to are sent from the Data Server to the ziAPI session. For a description of how the data
     are available in the struct, refer to the documentation of struct ziEvent. When no event was available within
     timeOutMilliseconds, the ziEvent::Type field will be ZI_DATA_NONE and the ziEvent::Count field will be zero.
     Otherwise these fields hold the values corresponding to the event that occurred.

     @param[in]  conn                Pointer to the ::ZIConnection for which events should be received
     @param[out] ev                  Pointer to a ::ZIEvent struct in which the received event will be written
     @param[in]  timeOutMilliseconds Time to wait for an event in milliseconds. If -1 it will wait forever,
                                     if 0 the function returns immediately.
     @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     See @link DataHandling Data Handling @endlink for an example

     @sa ziAPISubscribe, ziAPIUnSubscribe, ziAPIGetValueAsPollData, ziEvent

  */
ZI_EXPORT ZIResult_enum ziAPIPollDataEx(ZIConnection conn, ZIEvent* ev, uint32_t timeOutMilliseconds);

  /// triggers a value request, which will be given back on the poll event queue
  /**
     Use this function to receive the value of one or more nodes as one or more events using ::ziAPIPollDataEx, even
     when the node is not subscribed or no value change has occurred.

     @param[in] conn            Pointer to the ::ZIConnection with which the value should be retrieved
     @param[in] path            Path to the Node holding the value

     @return
     - ZI_INFO_SUCCESS          on success
     - ZI_ERROR_CONNECTION      when the connection is invalid (not connected) or when a communication error occurred
     - ZI_ERROR_LENGTH          if the Path's Length exceeds MAX_PATH_LEN or the length of the char-buffer for
                                the nodes given by MaxLen is too small for all elements
     - ZI_WARNING_OVERFLOW      when a FIFO overflow occurred
     - ZI_ERROR_COMMAND         on an incorrect answer of the server
     - ZI_ERROR_SERVER_INTERNAL if an internal error occurred in the Data Server
     - ZI_WARNING_NOTFOUND      if the given path could not be resolved or no value is attached to the node
     - ZI_ERROR_TIMEOUT         when communication timed out
     - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

     See @link DataHandling Data Handling @endlink for an example

     @sa ziAPISubscribe, ziAPIUnSubscribe, ziAPIPollDataEx

   */
ZI_EXPORT ZIResult_enum ziAPIGetValueAsPollData(ZIConnection conn, const char* path);

/** @} */  // end of group DataHandling



/** @defgroup Asynchronous   API for fast asynchronous operation
 *  @brief Functions in this group are non-blocking, and on return only report errors that can be identified directly
 *         on a client side (e.g. not connected). Any further results (including errors like node not found) of
 *         the command processing is returned as a special event in poll data. Tags are used to match the asynchronous
 *         replies with the sent commands.
 *  @{
 */

ZI_EXPORT ZIResult_enum ziAPIAsyncSetDoubleData(ZIConnection conn, const char* path, ZIDoubleData value);
ZI_EXPORT ZIResult_enum ziAPIAsyncSetIntegerData(ZIConnection conn, const char* path, ZIIntegerData value);
ZI_EXPORT ZIResult_enum ziAPIAsyncSetByteArray(ZIConnection conn, const char* path, uint8_t* buffer, uint32_t length);
ZI_EXPORT ZIResult_enum ziAPIAsyncSetString(ZIConnection conn, const char* path, const char* str);
ZI_EXPORT ZIResult_enum ziAPIAsyncSetStringUnicode(ZIConnection conn, const char* path, const wchar_t* wstr);
ZI_EXPORT ZIResult_enum ziAPIAsyncSubscribe(ZIConnection conn, const char* path, ZIAsyncTag tag);
ZI_EXPORT ZIResult_enum ziAPIAsyncUnSubscribe(ZIConnection conn, const char* path, ZIAsyncTag tag);
ZI_EXPORT ZIResult_enum ziAPIAsyncGetValueAsPollData(ZIConnection conn, const char* path, ZIAsyncTag tag);

/** @} */  // end of group Asynchronous



/** @defgroup ErrorHandling Error Handling and Logging in the LabOne C API

 * @brief This section describes how to get more information when an error occurs.

   @{

   In general, two types of errors can occur when using ziAPI. The two types are distinguished by the origin of the
   error: Whether it occurred within ziAPI itself or whether it occurred internally in the Zurich Instruments Core
   library.

   All ziAPI functions (apart from a very few exceptions) return an exit code ::ZIResult_enum, which will be non-zero
   if the function call was not entirely successful. If the error originated in ziAPI itself, the exit code describes
   precisely the type of error that occurred (in other words, the exit code is not ZI_ERROR_GENERAL). In this case the
   error message corresponding to the exit code can be obtained with the function ::ziAPIGetError.

   However, if the error has occurred internally, the exit code will be ZI_ERROR_GENERAL. In this case, the exit code
   does not describe the type of error precisely, instead a detailed error message is available to the user which can
   be obtained with the function ::ziAPIGetLastError. The function ::ziAPIGetLastError may be used with any function
   that takes a ZIConnection as an input argument (with the exception of ::ziAPIInit, ::ziAPIDestroy, ::ziAPIConnect,
   ::ziAPIConnectEx) and is the recommended function to use, if applicable, otherwise ::ziAPIGetError should be used.

   The function ::ziAPIGetLastError was introduced in LabOne 15.11 due to the availability of \ref modules "ziCore
   Modules" in ziAPI - its not desirable in general to map every possible error to an exit code in ziAPI; what is more
   relevant is the associated error message.

   In addition to these two functions, ziAPI's log can be very helpful whilst debugging ziAPI-based programs. The log
   is not enabled by default; it's enabled by specifying a logging level with ::ziAPISetDebugLevel.

*/

/// Returns a description and the severity for a ::ZIResult_enum.
/** This function returns a static char pointer to a description string for the given ::ZIResult_enum error code.  It
    also provides a parameter returning the severity (info, warning, error). If the given error code does not exist a
    description for an unknown error and the base for an error will be returned. If a description or the base is not
    needed NULL may be passed. In general, it's recommended to use ::ziAPIGetLastError instead to get detailed error
    messages.

    @param[in]  result          A ::ZIResult_enum for which the description or base will be returned
    @param[out] buffer          A pointer to a char array to return the description. May be NULL if no description
                                is needed.
    @param[out] base            The severity for the provided Status parameter:
                                - ZI_INFO_BASE    For infos.
                                - ZI_WARNING_BASE For warnings.
                                - ZI_ERROR_BASE   For errors.

    @return
    - ZI_INFO_SUCCESS           Upon success.

*/
ZI_EXPORT ZIResult_enum ziAPIGetError(ZIResult_enum result, char** buffer, int* base);

/// Returns the message from the last error that occurred.
/** This function can be used to obtain the error message from the last error that occurred associated with the
    provided ::ZIConnection. If the last ziAPI call is successful, then the last error message returned by
    ziAPIGetError is empty. Only ziAPI function calls that take ::ZIConnection as an input argument influence the
    message returned by ziAPIGetLastError, if they do not take ::ZIConnection as an input argument the last error
    message will neither be reset to be empty or set to an error message (in the case of the error). There are some
    exceptions to this rule, ziAPIGetLastError can also not be used with ::ziAPIInit, ::ziAPIConnect, ::ziAPIConnectEx
    and ::ziAPIDestroy. Note, a call to ziAPIGetLastError will also reset the last error message to empty if its call
    was successful. Since the buffer is left unchanged in the case of an error occurring in the call to
    ziAPIGetLastError it is safest to initialize the buffer with a known value, for example, "ziAPIGetLastError was
    not successful".

    @param[in]  conn            The ::ZIConnection from which to get the error message.
    @param[out] buffer          A pointer to a char array to return the message.
    @param[in]  bufferSize      The length of the provided buffer.

    @return
    - ZI_INFO_SUCCESS           Upon success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
                                In this case the provided buffer is left unchanged.
    - ZI_ERROR_LENGTH           If the message's length exceeds the provided bufferSize, the message is truncated and
                                written to buffer.

*/
ZI_EXPORT ZIResult_enum ziAPIGetLastError(ZIConnection conn, char* buffer, uint32_t bufferSize);

/// Enable ziAPI's log and set the severity level of entries to be included in the log.
/** Calling this function enables ziAPI's log at the specified severity level. On Windows the logs can be found by
    navigating to the Zurich Instruments "Logs" folder entry in the Windows Start Menu: Programs -> Zurich Instruments
    -> LabOne Servers -> Logs. This will open an Explorer window displaying folders containing log files from various
    LabOne components, in particular, the <code>ziAPILog</code> folder contains logs from ziAPI. On Linux, the logs can
    be found at "/tmp/ziAPILog_USERNAME", where "USERNAME" is the same as the output of the "whoami" command.

    @param[in]  debugLevel      An integer specifying the log's severity level:
                                - trace:   0,
                                - info:    1,
                                - debug:   2,
                                - warning: 3,
                                - error:   4,
                                - fatal:   5,
                                - status:  6.

    @sa ziAPIWriteDebugLog
 */
ZI_EXPORT void ziAPISetDebugLevel(int32_t debugLevel);

/// Write a message to ziAPI's log with the specified severity.
/** This function may be used to write a message to ziAPI's log from client code to assist with debugging. Note, this
    function is only available if the implementation used in ::ziAPIConnectEx is "ziAPI_Core" (the default
    implementation). Also logging must be first enabled using ::ziAPISetDebugLevel.

    @param[in]  debugLevel      An integer specifying the severity of the message to write in the log:
                                - trace:   0,
                                - info:    1,
                                - debug:   2,
                                - warning: 3,
                                - error:   4,
                                - fatal:   5,
                                - status:  6.
    @param[in]  message         A character array comprising of the message to be written.

    @sa ziAPISetDebugLevel
*/
ZI_EXPORT void ziAPIWriteDebugLog(int32_t debugLevel, const char* message);

/** @} */  // end of group ErrorHandling

ZI_EXPORT ZIResult_enum ReadMEMFile(const char* filename, char* buffer, int32_t bufferSize, int32_t* bytesUsed);

/** @defgroup modules Using ziCore Modules in the LabOne C API

    @brief This sections describes ziAPI's interface for working with ziCore Modules. Modules provide a high-level
    interface for performing common measurement tasks such as sweeping data (Sweeper Module) or recording bursts of
    when certain trigger criteria have been fulfilled (Software Trigger Module). For an introduction to working with
    Modules please see the "ziCore Modules" section in the LabOne Programming Manual:
    https://www.zhinst.com/manuals/programming .

   @{
 */

/// Defines the flags returned in the chunk header for all modules.
enum ZIChunkHeaderFlags_enum {
  /// Indicates that the chunk data is complete. This flag will be set if data is read out from the module before the
  /// measurement (e.g. sweep) has finished.
  ZI_CHUNK_HEADER_FLAG_FINISHED       = 0x00000001,
  /// Unused.
  ZI_CHUNK_HEADER_FLAG_ROLLMODE       = 0x00000002,
  /// Indicates that dataloss has occurred.
  ZI_CHUNK_HEADER_FLAG_DATALOSS       = 0x00000004,
  /// Indcates that the data is valid.
  ZI_CHUNK_HEADER_FLAG_VALID          = 0x00000008,
  /// Indicates whether then chunk contains data (opposite to setting).
  ZI_CHUNK_HEADER_FLAG_DATA           = 0x00000010,
  /// Internal use only.
  ZI_CHUNK_HEADER_FLAG_DISPLAY        = 0x00000020,
  /// chunk contains frequency domain data, opposite to time domain.
  ZI_CHUNK_HEADER_FLAG_FREQDOMAIN     = 0x00000040,
  /// chunk recorded in spectrum mode.
  ZI_CHUNK_HEADER_FLAG_SPECTRUM       = 0x00000080,
  /// chunk results overlap with neighboring chunks, see spectrum.
  ZI_CHUNK_HEADER_FLAG_OVERLAPPED     = 0x00000100,
  /// indicates that the current row is finished - useful for row first averaging.
  ZI_CHUNK_HEADER_FLAG_ROWFINISHED    = 0x00000200,
  /// exact sampling was used.
  ZI_CHUNK_HEADER_FLAG_ONGRIDSAMPLING = 0x00000400,
  /// row first averaging is enabled.
  ZI_CHUNK_HEADER_FLAG_ROWREPETITION  = 0x00000800,
  /// chunk contains preview (fft with less points).
  ZI_CHUNK_HEADER_FLAG_PREVIEW        = 0x00001000,
};

/// Defines flags returned in the chunk header that only apply for certain modules.
enum ZIChunkHeaderModuleFlags_enum {
  // Data Acquisition Module
  /// FFT Window used in the Data Acquisition Module: 0 - Rectangular, 1 - Hann, 2 - Hamming, 3 - Blackmanharris,
  /// 4 - Exponential, 5 - Cosine, 6 - CosineSquare
  ZI_CHUNK_HEADER_MODULE_FLAGS_WINDOW = 0x00000003

};

/// Structure to hold generic chunk data header information
struct ZIChunkHeader {
  /// System timestamp
  ZITimeStamp systemTime;

  /// Creation timestamp
  ZITimeStamp createdTimeStamp;

  /// Last changed timestamp
  ZITimeStamp changedTimeStamp;

// --- 8 bytes border
  /// Flags (bitmask of values from ZIChunkHeaderFlags_enum)
  uint32_t flags;

  /// moduleFlags (bitmask of values from ZIChunkHeaderModuleFlags_enum, module-specific)
  uint32_t moduleFlags;

// --- 8 bytes border
  /// Status Flag:
  /// [0] : selected
  /// [1] : group assigned
  /// [2] : color edited
  /// [4] : name edited
  uint32_t status;

  uint32_t reserved0;

// --- 8 bytes border
  /// Size in bytes used for memory usage calculation
  uint64_t chunkSizeBytes;

  /// SW Trigger counter since execution start
  uint64_t triggerNumber;

// --- 8 bytes border
  /// Name in history list
  char name[MAX_NAME_LEN];

  /// Group index in history list
  uint32_t groupIndex;

// --- 8 bytes border
  /// Color in history list
  uint32_t color;

  /// Active row in history list
  uint32_t activeRow;

// --- 8 bytes border
  /// Number of grid rows
  uint32_t gridRows;

  /// Number of grid columns
  uint32_t gridCols;

// --- 8 bytes border
  /// Grid mode interpolation mode (0 = off, 1 = nearest, 2 = linear, 3 = Lanczos)
  uint32_t gridMode;

  /// Grid mode operation (0 = replace, 1 = average)
  uint32_t gridOperation;

// --- 8 bytes border
  /// Grid mode direction (0 = forward, 1 = revers, 2 = bidirectional)
  uint32_t gridDirection;

  /// Number of repetitions in grid mode
  uint32_t gridRepetitions;

// --- 8 bytes border
  /// Delta between grid points in SI unit
  double gridColDelta;

// --- 8 bytes border
  /// Offset of first grid point relative to trigger
  double gridColOffset;

// --- 8 bytes border
  /// Delta between grid rows in SI unit
  double gridRowDelta;

// --- 8 bytes border
  /// Delay of first grid row relative to trigger
  double gridRowOffset;

// --- 8 bytes border
  /// For FFT the bandwidth of the signal
  double bandwidth;

// --- 8 bytes border
  /// The FFT center frequency
  double center;

// --- 8 bytes border
  /// For FFT the normalized effective noise bandwidth
  double nenbw;
};

#ifndef __cplusplus
typedef struct ZIChunkHeader ZIChunkHeader;
#endif

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(push)
#pragma warning(disable:4200)
#endif

/// This struct holds data of a single chunk from module lookup
struct ZIModuleEvent {
  /// For internal use - never modify!
  uint64_t allocatedSize;

  /// Chunk header
  ZIChunkHeader* header;

  /// Defines location of stored ZIEvent
  ZIEvent value[0];
};

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(pop)
#endif

#ifndef __cplusplus
typedef struct ZIModuleEvent ZIModuleEvent;
#endif

/// The pointer to a Module's data chunk to read out, updated via ::ziAPIModGetChunk.
typedef ZIModuleEvent* ZIModuleEventPtr;

/// Create a ::ZIModuleHandle that can be used for asynchronous measurement tasks.
/** This function initializes a ziCore module and provides a pointer (handle) with which to access and work with it.
    Note that this function does not start the module's thread. Before the thread can be started (with
    ::ziAPIModExecute):
    - the device serial (e.g., "dev100") to be used with module must be specified via ::ziAPIModSetByteArray.
    - the desired data (node paths) to record during the measurement must be specified via ::ziAPIModSubscribe.
    The module's thread is stopped with ::ziAPIModClear.

    @param[in]  conn            The ::ZIConnection which should be used to initialize the module.
    @param[out] handle          Pointer to the initialized ::ZIModuleHandle, which from then on can be used to
                                reference the module.
    @param[in]  moduleId        The name specifying the type the module to create (only the following ziCore Modules
                                are currently supported in ziAPI):
                                - "sweep" to initialize an instance of the Sweeper Module.
                                - "record" to initialize an instance of the Software Trigger (Recorder) Module.
                                - "zoomFFT" to initialize an instance of the Spectrum Module.
                                - "deviceSettings" to initialize an instance to save/load device settings.
                                - "pidAdvisor" to initialize an instance of the PID Advisor Module.
                                - "awgModule" to initialize an instance of the AWG Compiler Module.
                                - "impedanceModule" to initialize an instance of the Impedance Compensation Module.
                                - "scopeModule" to initialize an instance of the Scope Module to assembly scope shots.
                                - "multiDeviceSyncModule" to initialize an instance of the Device Synchronization Module.
                                - "dataAcquisitionModule" to initialize an instance of the Data Acquisition Module.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred.
    - ZI_WARNING_NOTFOUND       if the provided moduleId was invalid.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModExecute, ziAPIModClear
 */
ZI_EXPORT ZIResult_enum ziAPIModCreate(ZIConnection conn, ZIModuleHandle* handle, const char* moduleId);

/// Sets a module parameter to the specified double type.
/** This function is used to configure (set) module parameters which have double types.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to set data on.
    @param[in]  path            Path to the module parameter path.
    @param[in]  value           The double data to write to the path.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSetIntegerData, ziAPIModSetByteArray, ziAPIModSetString
 */
ZI_EXPORT ZIResult_enum ziAPIModSetDoubleData(ZIConnection conn, ZIModuleHandle handle, const char* path,
                                              ZIDoubleData value);

/// Sets a module parameter to the specified integer type.
/** This function is used to configure (set) module parameters which have integer types.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to set data on.
    @param[in]  path            Path to the module parameter path.
    @param[in]  value           The integer data to write to the path.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSetDoubleData, ziAPIModSetByteArray, ziAPIModSetString
 */
ZI_EXPORT ZIResult_enum ziAPIModSetIntegerData(ZIConnection conn, ZIModuleHandle handle, const char* path,
                                               ZIIntegerData value);

/// Sets a module parameter to the specified byte array.
/** This function is used to configure (set) module parameters which have byte array types.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to set data on.
    @param[in]  path            Path to the module parameter path.
    @param[in]  buffer          Pointer to the byte array with the data.
    @param[in]  length          Length of the data in the buffer.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSetDoubleData, ziAPIModSetIntegerData, ziAPIModSetString
 */
ZI_EXPORT ZIResult_enum ziAPIModSetByteArray(ZIConnection conn, ZIModuleHandle handle, const char* path,
                                             uint8_t* buffer, uint32_t length);

/// Sets a module parameter to the specified null-terminated string
/** This function is used to configure (set) module parameters which have string types.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to set data on.
    @param[in]  path            Path to the module parameter path.
    @param[in]  str             Pointer to a null-terminated string (max 64k characters).

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSetDoubleData, ziAPIModSetIntegerData, ziAPIModSetByteArray
 */
ZI_EXPORT ZIResult_enum ziAPIModSetString(ZIConnection conn, ZIModuleHandle handle, const char* path,
                                          const char* str);

/// Sets a module parameter to the specified null-terminated unicode string
/** This function is used to configure (set) module parameters which have string types.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to set data on.
    @param[in]  path            Path to the module parameter path.
    @param[in]  wstr            Pointer to a null-terminated unicode string (max 64k characters).

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSetDoubleData, ziAPIModSetIntegerData, ziAPIModSetByteArray
 */
ZI_EXPORT ZIResult_enum ziAPIModSetStringUnicode(ZIConnection conn, ZIModuleHandle handle, const char* path,
                                                 const wchar_t* wstr);

/// gets the integer-type value of the specified node
/** This function is used to retrieve module parameter values of type integer.

    @param[in]  conn            Pointer to ZIConnection with which the value should be retrieved
    @param[in]  handle          The ::ZIModuleHandle specifying the module in which the nodes should be subscribed to.
    @param[in]  path            Path to the node holding the value
    @param[out] value           Pointer to an 64bit integer in which the value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

@sa ziAPIModGetDouble, ziApiModGetString
*/
ZI_EXPORT ZIResult_enum ziAPIModGetInteger(ZIConnection conn, ZIModuleHandle handle, const char* path, ZIIntegerData* value);

/// gets the double-type value of the specified node
/** This function is used to retrieve module parameter values of type floating point double.

    @param[in]  conn            Pointer to ZIConnection with which the value should be retrieved
    @param[in]  handle          The ::ZIModuleHandle specifying the module in which the nodes should be subscribed to.
    @param[in]  path            Path to the node holding the value
    @param[out] value           Pointer to an floating point double in which the value should be written
    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

@sa ziAPIModGetInteger, ziApiModGetString
*/
ZI_EXPORT ZIResult_enum ziAPIModGetDouble(ZIConnection conn, ZIModuleHandle handle, const char* path, ZIDoubleData* value);

/// gets the null-terminated string value of the specified node
/** This function is used to retrieve module parameter values of type string.

    @param[in]  conn            Pointer to the ziConnection with which the value should be retrieved
    @param[in]  handle          The ::ZIModuleHandle specifying the module in which the nodes should be subscribed to.
    @param[in]  path            Path to the Node holding the value
    @param[out] buffer          Pointer to a buffer to store the retrieved null-terminated string
    @param[out] length          Pointer to an unsigned int to store the length of the string in
                                (including the null terminator). If an error occurred or
                                the length of the passed buffer is insufficient, a zero will be returned
    @param[in]  bufferSize      The length of the passed buffer

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

@sa ziAPIModGetInteger, ziApiModGetDouble
*/
ZI_EXPORT ZIResult_enum ziAPIModGetString(ZIConnection conn, ZIModuleHandle handle, const char* path, char* buffer,
                                          unsigned int* length, unsigned int bufferSize);

/// gets the null-terminated string value of the specified node
/** This function is used to retrieve module parameter values of type string.

    @param[in]  conn            Pointer to the ziConnection with which the value should be retrieved
    @param[in]  handle          The ::ZIModuleHandle specifying the module in which the nodes should be subscribed to.
    @param[in]  path            Path to the Node holding the value
    @param[out] wbuffer          Pointer to a buffer to store the retrieved null-terminated string
    @param[out] length          Pointer to an unsigned int to store the length of the string in
                                (including the null terminator). If an error occurred or
                                the length of the passed buffer is insufficient, a zero will be returned
    @param[in]  bufferSize      The length of the passed buffer

    @return
    - ZI_INFO_SUCCESS           on success
    - ZI_ERROR_CONNECTION       when the connection is invalid (not connected) or when a communication error occurred
    - ZI_ERROR_LENGTH           if the path's length exceeds MAX_PATH_LEN
    - ZI_WARNING_OVERFLOW       when a FIFO overflow occurred
    - ZI_ERROR_COMMAND          on an incorrect answer of the server
    - ZI_ERROR_SERVER_INTERNAL  if an internal error occurred in Data Server
    - ZI_WARNING_NOTFOUND       if the given path could not be resolved or no value is attached to the node
    - ZI_ERROR_TIMEOUT          when communication timed out
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

@sa ziAPIModGetInteger, ziApiModGetDouble, ziAPIModGetString
*/
ZI_EXPORT ZIResult_enum ziAPIModGetStringUnicode(ZIConnection conn, ZIModuleHandle handle, const char* path, wchar_t* wbuffer,
                                                 unsigned int* length, unsigned int bufferSize);

/// Returns all child parameter node paths found under the specified parent module parameter path
/** This function returns a list of parameter names found at the specified path. The path may contain wildcards. The
    list is returned in a null-terminated char-buffer, each element delimited by a newline. If the maximum length of
    the buffer (bufferSize) is not sufficient for all elements, nothing will be returned and the return value will be
    ::ZI_ERROR_LENGTH. Note, the provided path must match the module being addressed, i.e., path must exactly start
    with "sweep/" for the Sweeper Module.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle from which the parameter names should be retrieved.
    @param[in]  path            Path for which all children will be returned. The path may contain wildcard characters.
    @param[out] nodes           Upon call filled with newline-delimited list of the names of all the children found.
                                The string is zero-terminated.
    @param[in]  bufferSize      The length of the buffer specified as the nodes output parameter.
    @param[in]  flags           A combination of flags (applied bitwise) as defined in ::ZIListNodes_enum.

    @return
    - ZI_INFO_SUCCESS           On success
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_LENGTH           If the path's length exceeds ::MAX_PATH_LEN or the length of the char-buffer for the
                                nodes given by bufferSize is too small for all elements.
    - ZI_WARNING_OVERFLOW       When a FIFO overflow occurred.
    - ZI_ERROR_COMMAND          On an incorrect answer of the server.
    - ZI_ERROR_SERVER_INTERNAL  If an internal error occurred in Data Server.
    - ZI_WARNING_NOTFOUND       If the given path could not be resolved.
    - ZI_ERROR_TIMEOUT          When communication timed out.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

 */
ZI_EXPORT ZIResult_enum ziAPIModListNodes(ZIConnection conn, ZIModuleHandle handle, const char* path, char* nodes,
                                          uint32_t bufferSize, uint32_t flags);

/// Subscribes to the nodes specified by path, these nodes will be recorded during module execution.
/** This function subscribes to nodes so that whenever the value of the node changes while the module is executing the
    new value will be accumulated and then read using ::ziAPIModRead. By using wildcards or by using a path that is not
    a leaf node but contains sub nodes, more than one leaf can be subscribed to with one function call.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module in which the nodes should be subscribed to.
    @param[in]  path            Path specifying the nodes to subscribe to, may contain wildcards.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or a general error occurred, enable
                                ziAPI's log for detailed information, see ::ziAPISetDebugLevel.
    - ZI_ERROR_LENGTH           If the Path's Length exceeds MAX_PATH_LEN.
    - ZI_WARNING_OVERFLOW       When a FIFO overflow occurred.
    - ZI_ERROR_COMMAND          On an incorrect answer of the server.
    - ZI_ERROR_SERVER_INTERNAL  If an internal error occurred in the Data Server.
    - ZI_WARNING_NOTFOUND       If the given path could not be resolved or no node given by path is able
                                to hold values.
    - ZI_ERROR_TIMEOUT          When communication timed out.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

   @sa ziAPIModUnSubscribe, ziAPIModRead
*/
ZI_EXPORT ZIResult_enum ziAPIModSubscribe(ZIConnection conn, ZIModuleHandle handle, const char* path);

/// Unsubscribes to the nodes specified by path.
/** This function is the complement to ::ziAPIModSubscribe. By using wildcards or by using a path that is not a leaf
    node but contains sub nodes, more than one node can be unsubscribed with one function call.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifyin the module in which the nodes should be unsubscribed
                                from.
    @param[in]  path            Path specifying the nodes to unsubscribe from, may contain wildcards.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_LENGTH           If the Path's Length exceeds MAX_PATH_LEN.
    - ZI_WARNING_OVERFLOW       When a FIFO overflow occurred.
    - ZI_ERROR_COMMAND          On an incorrect answer of the server.
    - ZI_ERROR_SERVER_INTERNAL  If an internal error occurred in the Data Server.
    - ZI_WARNING_NOTFOUND       If the given path could not be resolved or no node given by path is able
                                to hold values.
    - ZI_ERROR_TIMEOUT          When communication timed out.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModSubscribe, ziAPIModRead
*/
ZI_EXPORT ZIResult_enum ziAPIModUnSubscribe(ZIConnection conn, ZIModuleHandle handle, const char* path);

/// Starts the module's thread and its associated measurement task.
/** Once the module's parameters has been configured as required via, e.g. ::ziAPIModSetDoubleData, this function
    starts the module's thread. This starts the module's main measurement task which will run asynchronously.
    The thread will run until either the module has completed its task or until ::ziAPIModFinish is called.
    Subscription or unsubscription is not possible while the module is executing. The status of the module can be
    obtained with either ::ziAPIModFinished or ::ziAPIModProgress.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModCreate, ziAPIModProgress, ziAPIModFinish
*/
ZI_EXPORT ZIResult_enum ziAPIModExecute(ZIConnection conn, ZIModuleHandle handle);

/// Manually issue a trigger forcing data recording (SW Trigger Module only).
/** This function is used with the Software Trigger Module in order to manually issue a trigger in order to force
 *  recording of data. A burst of subscribed data will be recorded as configured via the SW Trigger's parameters as
 *  would a regular trigger event.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

*/
ZI_EXPORT ZIResult_enum ziAPIModTrigger(ZIConnection conn, ZIModuleHandle handle);

/// Queries the current state of progress of the module's measurement task.
/** This function can be used to query the module's progress in performing its current measurement task, the progress
    is returned as a double in [0, 1], where 1 indicates task completion.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[out] progress        A pointer to ZIDoubleData indicating the current progress of the module.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModExecute, ziAPIModFinish, ziAPIModFinished
*/
ZI_EXPORT ZIResult_enum ziAPIModProgress(ZIConnection conn, ZIModuleHandle handle, ZIDoubleData* progress);

/// Queries whether the module has finished its measurement task.
/** This function can be used to query whether the module has finished its task or not.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[out] finished        A pointer to ZIIntegerData, upon return this will be 0 if the module is still
                                executing or 1 if it has finished executing.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModExecute, ziAPIModFinish, ziAPIModProgress
*/
ZI_EXPORT ZIResult_enum ziAPIModFinished(ZIConnection conn, ZIModuleHandle handle, ZIIntegerData* finished);

/// Stops the module performing its measurement task.
/** This functions stops the module performing its associated measurement task and stops recording any data. The task
    and data recording may be restarted by calling ::ziAPIModExecute' again.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModProgress, ziAPIModFinished
*/
ZI_EXPORT ZIResult_enum ziAPIModFinish(ZIConnection conn, ZIModuleHandle handle);

/// Saves the currently accumulated data to file.
/** This function saves the currently accumulated data to a file. The path of the file to save data to is specified via
    the module's directory parameter.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[in]  fileName        The basename of the file to save the data in.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModExecute, ziAPIModFinish, ziAPIModFinished
*/
ZI_EXPORT ZIResult_enum ziAPIModSave(ZIConnection conn, ZIModuleHandle handle, const char* fileName);

/// Make the currently accumulated data available for use in the C program.
/** This function can be used to either read (get) module parameters, in this case a path that addresses the module
    must be specified, or it can be used to read out the currently accumulated data from subscribed nodes in the
    module. In either case the actual data must then be accessed by the user using ::ziAPIModNextNode and
    ::ziAPIModGetChunk.


    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[in]  path            The path specifying the module parameter(s) to get, specify NULL to obtain all
                                subscribed data.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModGetChunk, ziAPIModNextNode
*/
ZI_EXPORT ZIResult_enum ziAPIModRead(ZIConnection conn, ZIModuleHandle handle, const char* path);

/// Make the data for the next node available for reading with ::ziAPIModGetChunk.
/** After callin ::ziAPIModRead, subscribed data (or module parameters) may now be read out on a node-by-node and
    chunk-by-chunk basis. All nodes with data available in the module can be iterated over by using ziAPIModNextNode,
    then for each node the chunks of data available are read out using ::ziAPIModGetChunk. Calling this function makes
    the data from the next node available for read.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[out] path            A string specifying the node's path whose data chunk points to.
    @param[in]  bufferSize      The length of the buffer specified as the path output parameter.
    @param[out] valueType       The ::ZIValueType_enum of the node's data.
    @param[out] chunks          The number of chunks of data available for the node.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModRead, ziAPIModGetChunk, ziAPIModEventDeallocate
*/
ZI_EXPORT ZIResult_enum ziAPIModNextNode(ZIConnection conn, ZIModuleHandle handle, char* path, uint32_t bufferSize,
                                         ZIValueType_enum* valueType, uint64_t* chunks);

/// Get the specified data chunk from the current node.
/** Data is read out node-by-node and then chunk-by-chunk. This function can be used to obtain specific data chunks
    from the current node that data is being read from. More precisely, it ppreallocates space for an event structure
    big enough to hold the node's data at the specified chunk index, updates ::ZIModuleEventPtr to point to this space
    and then copies the chunk data to this space.

    Note, before the very first call to ziAPIModGetChunk, the ::ZIModuleEventPtr should be initialized to NULL and then
    left untouched for all subsequent calls (even after calling ::ziAPIModNextNode to get data from the next node).
    This is because ziAPIModGetChunk internally manages the required space allocation for the event and then in
    subsequent calls only reallocates space when it is required. It is optimized to reduce the number of required space
    reallocations for the event.

    The ::ZIModuleEventPtr should be deallocated using ::ziAPIModEventDeallocate, otherwise the lifetime of the
    ::ZIModuleEventPtr is the same as the lifetime of the module. Indeed, the same ::ZIModuleEventPtr can be used, even
    for subsequent reads. It is also possible to work with multiple ::ZIModuleEventPtr so that some pointers can be
    kept for later processing.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[out] chunkIndex      The index of the data chunk to update the pointer to.
    @param[out] ev              The module's ::ZIModuleEventPtr that points to the currently available data chunk.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModRead, ziAPIModNextNode, ziAPIModEventDeallocate
*/
ZI_EXPORT ZIResult_enum ziAPIModGetChunk(ZIConnection conn, ZIModuleHandle handle, uint64_t chunkIndex,
                                         ZIModuleEventPtr* ev);

/// Deallocate the ::ZIModuleEventPtr being used by the module.
/** This function deallocates the ZIModuleEventPtr. Since a module event's allocated space is managed internally by
    ::ziAPIModGetChunk, when the user no longer requires the event (all data has been read out) it must be deallocated
    by the user with this function.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.
    @param[in]  ev              The ::ZIModuleEventPtr to deallocate.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModGetChunk, ziAPIModRead
*/
ZI_EXPORT ZIResult_enum ziAPIModEventDeallocate(ZIConnection conn, ZIModuleHandle handle, ZIModuleEventPtr ev);

/// Terminates the module's thread and destroys the module.
/** This function terminates the module's thread. After calling ziAPIModClear the module's handle may not be used any
    more. A new instance of the module must be initialized if required.

    @param[in]  conn            The ::ZIConnection from which the module was created.
    @param[in]  handle          The ::ZIModuleHandle specifying the module to execute.

    @return
    - ZI_INFO_SUCCESS           On success.
    - ZI_ERROR_CONNECTION       When the connection is invalid (not connected) or when a communication error occurred.
    - ZI_ERROR_GENERAL          If a general error occurred, use ::ziAPIGetLastError for a detailed error message.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIModExecute, ziAPIModFinish
*/
ZI_EXPORT ZIResult_enum ziAPIModClear(ZIConnection conn, ZIModuleHandle handle);

/** @} */  // end of group modules

/** @defgroup VectorWrite Vector Write
 *  @brief Functions for working with vector data writing.
 *  @{
 */

enum ZIVectorWriteStatus_enum {
  ZI_VECTOR_WRITE_STATUS_IDLE = 0,
  ZI_VECTOR_WRITE_STATUS_PENDING = 1
};

#ifndef __cplusplus
typedef enum ZIVectorWriteStatus_enum ZIVectorWriteStatus_enum;
#endif

ZI_EXPORT ZIResult_enum ziAPIVectorWriteBlock(ZIConnection conn, const char* path, ZIVectorData* vectorBlock);

/// status - see ::ZIVectorWriteStatus_enum
ZI_EXPORT ZIResult_enum ziAPIVectorWriteGetStatus(ZIConnection conn, const char* path, uint8_t* status);

/// vectorElementType - see ::ZIVectorElementType_enum
ZI_EXPORT ZIResult_enum ziAPIVectorWrite(ZIConnection conn, const char* path, const void* vectorPtr,
                                         uint8_t vectorElementType, uint64_t vectorSizeElements);

/** @} */  // end of group VectorWrite


/** @defgroup Discovery Device discovery
 *  @brief Functions for working with device Discovery.
 *  @{
 */

/** Perform a Discovery property look-up for the specified deviceAddress and return its device ID. Attention! This
    invalidates all pointers previously returned by ziAPIDiscovery* calls. The deviceId need not be deallocated by the
    user.

    @param[in]  conn          Pointer to ::ZIConnection with which the value should be retrieved.
    @param[out] deviceIds     Pointer to a buffer that is to contain the list of newline-separated
                              IDs of the devices found, e.g. "DEV2006\nDEV2007\n".
    @param[in]  bufferSize    The size of the buffer pointed to by deviceIds. If the buffer is too small
                              to hold the complete list of device IDs, its contents remain unchanged.

    @return
    - ZI_INFO_SUCCESS
    - ZI_ERROR_LENGTH         The provided buffer is too small to hold the list.
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDiscoveryFind, ziAPIDiscoveryGet, ziAPIDiscoveryGetValueI, ziAPIDiscoveryGetValueS
*/
ZI_EXPORT ZIResult_enum ziAPIDiscoveryFindAll(ZIConnection conn, char* deviceIds, uint32_t bufferSize);

/** Perform a Discovery property look-up for the specified deviceAddress and return its device ID. Attention! This
    invalidates all pointers previously returned by ziAPIDiscovery* calls. The deviceId need not be deallocated by the
    user.

    @param[in]  conn          Pointer to ::ZIConnection with which the value should be retrieved.
    @param[in]  deviceAddress The address or ID of the device to find, e.g., 'uhf-dev2006' or 'dev2006'.
    @param[out] deviceId      The ID of the device that was found, e.g. 'DEV2006'.

    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDiscoveryFindAll, ziAPIDiscoveryGet, ziAPIDiscoveryGetValueI, ziAPIDiscoveryGetValueS
*/
ZI_EXPORT ZIResult_enum ziAPIDiscoveryFind(ZIConnection conn, const char* deviceAddress, const char** deviceId);

/** Returns the device Discovery properties for a given device ID in JSON format. The function ::ziAPIDiscoveryFind must
    be called before ziAPIDiscoveryGet can be used. The propsJSON need not be deallocated by the user.

    @param[in]  conn          Pointer to ::ZIConnection with which the value should be retrieved.
    @param[in]  deviceId      The ID of the device to get Discovery information for, as returned by
                              ::ziAPIDiscoveryFind, e.g., 'dev2006'.
    @param[out] propsJSON     The Discovery properites in JSON format of the specified device.

    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDiscoveryFind, ziAPIDiscoveryGetValueI, ziAPIDiscoveryGetValueS
*/
ZI_EXPORT ZIResult_enum ziAPIDiscoveryGet(ZIConnection conn, const char* deviceId, const char** propsJSON);

/** Returns the specified integer Discovery property value for a given device ID. The function ::ziAPIDiscoveryFind must
    be called with the required device ID before using ziAPIDiscoveryGetValueI.

    @param[in]  conn          Pointer to ::ZIConnection with which the value should be retrieved.
    @param[in]  deviceId      The ID of the device to get Discovery information for, as returned by
                              ::ziAPIDiscoveryFind, e.g., 'dev2006'.
    @param[in]  propName      The name of the desired integer Discovery property.
    @param[out] value         Pointer to the value of the specified Discovery property.

    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDiscoveryFind, ziAPIDiscoveryGet, ziAPIDiscoveryGetValueS
*/
ZI_EXPORT ZIResult_enum ziAPIDiscoveryGetValueI(ZIConnection conn, const char* deviceId, const char* propName,
                                                ZIIntegerData* value);

/** Returns the specified string Discovery property value for a given device ID. The function ::ziAPIDiscoveryFind must
    be called with the required device ID before using ziAPIDiscoveryGetValueS. The value must not be deallocated by the
    user.

    @param[in]  conn          Pointer to ::ZIConnection with which the value should be retrieved.
    @param[in]  deviceId      The ID of the device to get Discovery information for, as returned by
                              ::ziAPIDiscoveryFind, e.g., 'dev2006'.
    @param[in]  propName      The name of the desired integer Discovery property.
    @param[out] value         Pointer to the value of the specified Discovery property.

    @return
    - ZI_INFO_SUCCESS
    - Other return codes may also be returned, for a detailed error message use ::ziAPIGetLastError.

    @sa ziAPIDiscoveryFind, ziAPIDiscoveryGet, ziAPIDiscoveryGetValueI
*/
ZI_EXPORT ZIResult_enum ziAPIDiscoveryGetValueS(ZIConnection conn, const char* deviceId, const char* propName,
                                                const char** value);

/** @} */  // end of group Discovery


typedef ZIValueType_enum DEPRECATED(ziAPIDataType);
typedef ZITimeStamp DEPRECATED(ziTimeStampType);
typedef ZIResult_enum DEPRECATED(ZI_STATUS);
typedef ZIConnection DEPRECATED(ziConnection);
typedef ZIDoubleData DEPRECATED(ziDoubleType);
typedef ZIIntegerData DEPRECATED(ziIntegerType);

/// The DemodSample struct holds data for the ZI_DATA_DEMODSAMPLE data type. Deprecated: See ::ZIDemodSample.
struct DEPRECATED(DemodSample {
  /// Time stamp at which the sample has been measured
  ziTimeStampType TimeStamp;

  /// X part of the sample
  double X;
  /// Y part of the sample
  double Y;

  /// oscillator frequency at that sample
  double Frequency;
  /// oscillator phase at that sample
  double Phase;

  /// the current bits of the DIO
  unsigned int DIOBits;

  /// reserved uint to maintain 8bytes alignment
  unsigned int Reserved;

  /// values of Aux inputs
  double AuxIn0;
  double AuxIn1;
});

#ifndef __cplusplus
typedef struct DemodSample DEPRECATED(DemodSample);
#endif

/// The AuxInSample struct holds data for the ZI_DATA_AUXINSAMPLE data type. Deprecated: See ::ZIAuxInSample.
struct DEPRECATED(AuxInSample {
  /// Time stamp at which the values have been measured
  ziTimeStampType TimeStamp;

  /// Channel 0 voltage
  double Ch0;
  /// Channel 1 voltage
  double Ch1;
});

#ifndef __cplusplus
typedef struct AuxInSample DEPRECATED(AuxInSample);
#endif

/// The DIOSample struct holds data for the ZI_DATA_DIOSAMPLE data type. Deprecated: See ::ZIDIOSample.
struct DEPRECATED(DIOSample {
  /// The timestamp at which the values have been measured.
  ziTimeStampType TimeStamp;

  /// The value of the DIO.
  unsigned int Bits;

  /// Alignment to 8 bytes.
  unsigned int Reserved;
});

#ifndef __cplusplus
typedef struct DIOSample DEPRECATED(DIOSample);
#endif

/// TREE_ACTION defines the values for the TreeChange::Action Variable
enum TREE_ACTION {
#ifdef _MSC_VER
  /// a tree has been removed
  TREE_ACTION_REMOVE = 0,

  /// a tree has been added
  TREE_ACTION_ADD = 1,

  /// a tree has changed
  TREE_ACTION_CHANGE = 2
#else
  /* empty enums are not allowed in ANSI C (gcc) */
  TREE_ACTION_REMOVE_DUMMY_FOR_ANSI_C = 0,
  TREE_ACTION_ADD_DUMMY_FOR_ANSI_C = 1,
  TREE_ACTION_CHANGE_DUMMY_FOR_ANSI_C = 2
#endif
};

#ifndef __cplusplus
typedef enum TREE_ACTION TREE_ACTION;
#endif

#ifdef _MSC_VER
#pragma deprecated(TREE_ACTION_REMOVE)
#pragma deprecated(TREE_ACTION_ADD)
#pragma deprecated(TREE_ACTION_CHANGE)
#else

#ifdef __cplusplus
#define TREE_ACTION_cast(val) TREE_ACTION(val)
#else
#define TREE_ACTION_cast(val) val
#endif

DEPRECATED_ENUM(const TREE_ACTION TREE_ACTION_REMOVE   = TREE_ACTION_cast(0));
DEPRECATED_ENUM(const TREE_ACTION TREE_ACTION_ADD      = TREE_ACTION_cast(1));
DEPRECATED_ENUM(const TREE_ACTION TREE_ACTION_CHANGE   = TREE_ACTION_cast(2));
#endif

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(push)
#pragma warning(disable:4200)
#endif

/// The ByteArrayData struct holds data for the ZI_DATA_BYTEARRAY data type. Deprecated: See ::ZIByteArray.
struct DEPRECATED(ByteArrayData {
  /// length of the data readable from the Bytes field
  unsigned int Len;
  /// the data itself. The array has the size given in Len
  unsigned char Bytes[0];
});

#ifndef __cplusplus
typedef struct ByteArrayData DEPRECATED(ByteArrayData);
#endif

#ifdef _MSC_VER
// Visual C++ specific
#pragma warning(pop)
#endif

/// This struct holds event data forwarded by the Data Server. Deprecated: See ::ZIEvent.
/**
 *  @ingroup DataHandling
    ziEvent is used to give out events like value changes or errors to the user. Event handling functionality is
    provided by ziAPISubscribe and ziAPIUnSubscribe as well as ziAPIPollDataEx.

    @include ExampleProcessEvent.c
    @sa ziAPISubscribe, ziAPIUnSubscribe, ziAPIPollDataEx
 */
struct DEPRECATED(ziEvent {
  /// Specifies the type of the data held by the ziEvent
  uint32_t Type;

  /// Number of values available in this event
  uint32_t Count;

  /// The path to the node from which the event originates
  unsigned char Path[MAX_PATH_LEN];

  /// Convenience pointer to allow for access to the first entry in Data using the correct type
  /// according to ziEvent::Type field.
  union Val {
    /// For convenience. The void field doesn't have a corresponding data type.
    void* Void;

    /// Data of type ZI_DATA_DEMODSAMPLE
    DemodSample* SampleDemod;

    /// Data of type ZI_DATA_AUXINSAMPLE
    AuxInSample* SampleAuxIn;

    /// Data of type ZI_DATA_DIOSAMPLE
    DIOSample* SampleDIO;

    /// Data of type ZI_DATA_DOUBLE
    ziDoubleType* Double;

    /// Data of type ZI_DATA_INTEGER
    ziIntegerType* Integer;

    /// Data of type ZI_DATA_TREE_CHANGED
    TreeChange* Tree;

    /// Data of type ZI_DATA_BYTEARRAY
    ByteArrayData* ByteArray;

    /// Data of type ZI_DATA_SCOPEWAVE
    ScopeWave* Wave;

    /// ensure union size is 8 bytes
    uint64_t alignment;
  } Val;

  /// The raw value data
  unsigned char Data[ MAX_EVENT_SIZE ];
});

#ifndef __cplusplus
typedef struct ziEvent DEPRECATED(ziEvent);
#endif

/// Deprecated: See ::ziAPIAllocateEventEx().
DEPRECATED(inline ziEvent* ziAPIAllocateEvent());
inline ziEvent* ziAPIAllocateEvent() {
  return (ziEvent*)ziAPIAllocateEventEx();
}

/// Deprecated: See ::ziAPIDeallocateEventEx().
DEPRECATED(inline void ziAPIDeallocateEvent(ziEvent* ev));
inline void ziAPIDeallocateEvent(ziEvent* ev) {
  ziAPIDeallocateEventEx((ZIEvent*)ev);
}


/// Checks if an event is available to read. Deprecated: See ::ziAPIPollDataEx().
/**
 *  @deprecated
    @ingroup DataHandling
    This function returns immediately if an event is pending. Otherwise it waits for an event for up to TimeOut
    milliseconds. All value changes that occur in nodes that have been subscribed to or in children of nodes that
    have been subscribed to are sent from the Data Server to the ziAPI session. For a description of how the data
    are available in the struct, refer the documentation of struct ziEvent. When no event was available within
    TimeOut milliseconds, the ziEvent::Type field will be ZI_DATA_NONE and the ziEvent::Count field will be zero.
    Otherwise these fields hold the values corresponding to the event occurred.

    @param[in]  conn     Pointer to the ::ZIConnection for which events should be received
    @param[out] ev       Pointer to a ::ziEvent struct in which the received event will be written
    @param[in]  timeOut  Time to wait for an event in milliseconds. If -1 it will wait forever, if 0 the function
                          returns immediately.
    @return
     - ZI_SUCCESS     On success.
     - ZI_CONNECTION  When the connection is invalid (not connected) or when a communication error occurred.
     - ZI_OVERFLOW    When a FIFO overflow occurred.

    See @link DataHandling Data Handling @endlink for an example

    @sa ziAPISubscribe, ziAPIUnSubscribe, ziAPIGetValueAsPollData, ziEvent
 */
DEPRECATED(inline ZIResult_enum ziAPIPollData(ZIConnection conn, ziEvent* ev, int timeOut));
inline ZIResult_enum ziAPIPollData(ZIConnection conn, ziEvent* ev, int timeOut) {
  return ziAPIPollDataEx(conn, (ZIEvent*)ev, timeOut);
}

/// @deprecated
/// @ingroup Parameters
/// Deprecated: See ::ziAPIGetDemodSample().
DEPRECATED(inline ZIResult_enum ziAPIGetValueS(ZIConnection conn, char* path, DemodSample* value));
inline ZIResult_enum ziAPIGetValueS(ZIConnection conn, char* path, DemodSample* value) {
  return ziAPIGetDemodSample(conn, path, (ZIDemodSample*)value);
}

/// @deprecated
/// @ingroup Parameters
/// Deprecated: See ::ziAPIGetDIOSample().
DEPRECATED(inline ZIResult_enum ziAPIGetValueDIO(ZIConnection conn, char* path, DIOSample* value));
inline ZIResult_enum ziAPIGetValueDIO(ZIConnection conn, char* path, DIOSample* value) {
  return ziAPIGetDIOSample(conn, path, (ZIDIOSample*)value);
}

/// @deprecated
/// @ingroup Parameters
/// Deprecated: See ::ziAPIGetAuxInSample().
DEPRECATED(inline ZIResult_enum ziAPIGetValueAuxIn(ZIConnection conn, char* path, AuxInSample* value));
inline ZIResult_enum ziAPIGetValueAuxIn(ZIConnection conn, char* path, AuxInSample* value) {
  return ziAPIGetAuxInSample(conn, path, (ZIAuxInSample*)value);
}

/// @deprecated
/// Converts a ziTimeStampType into a double-type representing seconds.
/**
     Deprecated: timestamps should instead be converted to seconds by dividing by the instrument's
     "clockbase". This is available as an leaf under the instrument's root "device" branch in the node
     hierarchy, e.g., /dev2001/clockbase.

     @param[in] TS the timestamp to convert to seconds
     @return The timestamp in seconds as a double
*/
DEPRECATED(double ZI_EXPORT ziAPISecondsTimeStamp(ziTimeStampType TS));

#define ZI_ERROR(dummy, result) return result;

#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 6
// Only supported by GCC >= 4.6
#pragma GCC diagnostic pop
#else
// There is no way for GCC < 4.6 to restore the previously set diagnostic level (push/pop is not supported),
// therefore we just set it to the default "warning" level. Unwanted side effect - it will be set to warning
// for the remainder of any file that includes ziAPI.h, even if it was changed via command line or with another
// pragma.
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#elif defined(_MSC_VER)
// Visual C++ specific
// restore 4996 - declared deprecated
#pragma warning(pop)
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __ZIAPI_H__
