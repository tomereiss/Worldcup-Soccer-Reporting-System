#pragma once

#include <unordered_map>
#include <queue>
#include <string>
#include "../include/Game.h"


class ClientDatabase{
    private:
    	std::unordered_map <std::string,int> topics; 	// {topic: subId}
        std::unordered_map <std::string, Game> games; // {gameName: Game}
        std::queue <std::string> unverifiedFrames; // queue of frames that waiting for receipt from the server for being executed
        std::string username;
        int subID;
        int receiptID;
    public:
        ClientDatabase(std::string user);
        virtual ~ClientDatabase();
        std::string getUsername();
        void setUsername(std::string username);
        int generateSubID(); //for subscription action
        int getSubscriptionID(std::string topic); // for unsubscribe frame creation
        std::string popFrame(); // popping next frame from queue
        bool isEmpty();
        void addUnverifiedFrame(std::string frame);
        std::string getTopic(int subID);
        void insertTopic(std::string topic, int subID); //for subscribe action
        void removeTopic(std::string topic); // for unsubscribe action
        void removeTopic(int subID); // for unsubscribe action
        bool GameObjectExists(std::string name);
        void insertEventToGame(std::string game, std::string user, Event &e);
        bool summarizeGame(std::string name, std::string user,std::string &output);
        void insertNewGame(Game g);
        void removeGameObject(std::string name);
        int generateReceiptID();
        void clear();
};