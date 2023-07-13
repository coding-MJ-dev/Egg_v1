#ifndef FILE_HANDLES
#define FILE_HANDLES

#include <SD.h>
#include <sd/FileLogger.hpp>

File eventsFile;
File telemetryFile;

bool setupFile(File &fileHandle, const char * path) {
  if (sdFileSystem.fileExists(path)) {
    fileHandle = sdFileSystem.openFileForWrite(path);
    return true;
  }

  const bool created = sdFileSystem.createFile(path);
  if (!created) {
    return false;
  }

  fileHandle = sdFileSystem.openFileForWrite(path);
  return true;
}

#endif
