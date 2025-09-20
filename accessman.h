
#ifndef ACCESSMAN_H
#define ACCESSMAN_H

#include <string>
#include "json.hpp"

using std::string;
using nlohmann::json;


string getHomeDir();
bool configExists(const string& filePath);
void tcpSet(bool send, bool recv, json& ndiConfig);
void rudpSet(bool send, bool recv, json& ndiConfig);

#endif
