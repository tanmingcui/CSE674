import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;

public class JHTTP_Client_POST {

  public static void main(String[] args) throws IOException {

    if ((args.length < 1) || (args.length > 2))  // Test for correct # of args
      throw new IllegalArgumentException("Parameter(s): <Server> [<Port>]");

    String server = args[0];       // Server name or IP address
    // Convert argument String to bytes using the default character encoding
    //byte[] data = args[1].getBytes();
   
    
    //try get method
    String POSTReq = new StringBuilder("POST /index.html HTTP/1.1")
    		.append("\n")
    		.append("var=100").toString();

    int servPort = (args.length == 2) ? Integer.parseInt(args[1]) : 7;
    System.out.println("The POST request content is: "+POSTReq);
    // Create socket that is connected to server on specified port
    Socket socket = new Socket(server, servPort);
    System.out.println("Connected to server...sending POST request");

    InputStream in = socket.getInputStream();
    PrintWriter pw = new PrintWriter(socket.getOutputStream());
    
    pw.write(POSTReq);
    
    byte[] recvMsg = new byte[1024];
    pw.flush();
    
    try {
    	Thread.sleep(2000);
    	in.read(recvMsg);
    }
    catch(Exception ex) {
    	System.out.println(ex.getMessage());
    }
    if(recvMsg[0] != 0)System.out.println("Received: " + new String(recvMsg));
    if(recvMsg[0]==0)System.out.println("No response from server");
    
    socket.close();
  }
}