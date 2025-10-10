#ifdef VERBOSE
              ErrorOk("RegCloseKey failed, iss = %d, value name = %s", iss2, lpValueName);
#endif
          }
      }
  }
  return iss;
}//RQD

/*****************************************************************************/

// Registry Query String

LONG RQS(HKEY hKey, LPCTSTR lpValueName, CHAR *szBuffer, DWORD cbMaxLen) {
  DWORD cbDataLocalCopy = cbMaxLen;
  LONG iss = ERROR_INVALID_FUNCTION;
  DWORD dwValueType = REG_NONE;

  // Fill data buffer with zeroes in case of error or short data...
  memset(szBuffer, 0, cbMaxLen);

  if (hKey) {
      iss = RegQueryValueEx(hKey, lpValueName, NULL, &dwValueType, (LPBYTE)szBuffer, &cbDataLocalCopy);
#if 0
      Info("Value type = %d", dwValueType);
      Info("Data size = %d", cbDataLocalCopy);
      Info("Max expected size = %d", cbMaxLen);
#endif
      if (iss == ERROR_SUCCESS) {
          switch (dwValueType) {
          case REG_SZ:
#if 0
              Info("%s = \"%s\"", lpValueName, szBuffer);
#endif
              break;
          default:
#if 0
              ErrorOk("Wrong data type!");
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
  //Make sure string is NULL-terminated if data was truncated...
  szBuffer[cbMaxLen-1] = 0;
  return iss;
}//RQS

/*****************************************************************************/

// Registry Set Binary Data

LONG RSB(HKEY hKey, LPCTSTR lpValueName, CONST VOID *dataBuffer, DWORD cbData) {
  LONG iss = ERROR_INVALID_FUNCTION;

  if (hKey) {
      iss = RegSetValueEx(hKey, lpValueName, 0, REG_BINARY, (CONST BYTE *)dataBuffer, cbData);
#ifdef VERBOSE
      if (iss != ERROR_SUCCESS) {
          ErrorOk("RegSetValueEx failed, iss = %d", iss);
      }
#endif
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
}//RSB

/*****************************************************************************/

// Registry Set Dword

LONG RSD(HKEY hKey, LPCTSTR lpValueName, DWORD dwValue) {
  LONG iss = ERROR_INVALID_FUNCTION;

  if (hKey) {
      iss = RegSetValueEx(hKey, lpValueName, 0, REG_DWORD, (CONST BYTE *)&dwValue,
                                                           sizeof(DWORD));
#ifdef VERBOSE
      if (iss != ERROR_SUCCESS) {
          ErrorOk("RegSetValueEx failed, iss = %d", iss);
      }
#endif
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
}//RSD

/*****************************************************************************/

// Registry Set String

LONG RSS(HKEY hKey, LPCTSTR lpValueName, LPCTSTR szBuffer) {
  LONG iss = ERROR_INVALID_FUNCTION;

  if (hKey) {
      iss = RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (CONST BYTE *)szBuffer,
                                                        (DWORD)strlen(szBuffer) + 1); //(DWORD) added for 64bit compatibility
#ifdef VERBOSE
      if (iss != ERROR_SUCCESS) {
          ErrorOk("RegSetValueEx failed, iss = %d", iss);
      }
#endif
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
}//RSS

/*****************************************************************************/

// Registry Enumerate Key

LONG REK(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cbName) {
  LONG iss = ERROR_INVALID_FUNCTION;

  // Fill name buffer with zeroes in case of error...
  memset(lpName, 0, cbName);

  if (hKey) {
      iss = RegEnumKey(hKey, dwIndex, lpName, cbName);
      if (iss == ERROR_SUCCESS) {
#if 0
          Info("Key index = %d", dwIndex);
          Info("Key name = %s", lpName);
#endif
      }
#ifdef VERBOSE
      else {
          ErrorOk("RegEnumKey failed, iss = %d, index = %d", iss, dwIndex);
      }
#endif
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
              ErrorOk("RegCloseKey failed, iss = %d", iss);
#endif
          }
      }
  }
  return iss;
}//REK

/*****************************************************************************/

// Registry Enumerate Value

LONG REV(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcbValueName,
         LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
  LONG iss = ERROR_INVALID_FUNCTION;

  // Fill name buffer with zeroes in case of error...
  memset(lpValueName, 0, *lpcbValueName);

  if (hKey) {
      iss = RegEnumValue(hKey, dwIndex, lpValueName, lpcbValueName, NULL,
                         lpType, lpData, lpcbData);
      if (iss == ERROR_SUCCESS) {
#if 0
          Info("Value index = %d", dwIndex);
          Info("Value name = %s", lpValueName);
          if (lpType) Info("Data type = %d", *lpType);
          if (lpcbData) Info("Data size = %d", *lpcbData);
#endif
      }
#ifdef VERBOSE
      else {
          ErrorOk("RegEnumValue failed, iss = %d, index = %d", iss, dwIndex);
      }
#endif
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
              ErrorOk("RegCloseKey failed, iss = %d", iss);
#endif
          }
      }
  }
  return iss;
}//REV

/*****************************************************************************/

// Registry Delete Key

LONG RDK(HKEY hKey, LPCTSTR lpSubKey) {
  LONG iss = ERROR_INVALID_FUNCTION;

  if (hKey) {
      iss = RegDeleteKey(hKey, lpSubKey);
#ifdef VERBOSE
      if (iss != ERROR_SUCCESS) {
          ErrorOk("RegDeleteKey failed, key = \"%s\", iss = %d", lpSubKey, iss);
      }
#endif
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
              ErrorOk("RegCloseKey failed, iss = %d", iss);
#endif
          }
      }
  }
  return iss;
}//RDK

/*****************************************************************************/

// Registry Delete Value

LONG RDV(HKEY hKey, LPCTSTR lpValueName) {
  LONG iss = ERROR_INVALID_FUNCTION;

  if (hKey) {
      iss = RegDeleteValue(hKey, lpValueName);