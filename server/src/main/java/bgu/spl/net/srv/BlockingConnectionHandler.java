package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.stomp.StompDatabase;
import bgu.spl.net.impl.stomp.User;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketException;
import java.util.concurrent.ConcurrentHashMap;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final StompMessagingProtocol protocol;
    private final MessageEncoderDecoder<String> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private final int connectionId;


    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<String> reader, StompMessagingProtocol protocol, int connectionId_, Connections connections) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        connectionId = connectionId_;
        this.protocol.start(connectionId, connections);
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { // just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                String nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null)
                    protocol.process(nextMessage);
            }
            close();

        }
        catch (IOException se){
            try {
                close();
            }
            catch (IOException pr) {System.out.println("Problem with closing socket");}
        }
    }

    @Override
    public void close() throws IOException {
        //disconnect the user with userId that equals to this connectionId
        StompDatabase.getInstance().disconnect(connectionId);

        connected = false;
        try {
            sock.close();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void send(String msg) {
        String line = msg.substring(0,msg.indexOf("\n")); // split string by Newline character
        try{
            out.write(encdec.encode(msg));
            out.flush();
        }catch (IOException e){ System.out.println("Problem with sending message to connectionID: " + connectionId);}

        if(line.equals("ERROR")) //after sending the error to the client, close the connection
            try{
                close();
            }catch (IOException pr){System.out.println("Problem with closing socket");}
    }
}
