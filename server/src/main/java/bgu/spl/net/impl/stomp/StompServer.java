package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;
import java.io.IOException;
import java.util.Arrays;

public class StompServer<T> implements Server<T> {
//    private static final String MODE_KEY = "-m";
//    private static final String PORT_KEY = "-p";

    public static void main(String[] args) {
        if(args.length == 2){
            int port=7777;
            try {
                port = Integer.parseInt(args[0]);
            }
            catch (NumberFormatException ex) {
                System.out.println("First argument must be port number");
                System.out.println("Usage: {port} {tpc/reactor}");
                System.exit(1);}

            String mode = args[1];

            if(mode.equals("reactor")){
                Server.reactor(4,
                        port,
                        StompProtocol::new,
                        StompMessageEncoderDecoder::new
                ).serve();
            }
            else if(mode.equals("tpc")){
                Server.threadPerClient(
                        port, //port
                        StompProtocol::new, //protocol factory
                        StompMessageEncoderDecoder::new //message encoder decoder factory
                ).serve();
            }
            else{
                System.out.println("Second argument must be 'tpc' or 'reactor'");
                System.out.println("Usage: {port} {tpc/reactor}");
            }
        }
        else {
            System.out.println("Usage: {port} {tpc/reactor}");
        }

    }

    @Override
    public void serve() {}

    @Override
    public void close() throws IOException {}
}


