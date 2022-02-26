package sample;


import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Scanner;

public class Client {
    public static void main(String[] args) throws IOException, ClassNotFoundException {

        Socket socket = new Socket("localhost", 6666);
        System.out.println("Connection established");

        // buffers
        ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
        ObjectInputStream in = new ObjectInputStream(socket.getInputStream());
        String IDmsg = (String) in.readObject();
        System.out.println(IDmsg);

        Scanner scanner = new Scanner(System.in);
        String ID = scanner.nextLine();
        out.writeObject(ID);
        // log In
        String loginmsg = (String) in.readObject();
        if(loginmsg.equals("-1")){
            System.out.println("Already logged in from different IP");
            socket.close();
            System.exit(0);
        }
        else if(loginmsg.equals("1")){
            System.out.println("Login Successful");
        }


        while (true) {
            try {
                String command = scanner.nextLine();
                out.writeObject(command);
                // read the message sent to this client
                String fromserver = (String) in.readObject();

                if(fromserver.equalsIgnoreCase("sending list")){
                    // logged in list
                    String loggedinlist = (String) in.readObject();
                    String[] loggedin = loggedinlist.trim().split(" ");
                    // active list
                    String activelist = (String) in.readObject();
                    String[] active = activelist.trim().split(" ");

                    System.out.println("Logged In So Far:");
                    for(int i = 0; i < loggedin.length; i++){
                        int flag = 0;
                        for(int j = 0; j < active.length; j++){
                            if(loggedin[i].equals(active[j])){
                                flag = 1;
                                break;
                            }
                        }
                        if(flag == 1){
                            System.out.println(loggedin[i] + "\t online");
                        }
                        else{
                            System.out.println(loggedin[i]);
                        }
                    }
                }

                if(fromserver.equalsIgnoreCase("logging out")){
                    System.out.println(fromserver);
                    socket.close();
                    System.exit(0);
                }

                if(fromserver.equalsIgnoreCase("sending file list")){  // look up own files
                    String contentsPrivate[] = (String[]) in.readObject();
                    System.out.println("Private Files:");
                    for(int i = 0; i < contentsPrivate.length; i++){
                        System.out.println(contentsPrivate[i]);
                    }
                    String contentsPublic[] = (String[]) in.readObject();
                    System.out.println("Public Files:");
                    for(int i = 0; i < contentsPublic.length; i++){
                        System.out.println(contentsPublic[i]);
                    }
                }

                if(fromserver.equalsIgnoreCase("Enter filetype and filename:")){  // download own file
                    System.out.println(fromserver);
                    String filedetails = scanner.nextLine();
                    out.writeObject(filedetails);

                    int chunksize = (int) in.readObject();
                    int bytes = 0;
                    String details = (String) in.readObject();
                    String filename = filedetails.split(" ")[1];
                    FileOutputStream fileOutputStream = new FileOutputStream("//home/nahian/Downloads/" + filename);

                    byte[] buffer = new byte[chunksize];

                    int fileSize = Integer.parseInt(details.split(" ")[1]);
                    while (fileSize > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, fileSize))) != -1) {
                        fileOutputStream.write(buffer,0,bytes);
                        fileSize -= bytes;      // read upto file size
                    }
                    String completionmsg = (String) in.readObject();
                    System.out.println(completionmsg);
                    fileOutputStream.close();
                }

                if(fromserver.equalsIgnoreCase("searching list")){   // look up others files
                    String listmsg = (String) in.readObject();
                    if(listmsg.contains("public file list")){
                        String filelist[] = (String[]) in.readObject();
                        System.out.println(listmsg.split("'")[0].split(" ")[1] + "'s file list:");
                        for(int i = 0; i < filelist.length; i++){
                            System.out.println(filelist[i]);
                        }
                    }
                    if(listmsg.equalsIgnoreCase("No files in this directory")){
                        System.out.println(listmsg);
                    }
                }

                if(fromserver.equalsIgnoreCase("send download details")){     //download others file
                    System.out.println(fromserver);
                    String detailsmsg = (String) in.readObject();
                    System.out.println(detailsmsg);
                    String details = scanner.nextLine();
                    out.writeObject(details);

                    int chunksize = (int) in.readObject();
                    int bytes = 0;
                    String filedetails = (String) in.readObject();
                    String filename = (String) filedetails.split(" ")[0];
                    FileOutputStream fileOutputStream = new FileOutputStream("//home/nahian/Downloads/" + filename);

                    byte[] buffer = new byte[chunksize];

                    int fileSize = Integer.parseInt(filedetails.split(" ")[1]);
                    while (fileSize > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, fileSize))) != -1) {
                        fileOutputStream.write(buffer,0,bytes);
                        fileSize -= bytes;      // read upto file size
                    }
                    String completionmsg = (String) in.readObject();
                    System.out.println(completionmsg);
                    fileOutputStream.close();
                }

                if(fromserver.equalsIgnoreCase("Enter file name:")){   // file request
                    System.out.println(fromserver);
                    String filename = scanner.nextLine();
                    out.writeObject(filename);
                    String confirmmsg = (String) in.readObject();
                    System.out.println(confirmmsg);
                }

                if(fromserver.equalsIgnoreCase("unread messages:")){   // view message
                    String inboxmsg = (String) in.readObject();
                    if(inboxmsg.equalsIgnoreCase("inbox not empty")){
                        int inboxsize = (int) in.readObject();
                        for(int i = 0; i < inboxsize; i++){
                            String msg = (String) in.readObject();
                            System.out.println(msg);
                        }
                    }
                    else{
                        System.out.println(inboxmsg);
                    }
                }

                if(fromserver.equalsIgnoreCase("Enter request ID and filesize:")){  //file upload for request
                    System.out.println(fromserver);
                    String filedetails = scanner.nextLine();
                    out.writeObject(filedetails);
                    String uploadmsg = (String) in.readObject();
                    if(uploadmsg.equalsIgnoreCase("Buffer Overloaded, Please wait.")){
                        System.out.println(uploadmsg);
                    }
                    else{
                        System.out.println(uploadmsg);
                        String chunksize = uploadmsg.split("\n")[1].split(" ")[0];
                        String fileID = uploadmsg.split("\n")[1].split(" ")[1];
                        String filename = uploadmsg.split("\n")[1].split(" ")[2];

                        // uploading file chunk by chunk
                        int bytes = 0;
                        File file = new File("src/files/" + filename);

                        FileInputStream fileInputStream = new FileInputStream(file);
                        // break file into chunks
                        byte[] buffer = new byte[Integer.parseInt(chunksize)];
                        try{
                            while ((bytes=fileInputStream.read(buffer))!=-1){
                                out.write(buffer,0,bytes);
                                out.flush();

                                String chunkmsg = (String) in.readObject();
                                out.writeObject("acknowledged");
                                // chunk confirmation and timeout
                                System.out.println(chunkmsg);
                                if(chunkmsg.contains("last")){
                                    out.writeObject("file sent completed");
                                    String filereceivemsg = (String) in.readObject();
                                    System.out.println(filereceivemsg);
                                }
                            }
                            fileInputStream.close();
                        }
                        catch (SocketTimeoutException e){
                            out.writeObject("timeout");
                            System.out.println("uploading failed");
                        }
                    }

                }

                if(fromserver.equalsIgnoreCase("upload process started")){   // upload file
                    String filemsg = (String) in.readObject();
                    System.out.println(filemsg);
                    String filedetails = scanner.nextLine();
                    out.writeObject(filedetails);
                    String uploadmsg = (String) in.readObject();
                    System.out.println(uploadmsg);
                    if(uploadmsg.equalsIgnoreCase("Buffer Overloaded, Please wait.")){
                        System.out.println(uploadmsg);
                    }
                    else{
                        String chunksize = uploadmsg.split("\n")[1].split(" ")[0];
                        String fileID = uploadmsg.split("\n")[1].split(" ")[1];
                        String filename = uploadmsg.split("\n")[1].split(" ")[2];

                        // uploading file chunk by chunk
                        int bytes = 0;
                        File file = new File("src/files/" + filename);
                        FileInputStream fileInputStream = new FileInputStream(file);
                        // break file into chunks
                        byte[] buffer = new byte[Integer.parseInt(chunksize)];
                        socket.setSoTimeout(30000);
                        try{
                            System.out.println("yo");
                            while ((bytes=fileInputStream.read(buffer))!=-1){
                                out.write(buffer,0,bytes);
                                out.flush();

                                String chunkmsg = (String) in.readObject();
                                out.writeObject("acknowledged");
                                // chunk confirmation and timeout
                                System.out.println(chunkmsg);
                                if(chunkmsg.contains("last")){
                                    out.writeObject("file sent completed");
                                    String filereceivemsg = (String) in.readObject();
                                    System.out.println(filereceivemsg);
                                }

                            }
                            fileInputStream.close();
                        }
                        catch (SocketTimeoutException e){
                            out.writeObject("timeout");
                            System.out.println("uploading failed");
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
