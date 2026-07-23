
#ifndef ACCESSMAN_HPP
#define ACCESSMAN_HPP

#include <cstdlib>
#include <filesystem>
#include <string>
#include "json.hpp"

inline std::string getHomeDir() {
	const char* home = std::getenv("HOME");
	return home ? std::string(home) : std::string("");
}

inline void multicastGenConfig(nlohmann::json& ndiConfig) {
	ndiConfig["ndi"]["multicast"] = {
		{"recv", {{"enable", false}}},
		{"send", {
			{"enable", false},
			{"netmask", "255.255.0.0"},
			{"netprefix", "239.255.0.0"},
			{"ttl", 1}
		}}
	};
}

inline bool configExists(const std::string& filePath) {
	return std::filesystem::exists(filePath);
}

inline void machineNameSet(std::string name, nlohmann::json& ndiConfig) {
	ndiConfig["ndi"]["machinename"] = name;
}

inline void tcpSet(bool send, bool recv, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("tcp")) {
		ndiConfig["ndi"]["tcp"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["tcp"].contains("send")) {
		ndiConfig["ndi"]["tcp"]["send"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["tcp"]["send"]["enable"] = send;

	if (!ndiConfig["ndi"]["tcp"].contains("recv")) {
		ndiConfig["ndi"]["tcp"]["recv"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["tcp"]["recv"]["enable"] = recv;
}

inline void rudpSet(bool send, bool recv, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("rudp")) {
		ndiConfig["ndi"]["rudp"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["rudp"].contains("send")) {
		ndiConfig["ndi"]["rudp"]["send"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["rudp"]["send"]["enable"] = send;

	if (!ndiConfig["ndi"]["rudp"].contains("recv")) {
		ndiConfig["ndi"]["rudp"]["recv"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["rudp"]["recv"]["enable"] = recv;
}

inline void unicastSet(bool send, bool recv, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("unicast")) {
		ndiConfig["ndi"]["unicast"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["unicast"].contains("send")) {
		ndiConfig["ndi"]["unicast"]["send"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["unicast"]["send"]["enable"] = send;

	if (!ndiConfig["ndi"]["unicast"].contains("recv")) {
		ndiConfig["ndi"]["unicast"]["recv"] = {{"enable", false}};
	}
	ndiConfig["ndi"]["unicast"]["recv"]["enable"] = recv;
}

inline void multicastRecvSet(bool recv, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("multicast")) {
		multicastGenConfig(ndiConfig);
	}

	if (!ndiConfig["ndi"]["multicast"].contains("recv")) {
		ndiConfig["ndi"]["multicast"]["recv"] = {{"enable", false}};
	}

	ndiConfig["ndi"]["multicast"]["recv"]["enable"] = recv;
}

inline void multicastSendSet(bool send, std::string netmask, std::string netprefix, int ttl, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("multicast")) {
		multicastGenConfig(ndiConfig);
	}

	if (!ndiConfig["ndi"]["multicast"].contains("send")) {
		ndiConfig["ndi"]["multicast"]["send"] = {
			{"enable", false},
			{"netmask", "255.255.0.0"},
			{"netprefix", "239.255.0.0"},
			{"ttl", 1}
		};
	}

	ndiConfig["ndi"]["multicast"]["send"]["enable"] = send;
	ndiConfig["ndi"]["multicast"]["send"]["netmask"] = netmask;
	ndiConfig["ndi"]["multicast"]["send"]["netprefix"] = netprefix;
	ndiConfig["ndi"]["multicast"]["send"]["ttl"] = ttl;
}

inline void groupsSet(std::string sendGroups, std::string recvGroups, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("groups")) {
		ndiConfig["ndi"]["groups"] = {{"recv", "Public,"}, {"send", "Public,"}};
	}

	ndiConfig["ndi"]["groups"]["recv"] = recvGroups;
	ndiConfig["ndi"]["groups"]["send"] = sendGroups;
}

inline void discoveryServerSet(std::string discoveryServers, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("networks")) {
		ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
	}

	ndiConfig["ndi"]["networks"]["discovery"] = discoveryServers;
}

inline void discoveryIpsSet(std::string ips, nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("networks")) {
		ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
	}

	ndiConfig["ndi"]["networks"]["ips"] = ips;
}

inline void generateMissingConfig(nlohmann::json& ndiConfig) {
	if (!ndiConfig["ndi"].contains("networks")) {
		ndiConfig["ndi"]["networks"] = {{"discovery", ""}, {"ips", ""}};
	}

	if (!ndiConfig["ndi"]["networks"].contains("discovery")) {
		ndiConfig["ndi"]["networks"]["discovery"] = "";
	}

	if (!ndiConfig["ndi"]["networks"].contains("ips")) {
		ndiConfig["ndi"]["networks"]["ips"] = "";
	}

	if (!ndiConfig["ndi"].contains("groups")) {
		ndiConfig["ndi"]["groups"] = {{"recv", "Public,"}, {"send", "Public,"}};
	}

	if (!ndiConfig["ndi"]["groups"].contains("recv")) {
		ndiConfig["ndi"]["groups"]["recv"] = "Public,";
	}

	if (!ndiConfig["ndi"]["groups"].contains("send")) {
		ndiConfig["ndi"]["groups"]["send"] = "Public,";
	}

	if (!ndiConfig["ndi"].contains("machinename")) {
		ndiConfig["ndi"]["machinename"] = "";
	}

	if (!ndiConfig["ndi"].contains("multicast")) {
		multicastGenConfig(ndiConfig);
	}

	if (!ndiConfig["ndi"]["multicast"].contains("send")) {
		ndiConfig["ndi"]["multicast"]["send"] = {{"enable", false}, {"netmask", ""}, {"netprefix", ""}, {"ttl", 1}};
	}

	if (!ndiConfig["ndi"]["multicast"]["send"].contains("ttl")) {
		ndiConfig["ndi"]["multicast"]["send"]["ttl"] = 1;
	}

	if (!ndiConfig["ndi"]["multicast"]["send"].contains("netmask")) {
		ndiConfig["ndi"]["multicast"]["send"]["netmask"] = "";
	}

	if (!ndiConfig["ndi"]["multicast"]["send"].contains("netprefix")) {
		ndiConfig["ndi"]["multicast"]["send"]["netprefix"] = "";
	}

	if (!ndiConfig["ndi"]["multicast"].contains("recv")) {
		ndiConfig["ndi"]["multicast"]["recv"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"].contains("unicast")) {
		ndiConfig["ndi"]["unicast"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["unicast"].contains("send")) {
		ndiConfig["ndi"]["unicast"]["send"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"]["unicast"].contains("recv")) {
		ndiConfig["ndi"]["unicast"]["recv"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"].contains("rudp")) {
		ndiConfig["ndi"]["rudp"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["rudp"].contains("send")) {
		ndiConfig["ndi"]["rudp"]["send"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"]["rudp"].contains("recv")) {
		ndiConfig["ndi"]["rudp"]["recv"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"].contains("tcp")) {
		ndiConfig["ndi"]["tcp"] = {
			{"recv", {{"enable", false}}},
			{"send", {{"enable", false}}}
		};
	}

	if (!ndiConfig["ndi"]["tcp"].contains("send")) {
		ndiConfig["ndi"]["tcp"]["send"] = {{"enable", false}};
	}

	if (!ndiConfig["ndi"]["tcp"].contains("recv")) {
		ndiConfig["ndi"]["tcp"]["recv"] = {{"enable", false}};
	}
}

#endif
