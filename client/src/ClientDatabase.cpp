#include "../include/ClientDatabase.h"
#include <iterator>

ClientDatabase::ClientDatabase(std::string user) : topics(), games(), unverifiedFrames(),username(user) ,subID(0),receiptID(0) {}

ClientDatabase::~ClientDatabase(){}

std::string ClientDatabase::getUsername(){ return username;}

void ClientDatabase::setUsername(std::string user){
    this->username = user;
}

int ClientDatabase:: generateSubID(){ //for subscription action
    subID++;
    return subID-1;
}

int ClientDatabase::getSubscriptionID(std::string topic){ // for unsubscribe frame creation
    std::unordered_map<std::string,int>::iterator it = topics.find(topic);
    if(it!=topics.end())
        return it->second;
    return -1;
}

std::string ClientDatabase::popFrame(){
    if(!unverifiedFrames.empty()){
        std::string first =  unverifiedFrames.front();
        unverifiedFrames.pop();
        return first;
    }
    return "";
    //the function is being called after the server send a receipt message to the client. Therefore, the queue can't be empty when popFrame is being called
}
bool ClientDatabase::isEmpty(){return unverifiedFrames.empty();}

void ClientDatabase::addUnverifiedFrame(std::string frame){
    unverifiedFrames.push(frame);
}

std::string ClientDatabase::getTopic(int subID){
    for(auto it = topics.begin(); it != topics.end() ; ++it){
        if(it->second == subID){
            return it->first;
        }  
    }
    return "";
}

void ClientDatabase::insertTopic(std::string topic,int subID){ //for subscribe action
    topics.emplace(topic,subID);
}

void ClientDatabase::removeTopic(std::string topic){ // for unsubscribe action
    topics.erase(topic);
}

void ClientDatabase::removeTopic(int subID){
    for(auto it = topics.begin(); it != topics.end() ; ++it){
        if(it->second == subID){
            topics.erase(it->first);
            break;
        }  
    }
}

bool ClientDatabase::GameObjectExists(std::string name){
    std::unordered_map<std::string,Game>::iterator it = games.find(name);
    if(it != games.end())
        return true;
    return false;
}

void ClientDatabase::insertEventToGame(std::string name, std::string user, Event &e){
    std::unordered_map<std::string,Game>::iterator it = games.find(name);
    if(it != games.end())
        it->second.insertEvent(user,e);
}

bool ClientDatabase::summarizeGame(std::string name, std::string user,std::string &output){
    std::unordered_map<std::string,Game>::iterator it = games.find(name);
    if(it != games.end()){
        it->second.summary(user,output);
        return true;
    }
    return false;
}

void ClientDatabase::insertNewGame(Game g){
    games.emplace(g.getGameName(),g);
}
void ClientDatabase::removeGameObject(std::string name){
    games.erase(name);
}

int ClientDatabase::generateReceiptID(){
    receiptID++;
    return receiptID-1;
}

void ClientDatabase::clear(){
    subID = 0;
    receiptID = 0;
    username = "";
    topics.clear();
    games.clear();

    while(!unverifiedFrames.empty())
        unverifiedFrames.pop();
}