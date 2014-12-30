#include "drive-by-download.h"
#include "third_party/build/build_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <string>

#if defined(OS_LINUX)
const mode_t DEFAULT_FOLDER_PERMISSIONS = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP;
#define PLATFORM_PATH_SEPARATOR_CHAR '/'
#define PLATFORM_PATH_SEPARATOR_STRING "/"
#elif defined(OS_WIN)
#define PLATFORM_PATH_SEPARATOR_CHAR '\\'
#define PLATFORM_PATH_SEPARATOR_STRING "\\"
#endif

bool startFirefox(char *&value, size_t &len) {
	system("firefox &");
  return true;
}

