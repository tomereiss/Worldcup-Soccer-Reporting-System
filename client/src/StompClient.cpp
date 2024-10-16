#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include "../include/ClientDatabase.h"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

std::atomic_bool terminate;
std::mutex m;

void socketListenerThread(ConnectionHandler &handler, StompProtocol &protocol, ClientDatabase &db){
    while (1){
		std::string answer;
		int len;
		if (!handler.getFrame(answer)) {
			terminate.store(true);
			break;		
        }

		len=answer.length();
        answer.resize(len-1);

		// PARSE FLUENT MESSAGES FROM SUBSCRIPTIONS

		std::stringstream stream(answer);
		std::string responseHeader;
		std::getline(stream, responseHeader, '\n');

    	if(responseHeader == protocol.message){
    		protocol.handle_message(answer);
    	}
    	else if(responseHeader == protocol.error){
    		protocol.handle_error(answer);
			terminate.store(true);
			break;	
    	}
    	else if(responseHeader == protocol.receipt_msg){
    		if(!protocol.handle_receipt(answer)){ //disconnect case by client
				terminate.store(true);
				break;
			}
    	}
    }
}

void keyBoardListenning(StompProtocol &protocol, ConnectionHandler &handler){
	while(!terminate){
		if(protocol.isConnected){
			const short bufsize = 1024;
			char buf[bufsize];
			std::cin.getline(buf, bufsize);
			std::string keyboardInput(buf);
			std::vector<std::string> frames;
			protocol.process_input(keyboardInput,frames);
			int size = frames.size();
			for(int i=0; i<size; i++){
				if (!handler.sendFrame(frames[i]))
					std::cout << "Error with sending frame to the server.\n" << std::endl;
			}
			frames.clear();
		}
	}
}

bool joinServer(ConnectionHandler &handler, Credentials &cred, std::string login_frame){
	// client-server handshake according to the protocol
	std::string answer= "";
    std::string temp = "";
    if (!handler.connect()) {
        std::cerr << "Cannot connect to " << cred.host << ":" << cred.port << std::endl;
		return false;
    }						
	if(!handler.sendFrame(login_frame)){ // sending CONNECT frame
		std::cerr << "Problem with sending frame to " << cred.host << ":" << cred.port << std::endl;
		return false;
	}

	if (!handler.getFrame(answer)) {
		std::cout << "Problem with getting frame to " << cred.host << ":" << cred.port << std::endl;
		return false;
	} 

	// getting response from server 'CONNECTED'
	std::stringstream stream(answer);
	stream >> temp; 

	// checking that the respsonse is 'CONNECTED'
	if(temp.compare("CONNECTED") != 0 ){
		std::cout << "\n" << answer << std::endl;
		return false;
	}
	std::cout << "Server joined successfully" << std::endl;
	return true;
}

int main(int argc, char *argv[]) {
	const char* accept_version = "1.2";
	std::string answer = "";
	std::string currentHost="";
	
	while(1){
		terminate.store(false);
		

 		// get input from user
		const short bufsize = 1024;
		char buf[bufsize];
		std::cin.getline(buf, bufsize);
		std::string keyboardInput(buf);
		std::string command;
		std::stringstream stream(keyboardInput);
		stream >> command;

		if(command == StompProtocol::login){ 	
			
			// Parsing login assuming legal input
			std::string word;
			stream >> word; 
			auto pos = word.find(":");
			std::string host = word.substr(0, pos); // Parsing host
			int port = stoi(word.substr(pos + 1)); // Parsing port
			std::string newHost = host + ":" + std::to_string(port);

			if(currentHost != newHost){ 
				// VALID HOST
				std::string username,password;

				stream >> username;  // Parsing username
    			stream >> password;  // Parsing password
				Credentials credentials{host,port,username,password};
				std::string loginFrame = StompProtocol::create_login_frame(credentials, accept_version);
				ConnectionHandler handler(credentials.host, credentials.port);

				if(joinServer(handler,credentials,loginFrame)){
					//SUCCESSFULL JOIN TO SERVER

					ClientDatabase db(credentials.username);
					StompProtocol protocol(db);
					protocol.isConnected.store(true);
					db.setUsername(credentials.username);
					currentHost = newHost;

					////SERVER'S MESSAGE THREAD ROUTINE 
    				std::thread listener(socketListenerThread,std::ref(handler),std::ref(protocol),std::ref(db));
					listener.detach();

					//KEYBOARD THREAD ROUTINE 
					keyBoardListenning(protocol, handler);

					//reset the current host
					currentHost = "";
				}
			}
			else
				std::cout << "Already connected to the same host." << std::endl;
		}
		else{
			std::cout << "First command must be login command 'login host:port {username} {password}" << std::endl;
		}
	}
}



