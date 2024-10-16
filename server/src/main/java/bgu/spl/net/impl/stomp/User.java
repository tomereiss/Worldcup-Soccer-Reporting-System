package bgu.spl.net.impl.stomp;

import java.util.ArrayList;
import java.util.List;

public class User {
    private final String username;
    private final String password;
    private boolean isConnected;
    private int userId;
    private List<String> topics;

    public User(String name, String passw, int connectionId) {
        username = name;
        password = passw;
        userId = connectionId;
        isConnected = true;
        topics = new ArrayList<>();
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public int getUserId(){return userId;}

    public boolean doesPasswordEqual(String pass) {
        return pass.equals(this.password);
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }
    public void addTopic(String topic){topics.add(topic);}
    public void removeTopic(String topic) {topics.remove(topic);}
    public List<String> getTopics(){return topics;}
}