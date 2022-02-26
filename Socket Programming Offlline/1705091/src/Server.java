package sample;
import java.io.File;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
//import javafx.util.Pair;

public class Server {
    private static ArrayList<String> LoggedinStudent = new ArrayList<>();
    private static ArrayList<String> ActiveStudent = new ArrayList<>();
    private static int MAX_BUFFER_SIZE = 1000000;
    private static int MAX_CHUNK_SIZE = 10;
    private static int MIN_CHUNK_SIZE = 5;
    private static int used_buffer = 0;
    private static HashMap<String, String> FileRequest = new HashMap<>();
    private static int requestCount = 0;
    private static HashMap<String,ArrayList<String>> clientInbox = new HashMap<>();
    private static HashMap<String, String> fileMap = new HashMap<>();

    public static void main(String[] args) throws Exception {
        ServerSocket welcomeSocket = new ServerSocket(6666);
        File uploadable_file = new File("src/files/");
        uploadable_file.mkdirs();

        while(true) {
            System.out.println("Waiting for connection...");
            Socket studentsocket = welcomeSocket.accept();
            System.out.println("Connection established");

            //open thread
            Thread clientthread = new ClientThread(studentsocket);
            clientthread.start();
        }
    }
    public ArrayList<String> getloggedinStudentList(){
        return LoggedinStudent;
    }

    public ArrayList<String> getActiveStudentList(){
        return ActiveStudent;
    }

    public int getMaxBufferSize(){
        return MAX_BUFFER_SIZE;
    }

    public int getMaxChunkSize(){
        return MAX_CHUNK_SIZE;
    }

    public int getMinChunkSize(){
        return MIN_CHUNK_SIZE;
    }

    public int getUsed_buffer(){
        return used_buffer;
    }

    public int getRequestCount(){
        return requestCount;
    }

    public HashMap<String, String> getFileRequest() {
        return FileRequest;
    }

    public HashMap<String, ArrayList<String>> getClientInbox(){
        return clientInbox;
    }

    public HashMap<String, String> getFileMap(){
        return fileMap;
    }
}

