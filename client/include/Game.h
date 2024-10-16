#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

class Event;

class Game{
private:
    std::string gameName;
    std::unordered_map <std::string, std::vector<std::string>> userToReports;
    
public:
    std::map <std::string,std::string> generalUpdates;
    std::map <std::string,std::string> teamA_updates;
    std::map <std::string,std::string> teamB_updates;

    Game(std::string username, Event &e);
    Game(std::string gameName);
    virtual ~Game();
    std::string getGameName();
    void insertEvent(std::string username, Event &event); // for report action and for MESSAGE from server
    void summary(std::string username,std::string &output);
    std::string mapToString(const std::map<std::string,std::string>& myMap);
};

