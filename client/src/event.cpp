#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;

std::string mapToString(const std::map<std::string,std::string>& myMap){//convert map to string. separate key and value with ":"
	std::ostringstream out;
	for(const auto& pair :myMap){
		out << "	" << pair.first << ":"  << pair.second << std::endl;
	}
	return out.str();
}

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string description)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(description) {}

Event::Event(const std::string &message, std::string &username) : team_a_name(), team_b_name(), name(),
      time(), game_updates(), team_a_updates(),team_b_updates(), description()
{
    std::stringstream ss(message);
    std::string line;

    while (std::getline(ss, line)) {  //parse each line
        if (line.find("user:") != std::string::npos)
            username = line.substr(line.find(":") + 1);
        else if (line.find("team a:") != std::string::npos)
            this->team_a_name = line.substr(line.find(":") + 1);
        else if (line.find("team b:") != std::string::npos)
            this->team_b_name = line.substr(line.find(":") + 1);
        else if (line.find("event name:") != std::string::npos)
            this->name = line.substr(line.find(":") + 1);
        else if (line.find("time:") != std::string::npos)
            this->time = stoi(line.substr(line.find(":") + 1));
        else if (line.find("general game updates:") != std::string::npos) {
            while (std::getline(ss, line)) {
                if (line.find("active:") != std::string::npos){
                    this->game_updates["active"] = line.substr(line.find(":") + 1);
                }
                else if (line.find("before halftime:") != std::string::npos){
                    this->game_updates["before halftime"] = line.substr(line.find(":") + 1);
                }
                else if (line.find("team a updates:") != std::string::npos){
                    break;
                }
            }
        }
        if (line.find("team a updates:") != std::string::npos) {
            while (std::getline(ss, line)) {
                if (line.find("goals:") != std::string::npos){
                    this->team_a_updates["goals"] = line.substr(line.find(":") + 1);;
                }
                else if (line.find("possession:") != std::string::npos){
                    this->team_a_updates["possession"] = line.substr(line.find(":") + 1);
                }
                else if (line.find("team b updates:") != std::string::npos)
                    break;
            }
        }
        if (line.find("team b updates:") != std::string::npos) {
            while (std::getline(ss, line)) {
                if (line.find("goals:") != std::string::npos){
                    this->team_b_updates["goals"] = line.substr(line.find(":") + 1);
                }
                else if (line.find("possession:") != std::string::npos){
                    this->team_b_updates["possession"] = line.substr(line.find(":") + 1);
                }
                else if (line.find("description:") != std::string::npos){
                    break;
                }
                    
            }
        }
        if (line.find("description:") != std::string::npos){
            std::getline(ss, line);
            this->description = line;
        }
            
    }
}

Event::~Event()
{}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_description() const
{
    return this->description;
}

std::string Event::get_topic() const
{
    return team_a_name + "_" + team_b_name;
}



names_and_events parseEventsFile(std::string json_path){
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items()){
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items()){
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items()){
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }

    names_and_events events_and_names{team_a_name, team_b_name, events};;
    return events_and_names;
}

std::string Event::toStore() const{
    return std::to_string(time) + " - " + get_name() + ":\n\n" + get_description() + "\n\n";	
}

std::string Event::toString(std::string username) const{
    return "\nuser:" + username + "\nteam a:"
                            + get_team_a_name() + "\nteam b:" + get_team_b_name() + "\nevent name:" 
                            + get_name() + "\ntime:" + std::to_string(get_time()) + "\ngeneral game updates:\n" + mapToString(get_game_updates())
                            + "\nteam a updates:\n" + mapToString(get_team_a_updates()) + "\nteam b updates:\n" + mapToString(get_team_b_updates())
                                + "\ndescription:\n" + get_description();
}
