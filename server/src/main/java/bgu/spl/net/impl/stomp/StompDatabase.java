package bgu.spl.net.impl.stomp;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

/***
Singleton Class used as server database
 ***/
public class StompDatabase{
    private final ConcurrentHashMap<String, ConcurrentLinkedQueue<SubscriptionPair<String, Integer>>> channels;   // channel and a list of his user subscribers

    private final ConcurrentHashMap<String, User> credentials; // database of user credentials
    private final ConcurrentHashMap<Integer, String> connectionIdToUsername; // mapping between connectionID to username
    private final ConcurrentHashMap<String, Integer> topicMessageID; // for generating messageID per topic
    private static volatile StompDatabase instance;
    private final String valid = "OK";
    private final String user_already_connected = "User is already connected";
    private final String incorrect_password = "Incorrect password";
    private final String topic_missing = "topic does not exist";
    private final String unsubscribed_user = "user isn't subscribed to this topic";
    private final String user_already_subscribed = "user is already subscribed";


    // Singleton Class
    private StompDatabase(){
        channels = new ConcurrentHashMap<>(); // {topic : ConcurrentLinkedQueue<SubscriptionPair<username, subID>>}
        credentials = new ConcurrentHashMap<>(); // {username: User}
        connectionIdToUsername = new ConcurrentHashMap<>(); // {UserId : UserName}
        topicMessageID = new ConcurrentHashMap<>();
    }
    public static StompDatabase getInstance(){
        if(instance == null){
            synchronized (StompDatabase.class){
                if(instance == null){
                    instance = new StompDatabase();
                }
            }
        }
        return instance;
    }

    public String checkConnection(User user){
        if(credentials.containsKey(user.getUsername())) { // username exists
            User existUser = credentials.get(user.getUsername()); // the valid user
            if(existUser.doesPasswordEqual(user.getPassword())){ // password check
                if(!existUser.isConnected()){ //connection check
                    existUser.setConnected(true);
                    connectionIdToUsername.put(user.getUserId(),user.getUsername()); // need to add the pair because it being removed at disconnect
                    credentials.replace(user.getUsername(), user);//mapping the new User object to the original username.
                    return valid;
                }
                return user_already_connected;
            }
            return incorrect_password;
        }
        // username does not exist - registration process
        credentials.put(user.getUsername(), user);
        connectionIdToUsername.put(user.getUserId(),user.getUsername());
        return valid;
    }

    public String checkSend(int connectionId, String topic){
        // making sure user is indeed subscribed to the topic
        if(!channels.containsKey(topic)) // topic does not exist
            return topic_missing;

        //checking if user is subscribed
        String username = connectionIdToUsername.get(connectionId);
        ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> subscribers = channels.get(topic);
        for(SubscriptionPair<String, Integer> pair: subscribers){
            if(pair.getUsername().equals(username))
                return valid;
        }
        return unsubscribed_user;
    }

    public String checkSubscribe(int connectionId, String topic, Integer subId){
        String username = connectionIdToUsername.get(connectionId);
        User user = credentials.get(username);
        if(!channels.containsKey(topic)){ // creating new topic
            ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> list = new ConcurrentLinkedQueue<>();
            list.add(new SubscriptionPair<>(username,subId));// add the user into channel subscription
            channels.put(topic,list); //create new topic
            topicMessageID.put(topic,0);
            user.addTopic(topic);
            return valid;
        }

        // topic exist, searching for user in topic subscribers list
        ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> subscribers = channels.get(topic);
        for(SubscriptionPair<String, Integer> pair: subscribers){
            if(pair.getUsername().equals(username))
                return user_already_subscribed;
        }
        subscribers.add(new SubscriptionPair<>(username,subId));// add the user into channel subscription
        user.addTopic(topic); // adding to list of subscribed topics the new topic
        return valid;
    }

    public String checkUnsubscribe(int connectionId){
        ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> list;
        String username = connectionIdToUsername.get(connectionId);
        User user = credentials.get(username);
        List<String> userTopics = user.getTopics();

        for(String topic: userTopics){
            list = channels.get(topic);
            for(SubscriptionPair<String, Integer> pair : list) {
                if(pair.getUsername().equals(username)){
                    list.remove(pair); // remove the user from channel subscription
                    user.removeTopic(topic);
                    return valid;
                }
            }
        }
        return unsubscribed_user;
    }

    public void disconnect(int connectionId){
        String username = connectionIdToUsername.get(connectionId);
        if(username != null){
            User user = credentials.get(username);

            //set connection status in credentials in database
            user.setConnected(false);

            List<String> userTopics = user.getTopics();
            // remove the user from all topics
            for(String topic:userTopics){
                ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> list = channels.get(topic);
                for(SubscriptionPair<String, Integer> tempPair:list) {
                    if(tempPair.getUsername().equals(username)){
                        list.remove(tempPair);
                        break;
                    }
                }
            }
            //delete [connectionId,username] from ConnectionIdToUsername map in database
            connectionIdToUsername.remove(connectionId);
        }
    }

    public int getAndIncrementMessageID(String channel){
        Integer x = topicMessageID.get(channel);
        topicMessageID.put(channel,x+1);
        return x;
    }

    public ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> getUsersInChannel (String channel){
        return channels.get(channel);
    }

    public ConcurrentHashMap<String, User> getCredentials(){
        return credentials;
    }
    public void clear() {
        channels.clear();
        credentials.clear();
        topicMessageID.clear();
        connectionIdToUsername.clear();
    }
}

