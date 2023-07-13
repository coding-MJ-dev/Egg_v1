#include <string>
#include <sd/FileLogger.hpp>
#include <event_logger.hpp>

bool FileLogger::initialise() {
  if (!SD.begin(CS_PIN)) {
    eventLogger.logEvent(F("ERROR\tSD begin failed"));

    this->initialised = false;
    return this->initialised;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    eventLogger.logEvent(F("ERROR\tNo SD card attached"));

    this->initialised = false;
    return this->initialised;
  } else if (cardType == CARD_MMC) {
    eventLogger.logEvent(F("INFO\tSD Card type: MMC"));
  } else if (cardType == CARD_SD) {
    eventLogger.logEvent(F("INFO\tSD Card type: SD"));
  } else if (cardType == CARD_SDHC) {
    eventLogger.logEvent(F("INFO\tSD Card type: SDHC"));
  } else {
    eventLogger.logEvent(F("INFO\tSD Card type: Unknown"));
  }

  // Log card characteristics
  char cardInfoBuffer[128];
  const uint64_t cardCapacity = SD.cardSize() / (1024 * 1024);
  const uint64_t cardFreeSpace = (SD.cardSize() - SD.usedBytes()) / (1024 * 1024);
  sprintf(cardInfoBuffer, "INFO\tSD card capacity: %lluMB, free space: %lluMB", cardCapacity, cardFreeSpace);
  eventLogger.logEvent(cardInfoBuffer);

  this->initialised = true;
  return this->initialised;
}

bool FileLogger::createFile(const char * path) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    eventLogger.logEvent(F("ERROR\tFailed to open file for writing"));
    return false;
  }

  file.close();
  return true;
}

File FileLogger::openFileForWrite(const char * path) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    eventLogger.logEvent(F("ERROR\tFailed to open file for writing"));
  }

  return file;
}

bool FileLogger::fileExists(const char * path) const {
  return SD.exists(path);
}

bool FileLogger::writeToFile(const char * path, std::string message) {
  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    eventLogger.logEvent(F("ERROR\tFailed to open file for appending"));
    return false;
  }

  if (!file.println(message.c_str())) {
    eventLogger.logEvent(F("ERROR\tFailed to write to file"));
    file.close();
    return false;
  }

  file.close();
  return true;
}

bool FileLogger::writeToFile(const char * path, const ArduinoJson::DynamicJsonDocument &payload) {
  if (payload.isNull()) {
    eventLogger.logEvent(F("ERROR\tPayload is null"));
    return false;
  }

  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    eventLogger.logEvent(F("ERROR\tFailed to open file for appending"));
    return false;
  }

  // Write payload as a JSON string
  serializeJson(payload, file);

  // Write newline character
  file.println();

  // Flush write buffer (this ensures the data is actually written onto the SD card)
  file.flush();

  // Close file handle (prevents SD card filesystem corruption)
  file.close();

  return true;
}

// Writes to a file (given a file handle)
bool FileLogger::writeToFile(File &fileHandle, std::string message) {
  if (!fileHandle) {
    eventLogger.logEvent(F("ERROR\tFile is not open"));
    return false;
  }

  if (!fileHandle.println(message.c_str())) {
    eventLogger.logEvent(F("ERROR\tFailed to write to file via handle"));
    return false;
  }

  // Flush write buffer (this ensures the data is actually written onto the SD card)
  fileHandle.flush();

  return true;
}

// Writes to a file (given a file handle)
bool FileLogger::writeToFile(File &fileHandle, const ArduinoJson::DynamicJsonDocument &payload) {
  if (!fileHandle) {
    eventLogger.logEvent(F("ERROR\tFile is not open"));
    return false;
  }

  // Write payload as a JSON string
  serializeJson(payload, fileHandle);

  // Write newline character
  fileHandle.println();

  // Flush write buffer (this ensures the data is actually written onto the SD card)
  fileHandle.flush();

  return true;
}

FileLogger sdFileSystem;
