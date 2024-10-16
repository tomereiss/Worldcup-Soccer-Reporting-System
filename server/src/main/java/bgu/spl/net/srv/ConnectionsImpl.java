package bgu.spl.net.srv;
import bgu.spl.net.impl.stomp.StompDatabase;
import bgu.spl.net.impl.stomp.SubscriptionPair;

import java.util.Iterator;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

/***
 Singleton Class used for managing server connections
 ***/
public class ConnectionsImpl<T> implements Connections<String> {
    private final AtomicInteger activeClientCounter;
    private final ConcurrentHashMap<Integer, ConnectionHandler<String>> connectionHandlersMap; // matching client and his connectionHandler
    private static volatile ConnectionsImpl<String> instance;

    // Singleton class
    private ConnectionsImpl(){
        activeClientCounter = new AtomicInteger(0);
        connectionHandlersMap = new ConcurrentHashMap<>();
    }

    public static ConnectionsImpl<?> getInstance(){
        if(instance == null){
            synchronized (StompDatabase.class){
                if(instance == null){
                    instance = new ConnectionsImpl<>();
                }
            }
        }
        return instance;
    }

    @Override
    public boolean send(int connectionId, String msg) {
        if(connectionHandlersMap.containsKey(connectionId)){
            connectionHandlersMap.get(connectionId).send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, String messageBody, int senderID) {
        ConcurrentLinkedQueue<SubscriptionPair<String, Integer>> usersInChannel = StompDatabase.getInstance().getUsersInChannel(channel);
        for (SubscriptionPair<String, Integer> tuple : usersInChannel){
            // need to skip to sending user
            int connectionID = StompDatabase.getInstance().getCredentials().get(tuple.getUsername()).getUserId();
            if(connectionID == senderID)
                continue;
            int messageID = StompDatabase.getInstance().getAndIncrementMessageID(channel);
            connectionHandlersMap.get(connectionID).send(createMessageFrame(channel,messageID,messageBody,tuple.getSubscriptionId()));
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connectionHandlersMap.remove(connectionId);
        activeClientCounter.decrementAndGet();
    }
    public int getConnectionId(){
        int num = activeClientCounter.incrementAndGet();
        for(int id = 0; id<=num ; id++) {//mapping between client and unique connectionID
            if (!connectionHandlersMap.containsKey(id)) {
                return id;
            }
        }
        return -1;
    }
    public void addClient(int connectionId, BlockingConnectionHandler handler){//add client to connectionHandlersMap (client id and his connectionHandler)
        connectionHandlersMap.put(connectionId,handler);
    }
    public void addClient(int connectionId, NonBlockingConnectionHandler handler){//add client to connectionHandlersMap (client id and his connectionHandler)
        connectionHandlersMap.put(connectionId,handler);
    }
    public String createMessageFrame(String topic, int messageID,String body, Integer subId){//add subscription I'd to message
        return "MESSAGE\nsubscription:" + subId + "\nmessage-id:" + messageID
                + "\ndestination:" + topic + "\n" + body + "\n";
    }
}
