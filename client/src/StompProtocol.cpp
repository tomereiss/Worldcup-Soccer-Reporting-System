#include "../include/StompProtocol.h"
#include "../include/event.h"
#include "../include/Game.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

StompProtocol::StompProtocol(ClientDatabase &db) : database(db), isConnected(false) {}

StompProtocol::~StompProtocol(){}

std::string StompProtocol::create_login_frame(Credentials &cred, std::string version){
	return "CONNECT\naccept-version:" + version +
			"\nhost:"+cred.host+"\nlogin:"+
			cred.username+"\npasscode:"+cred.password+"\n\0";
}

std::string StompProtocol::create_subscribe_frame(std::string line, int id, int receipt){
	std::stringstream stream(line);
    std::string word;
    stream >> word; // JOIN command
    stream >> word; // topic argument

	return "SUBSCRIBE\ndestination:/" + word + "\nid:" +
			std::to_string(id) +"\nreceipt:" + std::to_string(receipt) + "\n\0";
}

std::string StompProtocol::create_unsubscribe_frame(int id, int receipt){
	return "UNSUBSCRIBE\nid:" + std::to_string(id) + "\nreceipt:" +
			std::to_string(receipt) + "\n\0";
}

std::string StompProtocol::create_send_frame(std::string topic, std::string body){
	// create function that parses json file into several reports
	// and sends them via this function
	return "SEND\ndestination:/" + topic + "\n" + body + "\n\0";
}

std::string StompProtocol::create_disconnect_frame(int receipt){
	return "DISCONNECT\nreceipt:" + std::to_string(receipt) + "\n\0";
}

void StompProtocol::process_input(std::string &keyboardInput, std::vector<std::string> &output){
	std::string word = "";
	std::string frame;
	std::stringstream stream(keyboardInput);
	stream >> word;
	if(word == join){ // SUBSCRIBE
		std::string topic = "";
		stream >> topic; // extracting topic from the line
		frame = create_subscribe_frame(keyboardInput,database.generateSubID(), database.generateReceiptID());
		output.push_back(frame);
		database.addUnverifiedFrame(frame);
	}
	else if(word == exit){ // UNSUBSCRIBE
		std::string topic = "";
		stream >> topic; // extracting topic from the line
		frame = create_unsubscribe_frame(database.getSubscriptionID(topic),database.generateReceiptID());
		output.push_back(frame);
		database.addUnverifiedFrame(frame);
	}
	else if(word == report){ // SEND // // report {file}
		stream >> word;
		std::string frame;
		try{
			names_and_events fileContent = parseEventsFile(word);
			int size = fileContent.events.size();
			for(int i=0; i<size; i++){
				Event &e = fileContent.events[i]; 
				std::string topic = e.get_topic();
				if(database.getSubscriptionID(topic) == -1){
					std::cout << "user is not subscribed to: " << topic << std::endl;
					break;
				}
				std::string user = database.getUsername();
				output.push_back(create_send_frame(topic,e.toString(user)));

				//for summary usage
				if(database.GameObjectExists(topic)){
					database.insertEventToGame(topic,database.getUsername(),e);
					// Game &g = database.getGameObject(topic);
					// g.insertEvent(database.getUsername(), e);
				}
				else{
					std::cout << "client does not subscribed to this topic" << std::endl;
				}
			}
		}
		catch(const std::exception& e)
		{
			std::cout << "File not found or corrupt. Try again." << std::endl;
		}
		
		
	}
	else if(word == logout){ // DISCONNECT
		frame = create_disconnect_frame(database.generateReceiptID());
		output.push_back(frame);
		database.addUnverifiedFrame(frame);
		isConnected.store(false);
	}
	else if(word == summary){ // SUMMARY
		std::string username,gameName,file,summary;

		stream >> gameName;
		stream >> username;
		stream >> file;
		if(database.GameObjectExists(gameName)){
			std::string summary;
			if(database.summarizeGame(gameName,username,summary)){
				// writing to file
				std::ofstream f;
				f.open(file);
				if(f.is_open()){
					f << summary;
					f.close();
					std::cout << "Wrote summary of user  " << username << " reports into file " << file << std::endl;
				}
			}

		}
		else
			std::cout << "you are not subscribed to this topic" << std::endl;
		
		
	}
	else if(word == login)
		std::cout << "First use 'logout' command and then login again" << std::endl;
	else
		std::cout << "Invalid command: " << word << std::endl;

}

bool StompProtocol::handle_receipt(std::string message){
	std::stringstream stringStream(message);
	std::string line,receipt,serverReceipt;

	// parsing RECEIPT from server
	while (std::getline(stringStream, line)) {
		if (line.find("receipt-id:") != std::string::npos){
			serverReceipt = line.substr(line.find(":")+1);
			break;
		}
    }

	std::stringstream stringStream2(database.popFrame());

	while (std::getline(stringStream2, line)) {  //parse each line
        if (line.find(this->subscribe) == 0){
			std::string dest,id;
			while (std::getline(stringStream2, line)) {
                if (line.find("destination:") != std::string::npos)
                    dest = line.substr(line.find(":") + 2);
                else if (line.find("id:") != std::string::npos)
                    id = line.substr(line.find(":") + 1);
				else if (line.find("receipt:") != std::string::npos)
                    receipt = line.substr(line.find(":") + 1);
            }
			if(receipt == serverReceipt){
				database.insertNewGame(Game(dest));
				database.insertTopic(dest,std::stoi(id)); //update db topic (topic,subID)
				std::cout<< "Joined channel " << dest << std::endl;
			}
			else{
				std::cout<<"Unmatching receipts"<<std::endl;
			}
		}
        else if (line.find(this->unsubscribe) != std::string::npos) {
			int id;

            while (std::getline(stringStream2, line)) {
                if (line.find("id:") != std::string::npos)
                    id = std::stoi(line.substr(line.find(":") + 1));
                else if (line.find("receipt:") != std::string::npos)
                    receipt = line.substr(line.find(":") + 1);
            }
			if(receipt == serverReceipt){
				std::string topic = database.getTopic(id);
				//update db topic (topic,subID)
				database.removeTopic(id);
				database.removeGameObject(topic);
				std::cout<< "Exited channel " << topic << std::endl;

			}
			else
				std::cout<<"Unmatching receipts"<<std::endl;
        }
        else if (line.find(this->disconnect) != std::string::npos) {
            while (std::getline(stringStream2, line)) {
                if (line.find("receipt:") != std::string::npos)
                    receipt = line.substr(line.find(":") + 1);
            }
			if(receipt == serverReceipt){
				std::cout<< "Graceful shutdown." << std::endl;
				return false;
			}
			else
				std::cout<<"Unmatching receipts for shutdown"<<std::endl;
        }
    }
	return true;
}

void StompProtocol::handle_message(std::string message){
	// add the message to the relevant game databse, through Game object
	// if Game object does not exist create one
	std::string username;
	Event e = Event(message,username); 
	std::string topic = e.get_topic();

	//for summary usage
	//update Game object - debug is optional!!!
	if(database.GameObjectExists(topic)){
		database.insertEventToGame(topic,username,e);
		// Game &g = database.getGameObject(topic);
		// g.insertEvent(username, e);
	}
}

void StompProtocol::handle_error(std::string message){
	std::cout << "\n" << message << std::endl;
}
