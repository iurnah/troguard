#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <cstring>
#include <vector>

class FileEntry {
public:
  FileEntry(const char *name, bool isDirectory) {
    this->name = new char[strlen(name)];
    strcpy(this->name, name);
    this->isDirectory = isDirectory;
  }
  
  ~FileEntry() {
    delete[] name;
  }

  char *name;
  bool isDirectory;
};

bool fileExists(const char *name);
bool isDirectory(const char *name);
bool getFile(const char *filename, char *&value, size_t &len, const bool issBinary);
bool createDirectory(const char *filename);
bool removeFile(const char *filename);
bool saveText(const char *filename, const char *value, size_t len);
bool saveBinaryFile(const char *filename, const char *bytes, const size_t len);
bool getTempPath(char *&value, size_t &len);
bool listFiles(const char *normalisedDirectoryName, std::vector<FileEntry *> *&files);

#endif //FILE_IO_H_
