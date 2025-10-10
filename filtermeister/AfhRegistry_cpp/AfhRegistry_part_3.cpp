#ifdef VERBOSE
      if (iss != ERROR_SUCCESS) {
          ErrorOk("RegDeleteValue failed, name = \"%s\", iss = %d", lpValueName, iss);
      }
#endif
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
              ErrorOk("RegCloseKey failed, iss = %d", iss);
#endif
          }
      }
  }
  return iss;
}//RDV

/*****************************************************************************/
