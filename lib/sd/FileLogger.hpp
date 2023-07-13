#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include <string>
#include <ArduinoJson.hpp>
#include <core/Peripheral.hpp>
#include <SD.h>

class FileLogger : public virtual Peripheral {
 public:
  FileLogger() {}

  bool initialise() final;
  File openFileForWrite(const char * path);
  bool createFile(const char * path);
  bool fileExists(const char * path) const;

  // Write to file (internally opens file handle)
  bool writeToFile(const char * path, std::string message);
  bool writeToFile(const char * path, const ArduinoJson::DynamicJsonDocument &payload);

  // Write to file given a file handle
  bool writeToFile(File &fileHandle, std::string message);
  bool writeToFile(File &fileHandle, const ArduinoJson::DynamicJsonDocument &payload);

 private:
  static constexpr int CS_PIN = 14;
};

extern FileLogger sdFileSystem;

#endif
