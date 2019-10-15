#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>
using namespace std;
#include "wrapper.h"

void* reqHandle(void *arg);
void* recvHandler(void *arg);
void* serverHandler(void* arg);
void serverSend(int newsockfd);
void serverRecv(int newsockfd);
void dwnld(int fd, long long fileSize, char buffer[] ,std::vector<pair<string,int>> portOfPeersVector);

typedef struct threadData{
				char ip[MAXBUFFER];
                int port;
                char fileName[MAXBUFFER];
                long long seek_begin;
                long long readBytes;
            }threadStruct;

typedef struct serverDetailsFill{
                int port;
                char ip[MAXBUFFER];
            }serverDetails;


pthread_mutex_t lock;

// pthread_mutex_lock(&lock);

// pthread_mutex_unlock(&lock);

int main(int argc, char* argv[])
{           
            ifstream tracker_info;
            string trackerFile(argv[3]);
            tracker_info.open(trackerFile);
            string trackerIP;
            int trackerPort=0;
            tracker_info>>trackerIP;
            tracker_info>>trackerPort;

            serverDetails* serverDeta= (serverDetails*) malloc(sizeof(serverDetails));
            strcpy(serverDeta->ip,argv[1]);
            serverDeta->port=stoi(argv[2]);

            pthread_t stid;
            pthread_attr_t sattr;
            pthread_attr_init(&sattr);
            pthread_create(&stid,&sattr,serverHandler,serverDeta);
  
            while(1){
            
            int sockfd,portno,n,x;
            char buffer[MAXBUFFER];
            bzero(buffer,sizeof(buffer));
            scanf("%s",buffer);

            sockaddr_in client_addr;

            // stores info about given host

            hostent *server;


            // this is tracker port tp peer will connect
            portno=trackerPort;

            server = gethostbyname(trackerIP.c_str());

            sockfd=socket(AF_INET,SOCK_STREAM,0);

            if(sockfd<0)
                err_sys("Error opening socket");

            if(server==NULL){
                fprintf(stderr,"Error, no such host");
            }
            bzero((char *) &client_addr,sizeof(client_addr));

            client_addr.sin_family=AF_INET;

            bcopy((char *) server->h_addr_list[0], (char *) &client_addr.sin_addr.s_addr, server->h_length);

            client_addr.sin_port=htons(portno);

            if(connect(sockfd,(sockaddr *)  & client_addr, sizeof(client_addr))<0)
            {
                err_sys("error in connected part");
            }


            ////////////////Create user and all broing stuff //////////////////
            if(strcmp(buffer,"create_user")==0){
                send (sockfd ,buffer, sizeof(buffer), 0); // send command
                bzero(buffer,sizeof(buffer));
                
                scanf("%s",buffer);// send u name
                send (sockfd ,buffer, sizeof(buffer), 0);

                bzero(buffer,sizeof(buffer));
                scanf("%s",buffer); // send pwd
                send (sockfd ,buffer, sizeof(buffer), 0);
            
            }else{
                if(strcmp(buffer,"login")==0){
                    send (sockfd ,buffer, sizeof(buffer), 0); // send command
                    bzero(buffer,sizeof(buffer));
                    
                    scanf("%s",buffer);// send u name
                    send (sockfd ,buffer, sizeof(buffer), 0);

                    bzero(buffer,sizeof(buffer));
                    scanf("%s",buffer); // send pwd
                    send (sockfd ,buffer, sizeof(buffer), 0);

                }else{
                    if(strcmp(buffer,"logout")==0){
                        cout<<"connection closed"<<endl;
                        close(sockfd);
                        exit(0);
                }else{
                    cout<<"Wrong Commands, back to loop"<<endl;
                }

                }
            }


            ///////////////// End of Boring stuff///////////////////////////////
            bzero(buffer,sizeof(buffer));
            
            // cout<<"test";
            // cin>>x;

            // send detaiils to tracker

            strcpy(buffer,argv[1]);

            // int sendIPToServer=127;

            send (sockfd ,buffer, sizeof(buffer), 0);

            bzero(buffer,sizeof(buffer));

            int sendPortOfServer=serverDeta->port;

            send (sockfd ,&sendPortOfServer, sizeof(sendPortOfServer), 0);

            std::vector<pair<string,long long>> fileList=ListOfFiles();

            int vsz= fileList.size();

            send (sockfd ,&vsz, sizeof(vsz), 0);
            // senf the list of file size

            for(int i=0;i<vsz;i++){
                bzero(buffer,sizeof(buffer));       
                long long size=0;
                strcpy(buffer,fileList[i].first.c_str());
                size=fileList[i].second;
                send (sockfd ,buffer, sizeof(buffer), 0);
                send (sockfd ,&size, sizeof(size), 0);
            }
            // send detaiils to tracker

            // request file name from server
            cout<<"Enter name Of file"<<endl;
            scanf("%s",buffer);

            send(sockfd, buffer, sizeof(buffer), 0);

            // File Name u have requested::

            cout<<"File Name u have requested:: "<<buffer<<endl;

            char tempFileName[MAXBUFFER];

            bzero(tempFileName,sizeof(tempFileName));

            int noOfClientsFileHave=0;

            recv(sockfd,&noOfClientsFileHave,sizeof(noOfClientsFileHave),0);

            if(noOfClientsFileHave==0){
                cout<<"No peer have this file"<<endl;
                close(sockfd);
                continue;
            }
            cout<<noOfClientsFileHave<<" peer have file"<<endl;
            long long fileSize=0;
            int portOfPeers=0;
            char IPOfPeers[MAXBUFFER];
            
            vector<pair<string,int>> portPeerVector;
            for(int i=0;i<noOfClientsFileHave;i++){
                bzero(IPOfPeers,sizeof(IPOfPeers));
                recv(sockfd,IPOfPeers,sizeof(IPOfPeers),0);
                recv(sockfd,&portOfPeers,sizeof(portOfPeers),0);
                recv(sockfd,&fileSize,sizeof(fileSize),0);
                string ip(IPOfPeers);
                pair<string,int> peerPair=make_pair(ip,portOfPeers);

                cout<<"peer "<<i<<" :: "<<IPOfPeers<<":"<<portOfPeers<<endl;

                portPeerVector.push_back(peerPair);
            }

            cout<<"Want to download Files??"<<endl;
            cin>>tempFileName;
            if(strcmp(tempFileName,"yes")==0){
                dwnld(sockfd,fileSize,buffer,portPeerVector);
                close(sockfd);
            }else{
                close(sockfd);
            }   
        }// end of while
    
    // pthread_join(stid,NULL);
    return 0;
}

void dwnld(int fd, long long fileSize ,char fileName[], std::vector<pair<string,int>> portOfPeersVector){


            // multiple threads logic goes here
            int noOfThreads=portOfPeersVector.size();
            long long PIECESIZE=524288;
            long long seek_begin=0;
            seek_begin-=PIECESIZE;

            FILE *fp = fopen(fileName,"w");
            for(int i=0;i<fileSize;i++){
                fputc('\0',fp);
            }
            long size = ftell ( fp );
            // cout<<"RAW File Size"<<size<<endl;
            fclose(fp);
            
            threadStruct* threadDataStruct= new threadStruct[noOfThreads];

            pthread_t tid[noOfThreads];
            // send readbytes to thread to read
            while(fileSize>0){
                for(int i=0;i<noOfThreads;i++){
                	strcpy(threadDataStruct[i].ip,portOfPeersVector[i].first.c_str());
                    threadDataStruct[i].port=portOfPeersVector[i].second;
                    long long readBytes=min(PIECESIZE,fileSize);
                    fileSize-=readBytes;
                    strcpy(threadDataStruct[i].fileName,fileName);
                    seek_begin+=PIECESIZE;
                    threadDataStruct[i].seek_begin=seek_begin;
                    threadDataStruct[i].readBytes=readBytes;
                    pthread_create(&(tid[i]),NULL,recvHandler,&threadDataStruct[i]);
                    // pthread_join(tid[i],NULL);
                }
                for(int i=0;i<noOfThreads;i++){
                    pthread_join(tid[i],NULL);
                }
            }
}

void* serverHandler(void* arg){

    int sockfd, newsockfd;
    // default port
        
    serverDetails serverDeta=*(serverDetails*)arg; //data

    //socket address structure

    sockaddr_in serv_addr, cli_addr;

    socklen_t clilen;

    //Create socket

    sockfd= Socket();

    // fill socket details
    // string address_to_server="127.0.0.1";

    // cout<<"server"<<serverDeta.ip;

    serv_addr = Sock_Init((char*)serverDeta.ip,serverDeta.port);

    // bind associates the socket with its local address

    Bind(sockfd,serv_addr,sizeof(serv_addr));

    //listen marks the socket referred to by sockfd as a passive socket
    //That is, as a socket that will be used to accept incoming connection requests using accept

    Listen(sockfd,5);

    clilen=sizeof(cli_addr);

    // cout<<"waiting for connection"<<endl;

    // notice accept takes the address of client length cuz it fill up thhe details

    while(1){
        int input;
        newsockfd=accept(sockfd, (sockaddr *)&cli_addr,&clilen);
        if(newsockfd<0){
            err_sys("error on accept");
        }
        // cout<<"\nconnected\nclient details:: "<<endl;
        // cout<<"Port:: "<<ntohs(cli_addr.sin_port)<<endl;
        // cout<<"IP address:: "<<inet_ntoa(cli_addr.sin_addr)<<endl;

       
        pthread_t utid;
        pthread_attr_t uattr;
        pthread_attr_init(&uattr);
        pthread_create(&utid,&uattr,reqHandle,&newsockfd);
        pthread_detach(utid);
    }
close(sockfd);

}

void* reqHandle(void *arg){
        // cout<<"Server Request download handler"<<endl;
        int newsockfd=*(int *)arg;
        int n;
        char requestedFileName[MAXBUFFER];
        char buffer[MAXBUFFER];
        bzero(buffer,sizeof(buffer));
        bzero(requestedFileName,sizeof(requestedFileName));
        long long seek_begin;
        long long readBytes;
        // file name
        recv(newsockfd,requestedFileName,sizeof(requestedFileName),0);
        // starting postion
        recv(newsockfd,&seek_begin,sizeof(seek_begin),0);
        // size of bytes to read
        recv(newsockfd,&readBytes,sizeof(readBytes),0);

        FILE *fp = fopen(requestedFileName,"r");
        
        if(fp==NULL){
            err_sys_noexit("ERROR SERVER CLIENT 'REQHANDLE':");
            close(newsockfd);
         }
        fseek ( fp , seek_begin , SEEK_SET );
        // long long size = ftell ( fp );
        // cout<<"file current "
        while ( ( n = fread( buffer , sizeof(char) , sizeof(buffer) , fp ) ) > 0  && readBytes > 0 ){
            send (newsockfd , buffer, n, 0 );
            memset(buffer,'\0',sizeof(buffer));
            // cout<<"readbytes::"<<readBytes<<endl;
            // cout<<"n::"<<n<<endl;
            readBytes = readBytes - n ;
        }
        fclose(fp);  
        close(newsockfd); 
}

void* recvHandler(void *arg){
        // cout<<"recv handler"<<endl;
        threadStruct ts=*(threadStruct *)arg;
        int n;
        char buffer[MAXBUFFER];
        bzero(buffer,sizeof(buffer));
        strcpy(buffer,ts.fileName);
        
        // establish the connnection with given port

            int sockfd,portno;
           
            sockaddr_in client_addr;

            hostent *server;

            // this is tracker port tp peer will connect
            portno=ts.port;

            server = gethostbyname(ts.ip);

            sockfd=socket(AF_INET,SOCK_STREAM,0);

            if(sockfd<0)
                err_sys("Error opening socket");

            if(server==NULL){
                fprintf(stderr,"Error, no such host");
            }
            bzero((char *) &client_addr,sizeof(client_addr));

            client_addr.sin_family=AF_INET;

            bcopy((char *) server->h_addr_list[0], (char *) &client_addr.sin_addr.s_addr, server->h_length);

            client_addr.sin_port=htons(portno);

            if(connect(sockfd,(sockaddr *)  & client_addr, sizeof(client_addr))<0)
            {
                err_sys("error in connected part");
            }


        // end of connection
        FILE *fp = fopen(buffer,"r+");
        
        if(fp==NULL){
            err_sys("file error");
        }
        long long file_size=ts.readBytes;
        long long seek_begin=ts.seek_begin;

        if(file_size<0){
            fclose (fp);
            close(sockfd);
            return NULL;
        }

        send(sockfd, buffer,sizeof(buffer), 0);
        send(sockfd, &seek_begin,sizeof(seek_begin), 0);
        send(sockfd, &file_size,sizeof(file_size), 0);

        bzero(buffer,sizeof(buffer));

        fseek(fp,seek_begin,SEEK_SET);

        // cout<<"Port::::: "<<portno<<endl;
        // cout<<"fileSize::::: "<<file_size<<endl;
        // cout<<"seek_begin::::: "<<seek_begin<<endl;
        // cout<<"ftell::::: "<<ftell(fp)<<endl;


        while ( ( n = recv( sockfd, buffer, sizeof(buffer), 0) ) > 0  &&  file_size > 0){
                fwrite (buffer , sizeof (char), n, fp);
                bzero(buffer,sizeof(buffer));
                file_size = file_size - n;
                // cout<<"::"<<file_size<<endl;
        }
        fclose (fp);
        close(sockfd);
}
