package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicInteger;

public class StompProtocol implements StompMessagingProtocol {
    private int connectionId;
    private Connections<String> connections;
    private final String accept_version = "1.2";
    private final String valid = "OK";
    private boolean distribution = false;

    private boolean terminate = false;
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(String message) {
        String[] lines = (message).split(System.lineSeparator()); // split string by Newline character
        String frame;
        switch (lines[0]){
            case "CONNECT":
                frame = handleConnect(lines);
                break;
            case "SEND":
                frame = handleSend(lines);
                break;
            case "SUBSCRIBE":
                frame = handleSubscribe(lines);
                break;
            case "UNSUBSCRIBE":
                frame = handleUnsubscribe(lines);
                break;
            case "DISCONNECT":
                frame = handleDisconnect(lines);
                break;
            default: // invalid protocol command
                frame = handleError("Invalid command",lines);
                break;
        }
        if(distribution) // distribution happened inside the handleSend function
            distribution = false;
        else
            ConnectionsImpl.getInstance().send(connectionId,frame);
    }
    private String handleConnect(String[] lines){
        // checking that all necessary fields exist
        // checking fields according to database
        Map<String, String> parsedMessage = parseMessage(lines);
        String acceptVersion = parsedMessage.get("accept-version");
        String username = parsedMessage.get("login");
        String password = parsedMessage.get("passcode");
        String host = parsedMessage.get("host");

        if(acceptVersion == null)
            return handleError("Did not contain accept-version header.",lines);
        if(!acceptVersion.equals(accept_version))
            return handleError("accept-version is not compatible.",lines);
        if(username == null)
            return handleError("Did not contain a login header.",lines);
        if(password == null)
            return handleError("Did not contain a passcode header.",lines);
        if(host == null)
            return handleError("Did not contain a hostname header.",lines);

        User newUser = new User(username,password,connectionId);
        String answer = StompDatabase.getInstance().checkConnection(newUser);
        if(!answer.equals(valid))
            return handleError(answer,lines);
        else{
            return "CONNECTED\nversion:1.2\n"; // u0000 might not be necessary
        }
    }
    private String handleSend(String[] lines){
        // checking that all necessary fields exist
        // checking fields according to database
        String frame ="";
        Map<String, String> parsedMessage = parseMessage(lines);
        String topic = parsedMessage.get("destination");

        if(topic == null)
            return handleError("Did not contain a destination header",lines);

        topic = topic.substring(1);

        String answer = StompDatabase.getInstance().checkSend(connectionId, topic);
        if(!answer.equals(valid))
            return handleError(answer,lines);
        else{
            distribution = true; // distribution
            connections.send(topic, extractBodyMessage(lines),connectionId);  // add subscription id
        }
        return frame;
    }

    private String handleSubscribe(String[] lines){
        // checking that all necessary fields exist
        // checking fields according to database
        Map<String, String> parsedMessage = parseMessage(lines);
        String topic = parsedMessage.get("destination");
        String subID = parsedMessage.get("id");
        String receipt = parsedMessage.get("receipt");

        if(topic == null)
            return handleError("Did not contain a destination header",lines);
        if(subID == null)
            return handleError("Did not contain a subscribe-id header",lines);
        if(receipt == null)
            return handleError("Did not contain a receipt-id header",lines);

        topic = topic.substring(1);

        // checking if channel exists and user isn't subscribed to it yet
        // adding the user to the subscription list
        String answer = StompDatabase.getInstance().checkSubscribe(connectionId, topic, Integer.valueOf(subID));
        if(!answer.equals(valid))
            return handleError(answer,lines);
        else{
            return "RECEIPT\nreceipt-id:"+receipt+"\n";
        }
    }

    private String handleUnsubscribe(String[] lines){
        // checking that all necessary fields exist
        // checking fields according to database

        Map<String, String> parsedMessage = parseMessage(lines);
        String subID = parsedMessage.get("id");
        String receipt = parsedMessage.get("receipt");

        if(subID == null)
            return handleError("Did not contain a subscription-id header",lines);
        if(subID.equals("-1"))
            return handleError("user is not subscribed to this topic",lines);
        if(receipt == null)
            return handleError("Did not contain a receipt-id header",lines);


        // get relevant topic from subID
        String answer = StompDatabase.getInstance().checkUnsubscribe(connectionId);
        if(!answer.equals(valid))
            return handleError(answer,lines);
        else{
            return "RECEIPT\nreceipt-id:"+receipt+"\n";
        }
    }
    private String handleDisconnect(String[] lines){
        Map<String, String> parsedMessage = parseMessage(lines);
        String receipt = parsedMessage.get("receipt");
        if(receipt == null){
            return handleError("Did not contain a receipt-id header",lines);
        }
        StompDatabase.getInstance().disconnect(connectionId);
        return "RECEIPT\nreceipt-id:"+receipt+"\n";

    }

    private String handleError(String errorReason, String[] lines){
        terminate = true;
        StringBuilder err = new StringBuilder("ERROR\nThe original message:\n============\n");
        for(String line : lines)
            err.append(line).append("\n");
        err.append("============\n").append(errorReason).append("\n\n");
        return err.toString();
    }

    private String extractBodyMessage(String[] lines){
        StringBuilder body = new StringBuilder();
        boolean started = false;
        for(String line : lines){
            if(!started) {
                if(!line.equals(""))// skipping headers
                    continue;
                else
                    started = true;
            }
            else
                body.append(line).append("\n");
        }
        return body.toString();
    }

    public static Map<String, String> parseMessage(String[] lines) {
        Map<String, String> result = new HashMap<>();
        for (String line : lines) {
            String[] fields = line.split(":");
            if (fields.length == 2) {
                result.put(fields[0].trim(), fields[1].trim());
            }
        }
        return result;
    }
    @Override
    public boolean shouldTerminate() {return terminate;}
}
