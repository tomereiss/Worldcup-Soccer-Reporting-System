package bgu.spl.net.impl.stomp;

public class SubscriptionPair<String, Integer>  {
    private final String username;
    private final Integer subscriptionId;

    public SubscriptionPair(String username_, Integer subscriptionId_) {
        this.username = username_;
        this.subscriptionId = subscriptionId_;
    }

    public String getUsername() {
        return username;
    }

    public Integer getSubscriptionId() {
        return subscriptionId;
    }
}