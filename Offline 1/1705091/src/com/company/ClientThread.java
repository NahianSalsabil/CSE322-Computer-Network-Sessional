package com.company;

import java.io.*;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;

public class ClientThread extends Thread{
    Server server = new Server();
    Socket studentsocket;
    int filecount = 0;
    String ID;
    ClientThread(Socket studentsocket){
        this.studentsocket = studentsocket;
    }

    @Override
    public void run() {
        try {
            ObjectOutputStream out = new ObjectOutputStream(this.studentsocket.getOutputStream());
            ObjectInputStream in = new ObjectInputStream(this.studentsocket.getInputStream());
            out.writeObject("Enter Student ID: ");
            ID = (String) in.readObject();

            int flag = 1;
            for (int i = 0; i < server.getActiveStudentList().size(); i++){
                if (server.getActiveStudentList().get(i).equals(ID)){   // login failed
                    out.writeObject("-1");
                    studentsocket.close();
                    flag = 0;
                    return;
                }
            }
            if(flag == 1){   // login successful
                out.writeObject("1");
                int exist = 0;
                server.getActiveStudentList().add(ID);
                for (int i = 0; i < server.getloggedinStudentList().size(); i++){
                    if(server.getloggedinStudentList().get(i).equals(ID)){
                        exist = 1;
                        break;
                    }
                }
                if(exist == 0){  // new student log in
                    server.getloggedinStudentList().add(ID);
                    File stuDirPrivate = new File("src/com/company/" + ID + "/private");
                    stuDirPrivate.mkdirs();
                    File stuDirPublic = new File("src/com/company/" + ID + "/public");
                    stuDirPublic.mkdirs();
                }
            }

            while (true)
            {
                String command = (String) in.readObject();
                Thread.sleep(1000);
                if(command.equalsIgnoreCase("log out")){     // Log Out
                    server.getActiveStudentList().remove(new String(ID));
                    out.writeObject("logging out");
                    studentsocket.close();
                    return;
                }

                if(command.equalsIgnoreCase("lookup list")){   // Look up Student list
                    out.writeObject("sending list");
                    String loggedinlist = "";
                    for(int i = 0; i < server.getloggedinStudentList().size(); i++){
                        if(server.getloggedinStudentList().get(i) == ID)
                            continue;
                        loggedinlist += " " + server.getloggedinStudentList().get(i);
                    }
                    out.writeObject(loggedinlist);
                    String activelist = "";
                    for(int i = 0; i < server.getActiveStudentList().size(); i++){
                        if(server.getActiveStudentList().get(i) == ID)
                            continue;
                        activelist += " " + server.getActiveStudentList().get(i);
                    }
                    out.writeObject(activelist);
                }

                if(command.equalsIgnoreCase("lookup file")){  // lookup own private and public file
                    out.writeObject("sending file list");
                    File directoryPathPrivate = new File("src/com/company/" + ID + "/private");
                    String contentsPrivate[] = directoryPathPrivate.list();
                    out.writeObject(contentsPrivate);
                    File directoryPathPublic = new File("src/com/company/" + ID + "/public");
                    String contentsPublic[] = directoryPathPublic.list();
                    out.writeObject(contentsPublic);
                }

                if (command.equalsIgnoreCase("download file")){   // download own file
                    out.writeObject("Enter filetype and filename:");
                    String filedetails = (String) in.readObject();
                    String filetype = filedetails.split(" ")[0];
                    String filename = filedetails.split(" ")[1];

                    int chunksize = server.getMaxChunkSize();
                    out.writeObject(chunksize);
                    //downloading file
                    int bytes = 0;
                    File file = new File("src/com/company/" + ID + "/" + filetype + "/" + filename);
                    long filesize = file.length();
                    out.writeObject(filename + " " + filesize);
                    FileInputStream fileInputStream = new FileInputStream(file);

                    byte[] buffer = new byte[chunksize];

                    while ((bytes=fileInputStream.read(buffer))!=-1){
                        out.write(buffer,0,bytes);
                        out.flush();
                    }
                    out.writeObject("file download completed");
                    fileInputStream.close();
                }

                if(command.contains("lookup others file")){   // others files lookup
                    out.writeObject("searching list");
                    String studentID = command.split(" ")[3];
                    File directoryPath = new File("src/com/company/" + studentID + "/public");
                    String contents[] = directoryPath.list();

                    if(contents != null){
                        out.writeObject("sending " + studentID + "'s public file list");
                        out.writeObject(contents);
                    }
                    else{
                        out.writeObject("No files in this directory");
                    }
                }

                if(command.contains("download")){      // download others file
                    out.writeObject("send download details");
                    out.writeObject("Enter ID and filename:");
                    String downloaddetails = (String) in.readObject();
                    System.out.println(downloaddetails);
                    String IDforfile = downloaddetails.split(" ")[0];
                    String filename = downloaddetails.split(" ")[1];
                    System.out.println(IDforfile);
                    System.out.println(filename);
                    int chunksize = server.getMaxChunkSize();
                    out.writeObject(chunksize);

                    //downloading file
                    int bytes = 0;
                    File file = new File("src/com/company/" + IDforfile + "/public/" + filename);
                    long filesize = file.length();
                    out.writeObject(filename + " " + filesize);
                    FileInputStream fileInputStream = new FileInputStream(file);

                    byte[] buffer = new byte[chunksize];

                    while ((bytes=fileInputStream.read(buffer))!=-1){
                        out.write(buffer,0,bytes);
                        out.flush();
                    }
                    out.writeObject("file download completed");
                    fileInputStream.close();
                }

                if(command.equalsIgnoreCase("request file")){  // file request
                    out.writeObject("Enter file name:");
                    String filename = (String) in.readObject();
                    int requestCount = server.getRequestCount();
                    requestCount++;
                    String requestID = ID + "_" + requestCount;
                    server.getFileRequest().put(requestID,filename);  // hashmap e dhukalam
                    // shob client er inbox e dite hobe

                    for(int i = 0; i < server.getloggedinStudentList().size(); i++){
                        if(server.getClientInbox().containsKey(server.getloggedinStudentList().get(i))){
                            if(server.getClientInbox().get(server.getloggedinStudentList().get(i)) != null){
                                ArrayList<String> message = server.getClientInbox().get(server.getloggedinStudentList().get(i));
                                message.add("file request: " + requestID + " " + filename);
                                server.getClientInbox().replace(server.getloggedinStudentList().get(i),message);
                            }
                            else{
                                ArrayList<String> message = new ArrayList<>();
                                message.add("file request: " + requestID + " " + filename);
                                server.getClientInbox().replace(server.getloggedinStudentList().get(i),message);
                            }
                        }

                        else{
                            ArrayList<String> message = new ArrayList<>();
                            message.add("file request: " + requestID + " " + filename);
                            server.getClientInbox().put(server.getloggedinStudentList().get(i),message);
                        }
                    }
                    out.writeObject("request sent");
                }

                if(command.equalsIgnoreCase("view message")){  // view message
                    out.writeObject("unread messages:");
                    ArrayList<String> messages = server.getClientInbox().get(ID);
                    if(messages != null){
                        out.writeObject("inbox not empty");
                        out.writeObject(messages.size());
                        for(int i = 0; i < messages.size(); i++){
                            out.writeObject(messages.get(i));
                        }
                        server.getClientInbox().replace(ID, null);
                    }
                    else{
                        out.writeObject("You have no unread messages");
                    }
                }

                if(command.equalsIgnoreCase("upload for a request")){  // file upload for request
                    out.writeObject("Enter request ID and filesize:");
                    String filedetails = (String) in.readObject();
                    String requestID = filedetails.split(" ")[0];
                    String filename = server.getFileRequest().get(requestID);
                    String filesize = filedetails.split(" ")[1];
                    String filetype = "public";

                    if(server.getMaxBufferSize() < server.getUsed_buffer() + Integer.parseInt(filesize)){
                        out.writeObject("Buffer Overloaded, Please wait.");
                    }
                    else{
                        int chunksize = (int) (Math.random()*(server.getMaxChunkSize()- server.getMinChunkSize()+1)+ server.getMinChunkSize());
                        filecount++;
                        String fileID = ID + "_" + filecount;  // fileID rakshi, map kora lagbe file name er sathe
                        server.getFileMap().put(fileID, filename);
                        out.writeObject("Sending uploading details...\n" + chunksize + " " + fileID + " " + filename);

                        // receiving file chunk by chunk
                        int bytes = 0;
                        FileOutputStream fileOutputStream = new FileOutputStream("src/com/company/" + ID + "/" + filetype + "/" + filename);

                        byte[] buffer = new byte[chunksize];
                        int fileSize = Integer.parseInt(filesize);
                        int chunkcount = 0;
                        int tempfilesize = fileSize;
                        int bytes_sum = 0;
                        //int count = 0;  // for checking timeout
                        try{
                            while (fileSize > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, fileSize))) != -1) {
                                fileOutputStream.write(buffer,0,bytes);
                                fileSize -= bytes;      // read upto file size
                                bytes_sum += bytes;
                                chunkcount++;
                                System.out.println(fileSize);
                                //count++;          // for checking timeout
//                            if(count == 6){
//                                Thread.sleep(40000);
//                            }
                                if(fileSize == 0){
                                    System.out.println("yo");
                                    out.writeObject("last chunk received");
                                }
                                else{
                                    out.writeObject(chunkcount + " chunk received");
                                }
                                String backACK = (String) in.readObject();
                                System.out.println(backACK);
                                if(backACK.equalsIgnoreCase("timeout")){
                                    File file = new File("src/com/company/" + ID + "/" + filetype + "/" + filename);
                                    file.delete();
                                }
                            }
                            String completemsg = (String)  in.readObject();
                            if(completemsg.equalsIgnoreCase("file sent completed")){
                                System.out.println(completemsg);
                                if(tempfilesize == bytes_sum){
                                    out.writeObject("uploaded successfully");
                                    // notification pathate hobe requester ke
                                    String requesterID = requestID.split("_")[0];
                                    if(server.getClientInbox().containsKey(requesterID)){
                                        if(server.getClientInbox().get(requesterID) != null){
                                            ArrayList<String> message = server.getClientInbox().get(requesterID);
                                            message.add("file uploaded: " + ID + " uploaded " + filename + " file for your request");
                                            server.getClientInbox().replace(requesterID,message);
                                        }
                                        else{
                                            ArrayList<String> message = new ArrayList<>();
                                            message.add("file request: " + requestID + " " + filename);
                                            server.getClientInbox().replace(requesterID,message);
                                        }
                                    }

                                    else{
                                        ArrayList<String> message = new ArrayList<>();
                                        message.add("file request: " + requestID + " " + filename);
                                        server.getClientInbox().put(requesterID,message);
                                    }
                                }
                                else{
                                    out.writeObject("uploading failed");
                                }
                            }
                            fileOutputStream.close();
                        }
                        catch (SocketException e){
                            System.out.println("sender got disconnected");
                            File file = new File("src/com/company/" + ID + "/" + filetype + "/" + filename);
                            file.delete();
                        }
                    }
                }

                if(command.equalsIgnoreCase("upload file")){   // file upload
                    out.writeObject("upload process started");
                    out.writeObject("Enter filename,filesize and filetype:");
                    String filedetails = (String) in.readObject();
                    String filename = filedetails.split(" ")[0];
                    String filesize = filedetails.split(" ")[1];
                    String filetype = filedetails.split(" ")[2];
                    System.out.println(filename + " " + filesize);

                    if(server.getMaxBufferSize() < server.getUsed_buffer() + Integer.parseInt(filesize)){
                        out.writeObject("Buffer Overloaded, Please wait.");
                    }
                    else{
                        int chunksize = (int) (Math.random()*(server.getMaxChunkSize()- server.getMinChunkSize()+1)+ server.getMinChunkSize());
                        filecount++;
                        String fileID = ID + "_" + filecount;  // fileID rakshi, map kora lagbe file name er sathe
                        out.writeObject("Sending uploading details...\n" + chunksize + " " + fileID + " " + filename);

                        // receiving file chunk by chunk
                        int bytes = 0;
                        FileOutputStream fileOutputStream = new FileOutputStream("src/com/company/" + ID + "/" + filetype + "/" + filename);

                        byte[] buffer = new byte[chunksize];
                        int fileSize = Integer.parseInt(filesize);
                        int chunkcount = 0;
                        int tempfilesize = fileSize;
                        int bytes_sum = 0;
                        //int count = 0;  // for checking timeout
                        try{
                            while (fileSize > 0 && (bytes = in.read(buffer, 0, (int)Math.min(buffer.length, fileSize))) != -1) {
                                System.out.println("yo");
                                fileOutputStream.write(buffer,0,bytes);
                                fileSize -= bytes;      // read upto file size
                                bytes_sum += bytes;
                                chunkcount++;
                                System.out.println(fileSize);
                                //count++;          // for checking timeout
//                            if(count == 6){
//                                Thread.sleep(40000);
//                            }
                                if(fileSize == 0){
                                    out.writeObject("last chunk received");
                                }
                                else{
                                    out.writeObject(chunkcount + " chunk received");
                                }
                                String backACK = (String) in.readObject();
                                System.out.println(backACK);
                                if(backACK.equalsIgnoreCase("timeout")){
                                    File file = new File("/src/" + ID + "/" + filetype + "/" + filename);
                                    file.delete();
                                }
                            }
                            String completemsg = (String)  in.readObject();
                            if(completemsg.equalsIgnoreCase("file sent completed")){
                                System.out.println(completemsg);
                                if(tempfilesize == bytes_sum){
                                    out.writeObject("uploaded successfully");
                                }
                                else{
                                    out.writeObject("uploading failed");
                                }
                            }
                            fileOutputStream.close();
                        }
                        catch (SocketException e){
                            System.out.println("sender got disconnected");
                            File file = new File("src/com/company/" + ID + "/" + filetype + "/" + filename);
                            file.delete();
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println(ID + " got disconnected");
        }

    }

}
