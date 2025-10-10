///////////////////////////////////////////////////////////////////////////////////
//  File: AfhRegistry.c
//
//  Source file for the FilterMeisterVS Windows Registry utility routines.
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/

#ifdef _DEBUG
// Turn on VERBOSE when debugging this module...
//////#define VERBOSE 1
#endif

#include "AfhFM.h"

/*****************************************************************************/

HKEY ROK(HKEY hKey, LPCTSTR lpSubKeyName) {
    LONG iss;
    HKEY hSubKey = NULL;

    if (hKey) {
        iss = RegOpenKeyEx(hKey, lpSubKeyName, 0, KEY_READ, &hSubKey);
        if (iss != ERROR_SUCCESS) {
#ifdef VERBOSE
            ErrorOk("Could not open subkey \"%s\"\n"
                   "iss = %d", lpSubKeyName, iss);
#endif
            hSubKey = NULL;
        }
#ifdef _WIN64
        if ((size_t)hKey < 0x80000000) {
#else
        if (((DWORD)hKey & 0x80000000) == 0) {
#endif
            // Not a reserved key.
            iss = RegCloseKey(hKey);
#ifdef VERBOSE
            if (iss != ERROR_SUCCESS) {
                ErrorOk("RegCloseKey failed, iss = %d, subkey = %s", iss, lpSubKeyName);
            }
#endif
        }
    }//if
    return hSubKey;
}//ROK

/*****************************************************************************/

HKEY RCK(HKEY hKey, LPCTSTR lpSubKeyName) {
    LONG iss;
    HKEY hSubKey = NULL;
    DWORD dwDisposition;

    if (hKey) {
        iss = RegCreateKeyEx(hKey, lpSubKeyName, 0, "",
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                             NULL, &hSubKey, &dwDisposition);
        if (iss != ERROR_SUCCESS) {
#ifdef VERBOSE
            ErrorOk("Could not create subkey \"%s\"\n"
                   "iss = %d", lpSubKeyName, iss);
#endif
            hSubKey = NULL;
        }
#ifdef _WIN64
        if ((size_t)hKey < 0x80000000) {
#else
        if (((DWORD)hKey & 0x80000000) == 0) {
#endif
            // Not a reserved key.
            iss = RegCloseKey(hKey);
#ifdef VERBOSE
            if (iss != ERROR_SUCCESS) {
                ErrorOk("RegCloseKey failed, iss = %d, subkey = %s", iss, lpSubKeyName);
            }
#endif
        }
    }//if
    return hSubKey;
}//RCK

/*****************************************************************************/

#ifdef FULL

// Registry Query Value

LONG RQV(HKEY hKey, LPCTSTR lpValueName, LPBYTE dataBuffer, DWORD cbData) {
  DWORD cbDataLocalCopy = cbData;
  LONG iss = ERROR_INVALID_FUNCTION;
  DWORD dwValueType = REG_NONE;

  // Fill data buffer with zeroes in case of error or short data...
  memset(dataBuffer, 0, cbData);

  if (hKey) {
      iss = RegQueryValueEx(hKey, lpValueName, NULL, &dwValueType, dataBuffer, &cbDataLocalCopy);
#if 0
      Info("Value type = %d", dwValueType);
      Info("Data size = %d", cbDataLocalCopy);
      Info("Expected size = %d", cbData);
#endif
      if (iss == ERROR_SUCCESS) {
          switch (dwValueType) {
          case REG_SZ:
#if 0
              Info("%s = \"%s\"", lpValueName, dataBuffer);
#endif
              break;
          case REG_DWORD:
              if (cbDataLocalCopy != sizeof(DWORD) ||
                  cbData != sizeof(DWORD)) {
                  //wrong data length
                  iss = ERROR_INVALID_DATA;
              }
#if 0
              Info("%s = 0x%8.8x", lpValueName, *(DWORD *)dataBuffer);
#endif
              break;
          case REG_BINARY:
#if 0
              Info("%s = 0x%8.8x...", lpValueName, *(DWORD *)dataBuffer);
#endif
              break;
          default:
#if 0
              ErrorOk("Wrong data type!");
#endif
              break;
          }//switch
      }
      else {
#ifdef VERBOSE
          ErrorOk("RegQueryValueEx failed, iss = %d, value name = \"%s\"", iss, lpValueName);
#endif
      }
#ifdef _WIN64
        if ((DWORD_PTR)hKey < 0x80000000) {
#else
        if (((DWORD)hKey & 0x80000000) == 0) {
#endif
          // Not a reserved key.
          LONG iss2;
          iss2 = RegCloseKey(hKey);
          if (iss2 != ERROR_SUCCESS) {
              iss = iss2;
#ifdef VERBOSE
              ErrorOk("RegCloseKey failed, iss = %d, value name = %s", iss2, lpValueName);
#endif
          }
      }
  }
  if (iss != ERROR_SUCCESS && dwValueType == REG_SZ) {
      //return a default string value
      strncpy(dataBuffer, "(undefined)", cbData);
  }
  return iss;
}//RQV

#endif //FULL

/*****************************************************************************/

// Registry Query Binary data

LONG RQB(HKEY hKey, LPCTSTR lpValueName, VOID *dataBuffer, DWORD cbData) {
  DWORD cbDataLocalCopy = cbData;
  LONG iss = ERROR_INVALID_FUNCTION;
  DWORD dwValueType = REG_NONE;

  // Fill data buffer with zeroes in case of error or short data...
  memset(dataBuffer, 0, cbData);

  if (hKey) {
      iss = RegQueryValueEx(hKey, lpValueName, NULL, &dwValueType, (LPBYTE)dataBuffer, &cbDataLocalCopy);
#if 0
      Info("Value type = %d", dwValueType);
      Info("Data size = %d", cbDataLocalCopy);
      Info("Expected size = %d", cbData);
#endif
      if (iss == ERROR_SUCCESS) {
          switch (dwValueType) {
#if 0
          case REG_DWORD:
              // DWORD data is acceptable as binary data???
              if (cbDataLocalCopy != sizeof(DWORD) ||
                  cbData != sizeof(DWORD)) {
                  //wrong data length
                  iss = ERROR_INVALID_DATA;
              }
#if 0
              Info("%s = 0x%8.8x", lpValueName, *(DWORD *)dataBuffer);
#endif
              break;
#endif
          case REG_BINARY:
#if 0
              Info("%s = 0x%8.8x...", lpValueName, *(DWORD *)dataBuffer);
#endif
              if (cbDataLocalCopy != cbData) {
                  //wrong data length
                  iss = ERROR_INVALID_DATA;
              }
              break;
          default:
#if 0
              ErrorOk("Wrong data type (%d)!", dwValueType);
#endif
              iss = ERROR_INVALID_DATA;
              break;
          }//switch
      }
      else {
#ifdef VERBOSE
          ErrorOk("RegQueryValueEx failed, iss = %d, value name = \"%s\"", iss, lpValueName);
#endif
      }
#ifdef _WIN64
        if ((size_t)hKey < 0x80000000) {
#else
        if (((DWORD)hKey & 0x80000000) == 0) {
#endif
          // Not a reserved key.
          LONG iss2;
          iss2 = RegCloseKey(hKey);
          if (iss2 != ERROR_SUCCESS) {
              iss = iss2;
#ifdef VERBOSE
              ErrorOk("RegCloseKey failed, iss = %d, value name = %s", iss2, lpValueName);
#endif
          }
      }
  }
  return iss;
}//RQB

/*****************************************************************************/

// Registry Query DWORD Value

LONG RQD(HKEY hKey, LPCTSTR lpValueName, DWORD *pdwValue) {
  DWORD cbDataLocalCopy = sizeof(DWORD);
  LONG iss = ERROR_INVALID_FUNCTION;
  DWORD dwValueType = REG_NONE;

  *pdwValue = 0;    //in case of error

  if (hKey) {
      iss = RegQueryValueEx(hKey, lpValueName, NULL, &dwValueType, (LPBYTE)pdwValue, &cbDataLocalCopy);
#if 0
      Info("Value type = %d", dwValueType);
      Info("Data size = %d", cbDataLocalCopy);
#endif
      if (iss == ERROR_SUCCESS) {
          switch (dwValueType) {
#if 0
          case REG_BINARY:  //4-byte binary data acceptable for DWORD???
#endif
          case REG_DWORD:
              if (cbDataLocalCopy != sizeof(DWORD)) {
                  //wrong data length
                  iss = ERROR_INVALID_DATA;
              }
#if 0
              Info("%s = 0x%8.8x", lpValueName, *pdwValue);
#endif
              break;
          default:
#if 0
              ErrorOk("Wrong data type (%d)!", dwValueType);
#endif
              iss = ERROR_INVALID_DATA;
              break;
          }//switch
      }
      else {
#ifdef VERBOSE
          ErrorOk("RegQueryValueEx failed, iss = %d, value name = \"%s\"", iss, lpValueName);
#endif
      }
#ifdef _WIN64
        if ((size_t)hKey < 0x80000000) {
#else
        if (((DWORD)hKey & 0x80000000) == 0) {
#endif
          // Not a reserved key.
          LONG iss2;
          iss2 = RegCloseKey(hKey);
          if (iss2 != ERROR_SUCCESS) {
              iss = iss2;