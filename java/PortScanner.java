package util;
import java.net.Socket;
import java.net.InetSocketAddress;

public class PortScanner{
    
    private String host;
    private int startPort, endPort;

    public PortScanner(String host, int startPort, int endPort){
        this.host = host;
        setStartPort(startPort);
        setEndPort(endPort);
    }

    public PortScanner(String host, int port){
        this(host, port, port);
    }

    public PortScanner(String host){
        this(host, 0, 65535);
    }

    private void setStartPort(int port){
        if(port < 0)
            throw new 
                java.lang.IllegalArgumentException("Port has to be positive");
        startPort = port;
    }

    private void setEndPort(int port){
        if(port > 65535)
            throw new 
                java.lang.IllegalArgumentException("Port cannot be > 65535");
        endPort = port;
    }

    public void scan(){
        for(int port = startPort; port <= endPort; port++){
            System.out.println("Port " + port + " " + 
                    (portIsOpen(port) ? "open" : "closed"));
        }
    }

    public boolean portIsOpen(int port){
        try{
            Socket s = new Socket();
            s.connect(new InetSocketAddress(host, port), 200);
            s.close();
            return true;
        }catch(Exception e){
            return false;
        }
    }

    public static void main(String[] args){
        PortScanner p = new PortScanner("localhost", 1, 200);
        p.scan();
    }
}
