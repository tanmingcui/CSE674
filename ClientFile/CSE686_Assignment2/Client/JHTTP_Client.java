import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class JHTTP_Client {

  public static void main(String[] args) throws IOException {

    if ((args.length < 1) || (args.length > 2))  // Test for correct # of args
      throw new IllegalArgumentException("Parameter(s): <Server> [<Port>]");

    String server = args[0];       // Server name or IP address
    // Convert argument String to bytes using the default character encoding
    //byte[] data = args[1].getBytes();
    byte[]data;
    
    //try get method
    String HEADReq = new StringBuilder("HEAD /index.html HTTP/1.1 \n").toString();
    data = HEADReq.getBytes();

    int servPort = (args.length == 2) ? Integer.parseInt(args[1]) : 7;
    System.out.println("The HEAD request content is: " + HEADReq);
    // Create socket that is connected to server on specified port
    Socket socket = new Socket(server, servPort);
    System.out.println("Connected to server...sending HEAD request");

    InputStream in = socket.getInputStream();
    OutputStream out = socket.getOutputStream();

    out.write(data);  // Send the encoded string to the server

    // Receive the same string back from the server
    //int totalBytesRcvd = 0;  // Total bytes received so far
    //int bytesRcvd;           // Bytes received in last read
    
    byte[] recvMsg = new byte[1024];
    /*while (totalBytesRcvd < data.length) {
      if ((bytesRcvd = in.read(data, totalBytesRcvd,  
                        data.length - totalBytesRcvd)) == -1)
        throw new SocketException("Connection closed prematurely");
      totalBytesRcvd += bytesRcvd;
    }  // data array is full
	*/
    out.flush();
    try {
    	Thread.sleep(2000);
    	in.read(recvMsg);
    }
    catch(Exception ex) {
    	System.out.println(ex.getMessage());
    }
    if(recvMsg[0] != 0)System.out.println("Received: " + new String(recvMsg));
    if(recvMsg[0]==0)System.out.println("No response from server");

    socket.close();  // Close the socket and its streams
  }
}