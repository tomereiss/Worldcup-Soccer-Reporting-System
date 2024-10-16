#include "../include/Game.h"
#include "../include/event.h"
#include <iostream>
#include <sstream>
#include <vector>

Game::Game(std::string username, Event &e): gameName(e.get_topic()), userToReports(),generalUpdates(e.get_game_updates()), teamA_updates(e.get_team_a_updates()), teamB_updates(e.get_team_b_updates()){
    std::vector<std::string> events;
    events.push_back(e.toStore());
    userToReports[username] = events;
}
Game::Game(std::string gameName):gameName(gameName), userToReports(),generalUpdates(), teamA_updates(), teamB_updates(){}

Game::~Game(){}

std::string Game::getGameName(){return gameName;}

void Game::insertEvent(std::string username, Event &e){ // for report action and for MESSAGE from server
    std::unordered_map<std::string,std::vector<std::string>>::iterator it = userToReports.find(username);
    if(it != userToReports.end())
        it->second.push_back(e.toStore());
    else{
        std::vector<std::string> events;
        events.push_back(e.toStore());
        userToReports.emplace(username,events); // dtor being called ??????????????
    }

    //update game stats
    for(const auto& pair : e.get_game_updates())
        generalUpdates[pair.first] = pair.second;
    for(const auto& pair : e.get_team_a_updates())
        teamA_updates[pair.first] = pair.second;				
    for(const auto& pair : e.get_team_b_updates())
        teamB_updates[pair.first] = pair.second;
}

void Game::summary(std::string username, std::string &output){
    std::unordered_map<std::string,std::vector<std::string>>::iterator it = userToReports.find(username);

    if(it != userToReports.end()){
        auto pos = gameName.find("_"); // locating ':'
        std::string teamA = gameName.substr(0, pos); // Parsing teamA
        std::string teamB = gameName.substr(pos + 1); // Parsing teamB
        output = teamA + " vs " + teamB + "\nGame stats:\nGeneral stats:\n"+ mapToString(generalUpdates) 
                            + teamA +  " stats:" + "\n" + mapToString(teamA_updates) 
                            + teamB +  " stats:" + "\n" + mapToString(teamB_updates)
                            + "Game event reports:\n";

        for(std::string event : it->second){
            output += event;
        }
    }
    else
        std::cout << "Specified user has no reports for this game." << std::endl;
}

std::string Game::mapToString(const std::map<std::string,std::string>& myMap){ //convert map to string. separate key and value with ":"
	std::string out="";
	for(const auto& pair :myMap){
		out += pair.first + ":" + pair.second + "\n";
	}
	return out;
}