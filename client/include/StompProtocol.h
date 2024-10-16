#pragma once
#include <string>
#include <atomic>
#include "../include/ClientDatabase.h"

struct Credentials{
    std::string host;
    int port;
    std::string username;
    std::string password;
};

class StompProtocol{
private:
    ClientDatabase database;
public:
    StompProtocol(ClientDatabase &db);
    virtual ~StompProtocol();
    std::string static create_send_frame(std::string topic, std::string msg);
    std::string static create_unsubscribe_frame(int id, int receipt);
    std::string static create_subscribe_frame(std::string line, int id, int receipt);
    std::string static create_login_frame(Credentials &cred, std::string version);
    std::string static create_disconnect_frame(int receipt);
    void process_input(std::string &keyboardInput, std::vector<std::string> &output);
    bool handle_receipt(std::string message);
    void handle_message(std::string message);
    void handle_error(std::string message);

    std::atomic_bool isConnected;
    static constexpr const char* login = "login";
    static constexpr const char* join = "join";
    static constexpr const char* exit = "exit";
    static constexpr const char* summary = "summary";
    static constexpr const char* report = "report";
    static constexpr const char* logout = "logout";
    static constexpr const char* connected = "CONNECTED";
    static constexpr const char* error = "ERROR";
    static constexpr const char* message = "MESSAGE";
    static constexpr const char* receipt_msg = "RECEIPT";
    static constexpr const char* subscribe = "SUBSCRIBE";
    static constexpr const char* unsubscribe = "UNSUBSCRIBE";
    static constexpr const char* disconnect = "DISCONNECT";
};

