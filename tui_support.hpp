#ifndef TUI_SUPPORT_HPP
#define TUI_SUPPORT_HPP

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "accessman.hpp"
#include "json.hpp"

namespace tui_support {

using std::string;
using std::vector;
using nlohmann::json;

struct ConfigPaths {
  std::filesystem::path ndiDir;
  std::filesystem::path configPath;
  std::filesystem::path backupConfigPath;
};

inline ConfigPaths getConfigPaths() {
  const std::filesystem::path ndiDir = std::filesystem::path(getHomeDir()) / ".ndi";
  const std::filesystem::path configPath = ndiDir / "ndi-config.v1.json";
  const std::filesystem::path backupConfigPath = configPath.string() + ".bak";
  return {ndiDir, configPath, backupConfigPath};
}

inline bool ensureConfigDirectory(const std::filesystem::path& ndiDir) {
  std::error_code fsError;
  const bool ndiDirExists = std::filesystem::exists(ndiDir, fsError);
  if (fsError) {
    std::cerr << "Could not check NDI config directory: " << ndiDir << " (" << fsError.message() << ")" << std::endl;
    return false;
  }

  if (!ndiDirExists) {
    std::filesystem::create_directories(ndiDir, fsError);
    if (fsError) {
      std::cerr << "Could not create NDI config directory: " << ndiDir << " (" << fsError.message() << ")" << std::endl;
      return false;
    }
    std::cout << ".ndi directory does not exist. Creating." << std::endl;
  }

  return true;
}

inline json loadConfigWithDefaults(const std::filesystem::path& configPath) {
  json ndiConfig = json::object();

  std::ifstream inputFile(configPath);
  if (inputFile.is_open()) {
    try {
      inputFile >> ndiConfig;
    } catch (const json::exception& e) {
      std::cerr << "Invalid NDI Config JSON at " << configPath << ": " << e.what() << std::endl;
      std::cout << "Using default config values for this run." << std::endl;
      ndiConfig = json::object();
    }
    inputFile.close();
  } else {
    std::cout << "NDI config file does not exist yet. A new ndi-config.v1.json will be created." << std::endl;
  }

  try {
    generateMissingConfig(ndiConfig);
  } catch (const json::exception& e) {
    std::cerr << "Could not normalize NDI config, resetting to defaults: " << e.what() << std::endl;
    ndiConfig = json::object();
    generateMissingConfig(ndiConfig);
  }

  return ndiConfig;
}

inline bool loadBackupConfig(const std::filesystem::path& backupConfigPath, json& backupConfig, string& errorMessage) {
  if (!std::filesystem::exists(backupConfigPath)) {
    errorMessage = "No backup file found";
    return false;
  }

  std::ifstream backupInputFile(backupConfigPath);
  if (!backupInputFile.is_open()) {
    errorMessage = "Could not open backup file";
    return false;
  }

  try {
    backupInputFile >> backupConfig;
    generateMissingConfig(backupConfig);
  } catch (const json::exception&) {
    errorMessage = "Backup file is invalid JSON";
    return false;
  }

  return true;
}

inline bool saveConfigAtomicallyWithBackup(
  const std::filesystem::path& configPath,
  const std::filesystem::path& backupConfigPath,
  const json& ndiConfig,
  string& errorMessage
 ) {
  const std::filesystem::path tempConfigPath = configPath.string() + ".tmp";

  std::ofstream outputFile(tempConfigPath);
  if (!outputFile.is_open()) {
    errorMessage = "Could not open temp NDI config JSON: " + tempConfigPath.string();
    return false;
  }

  outputFile << ndiConfig.dump(2);
  outputFile.flush();
  if (!outputFile.good()) {
    errorMessage = "Failed to write temp NDI config JSON: " + tempConfigPath.string();
    outputFile.close();
    std::error_code cleanupError;
    std::filesystem::remove(tempConfigPath, cleanupError);
    return false;
  }
  outputFile.close();

  if (std::filesystem::exists(configPath)) {
    std::error_code backupError;
    std::filesystem::copy_file(
      configPath,
      backupConfigPath,
      std::filesystem::copy_options::overwrite_existing,
      backupError
    );
    if (backupError) {
      std::cerr << "Warning: could not create NDI config backup at " << backupConfigPath
                << ": " << backupError.message() << std::endl;
    }
  }

  std::error_code renameError;
  std::filesystem::rename(tempConfigPath, configPath, renameError);
  if (renameError) {
    errorMessage = "Could not atomically replace NDI config JSON: " + renameError.message();
    std::error_code cleanupError;
    std::filesystem::remove(tempConfigPath, cleanupError);
    return false;
  }

  return true;
}

inline string trim(const string& value) {
  const string whitespace = " \t\n\r";
  const size_t start = value.find_first_not_of(whitespace);
  if (start == string::npos) {
    return "";
  }
  const size_t end = value.find_last_not_of(whitespace);
  return value.substr(start, end - start + 1);
}

inline vector<string> splitCsv(const string& value) {
  vector<string> tokens;
  std::stringstream ss(value);
  string token;
  while (std::getline(ss, token, ',')) {
    token = trim(token);
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

inline bool isValidIPv4(const string& value) {
  std::stringstream ss(value);
  string part;
  int count = 0;
  while (std::getline(ss, part, '.')) {
    if (part.empty() || part.size() > 3) {
      return false;
    }
    for (char c : part) {
      if (!std::isdigit(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    int octet = std::stoi(part);
    if (octet < 0 || octet > 255) {
      return false;
    }
    ++count;
  }
  return count == 4;
}

inline bool isValidNetmask(const string& value) {
  if (!isValidIPv4(value)) {
    return false;
  }

  std::stringstream ss(value);
  string part;
  uint32_t mask = 0;
  while (std::getline(ss, part, '.')) {
    mask = (mask << 8) | static_cast<uint32_t>(std::stoi(part));
  }

  bool seenZero = false;
  for (int bit = 31; bit >= 0; --bit) {
    const bool set = ((mask >> bit) & 1U) != 0;
    if (!set) {
      seenZero = true;
      continue;
    }
    if (seenZero) {
      return false;
    }
  }
  return true;
}

inline bool isValidMulticastPrefix(const string& value) {
  if (!isValidIPv4(value)) {
    return false;
  }

  std::stringstream ss(value);
  string first;
  std::getline(ss, first, '.');
  int firstOctet = std::stoi(first);
  return firstOctet >= 224 && firstOctet <= 239;
}

inline bool validateCsvIPv4(const string& value) {
  for (const string& token : splitCsv(value)) {
    if (!isValidIPv4(token)) {
      return false;
    }
  }
  return true;
}

}  // namespace tui_support

#endif
