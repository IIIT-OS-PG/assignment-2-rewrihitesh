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

void* dwnldHandle(void *arg);
void* uploadHandle(void *arg);
void* serverHandler(void* arg);

int connectedClient=0;

#define BUFFERMAX 4096

typedef struct serverDetailsFill{
                int port;
                char ip[4096];
            }serverDetails;

typedef struct peerData{
    char ip[4096];
    int port;
    vector<pair<string, long long>> fileEnteriesVector;
}peerDetails;

vector<peerDetails> clientDetailsVector;

int main(int argc, char* argv[]){
    try{
        ifstream tracker_info;
        string trackerFile(argv[1]);
        tracker_info.open(trackerFile);
        string trackerIP;
        int trackerPort=0;
        tracker_info>>trackerIP;
        tracker_info>>trackerPort;

        serverDetails* serverDeta= (serverDetails*) malloc(sizeof(serverDetails));
        strcpy(serverDeta->ip,trackerIP.c_str());
        serverDeta->port=trackerPort;

        pthread_t stid;
        pthread_attr_t sattr;
        pthread_attr_init(&sattr);
        pthread_create(&stid,&sattr,serverHandler,serverDeta);

        pthread_join(stid,NULL);
    }catch(char* excp){
    // cout<<excp<<endl;
}catch(...){
    // cout<<"danish sir main"<<endl;
}
    
	return 0;
}

void* serverHandler(void* arg){

    try{
            // default tracker port
        // all peer will connect to it
        int sockfd,newsockfd;
        serverDetails serverDeta=*(serverDetails*)arg;

        //socket address structure
        unordered_map<string, string> umap;
        
        sockaddr_in serv_addr, cli_addr;

        socklen_t clilen;

        clilen=sizeof(cli_addr);

        //Create socket

        sockfd= Socket();

        // fill socket details
        // string address_to_server="127.0.0.1";

        serv_addr = Sock_Init((char*)serverDeta.ip,serverDeta.port);

        // bind associates the socket with its local address

        Bind(sockfd,serv_addr,sizeof(serv_addr));

        //listen marks the socket referred to by sockfd as a passive socket
        //That is, as a socket that will be used to accept incoming connection requests using accept

        Listen(sockfd,15);

        clilen=sizeof(cli_addr);

        // cout<<"waiting for connection"<<endl;

        // notice accept takes the address of client length cuz it fill up the details

        while(1){
            int input;
            char buffer[BUFFERMAX];
            char buffer1[BUFFERMAX];
            bzero(buffer,sizeof(buffer));
            bzero(buffer1,sizeof(buffer1));
            newsockfd=accept(sockfd, (sockaddr *)&cli_addr,&clilen);
            if(newsockfd<0){
                err_sys("error on accept");
            }
            // cout<<"connected\nclient details:: "<<endl;
            // cout<<"Port:: "<<ntohs(cli_addr.sin_port)<<endl;
            // cout<<"IP address:: "<<inet_ntoa(cli_addr.sin_addr);
            // cout<<endl;

            //////////////////////// User Creation ////////////////////////////////////
            
            recv(newsockfd,buffer,sizeof(buffer),0); //cmd
            
            if(strcmp(buffer,"create_user")==0){
                bzero(buffer,sizeof(buffer));
                bzero(buffer1,sizeof(buffer1));
                recv(newsockfd,buffer,sizeof(buffer),0); // uname
                
                recv(newsockfd,buffer1,sizeof(buffer1),0); // pwd

                string unm(buffer);    
                string upwd(buffer1);
                umap[unm]=upwd;  
            
            }else{
                if(strcmp(buffer,"login")==0){
                    bzero(buffer,sizeof(buffer));
                    bzero(buffer1,sizeof(buffer1));
                    
                    recv(newsockfd,buffer,sizeof(buffer),0); // uname

                    recv(newsockfd,buffer1,sizeof(buffer1),0); // pwd
                    string unm(buffer);    
                    string upwd(buffer1);
                    if (umap.find(unm) != umap.end()){
                            if(umap[unm]!=upwd){
                                close(newsockfd);
                            } 
                    }else{
                        close(newsockfd);
                    }
                }
            }

            
            //////////////////////// Session Management ///////////////////////////////
            
            bzero(buffer,sizeof(buffer));
            bzero(buffer1,sizeof(buffer1)); 

            peerDetails *p= (peerDetails*) malloc(sizeof(peerDetails));
            p->fileEnteriesVector.resize(100);

            char ipReceived[MAXBUFFER];
            bzero(ipReceived,sizeof(ipReceived));
            recv(newsockfd,ipReceived,sizeof(ipReceived),0);

            strcpy(p->ip,ipReceived);
            
            // cout<<"ip:******:"<<p->ip<<endl;

            int portReceived=0;

            recv(newsockfd,&portReceived,sizeof(portReceived),0);
           
            p->port=portReceived;

            // cout<<"port::"<<portReceived<<endl;
            // number of enteries
            // but for now there is only one entry

            int noOfEnteries=0;

            recv(newsockfd,&noOfEnteries,sizeof(noOfEnteries),0);
            // cout<<"enteries::"<<noOfEnteries<<endl;

            for(int i=0;i<noOfEnteries;i++){
                char fileName[MAXBUFFER];
                bzero(fileName,sizeof(fileName));
                recv(newsockfd,fileName,sizeof(fileName),0);
                string s(fileName);

                pair<string, long long> pa;      

                pa.first=s;

                long long sizeBytes=0;

                recv(newsockfd,&sizeBytes,sizeof(sizeBytes),0);

                pa.second=sizeBytes;

                
                p->fileEnteriesVector.push_back(pa);
                
                // cout<<"fileName::"<<fileName<<endl;
            }

            clientDetailsVector.push_back(*p);  

            // cout<<"enteries::"<<noOfEnteries<<endl;

            // bzero(fileName,sizeof(fileName));

            // cout<<"portReceived from the client is:: "<<portReceived<<endl;

            connectedClient++;
        
            // cout<<"connected clients:: "<<connectedClient<<endl;    
            int *arg=(int *)malloc(sizeof(int));
            *arg=newsockfd;

            pthread_t utid;
            pthread_attr_t uattr;
            pthread_attr_init(&uattr);
            pthread_create(&utid,&uattr,dwnldHandle,arg);
            pthread_detach(utid);
        }

    }catch(char* excp){
    // cout<<excp<<endl;
}catch(...){
    // cout<<"danish sir"<<endl;
}
// close(sockfd);

}



void* dwnldHandle(void *arg){
        try{
            cout<<"Server Request download handler"<<endl;
            int newsockfd=*(int *)arg;
            int n;
            char buffer[MAXBUFFER];
            int x=1;
            while(x--){
            // cout<<"Sockfd"<<newsockfd<<endl;

                memset(buffer,'\0',sizeof(buffer));
               
                recv(newsockfd,buffer,sizeof(buffer),0);
                // cout<<"Hardclose temp"<<temp<<endl;
                int noOfClientsFileHave=0;

                string fileName(buffer);

                if(fileName=="exit"){
                    // close(newsockfd);
                    cout<<"exit Received"<<endl;
                    connectedClient--;
                    return NULL;
                }

                // cout<<"*****************************"<<endl;

                for(int i=0;i<clientDetailsVector.size();i++){
                    // cout<<"clientDetailsVector "<<clientDetailsVector.size()<<endl;
                    for(int j=0;j<clientDetailsVector[i].fileEnteriesVector.size();j++){
                        if(clientDetailsVector[i].fileEnteriesVector[j].first==fileName){
                            // cout<<"clientDetailsVector[i].fileEnteriesVector[j] "<<clientDetailsVector[i].fileEnteriesVector[j].first<<endl;
                            noOfClientsFileHave++;
                        }
                    }
                }
                // cout<<"noOfClientsFileHave "<<noOfClientsFileHave<<endl;
                // cout<<"fileName "<<fileName<<endl;

                send(newsockfd,&noOfClientsFileHave,sizeof(noOfClientsFileHave),0);

                //////////////////////////////////////////////////////////////////

                int tempPortSend=0;
                long long fileSize=0; // vector data

                ///////////////////////////////////////////////////////////////////

                char IPOfPeers[MAXBUFFER];
                

                for(int i=0;i<clientDetailsVector.size();i++){
                    for(int j=0;j<clientDetailsVector[i].fileEnteriesVector.size();j++){
                        if(clientDetailsVector[i].fileEnteriesVector[j].first==fileName){
                            bzero(IPOfPeers,sizeof(IPOfPeers));
                            strcpy(IPOfPeers,clientDetailsVector[i].ip);
                            tempPortSend=clientDetailsVector[i].port;
                            fileSize=clientDetailsVector[i].fileEnteriesVector[j].second;
                            // cout<<"2352352523*********"<<buffer<<endl;
                            send(newsockfd,IPOfPeers,sizeof(IPOfPeers),0);

                            send(newsockfd,&tempPortSend,sizeof(tempPortSend),0);
                            
                            send(newsockfd,&fileSize,sizeof(fileSize),0);
                                
                        }
                    }
                }
                close(newsockfd);
            }
        }catch(char *excp){
            cout<<excp<<endl;
        }catch(...){
            cout<<"danish"<<endl;
        }
        
        // connectedClient--;
}