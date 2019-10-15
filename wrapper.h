#define MAXBUFFER 4096


int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void err_sys(const char* x)
{
    perror(x);
    exit(1);
}

void err_sys_noexit(const char* x)
{
    perror(x);
    exit(1);
}

long long fsize(char* filename){
    
    struct stat st;
    stat(filename, &st);
    long long size = st.st_size;
    return size;
}


// above functions are called internally


void Bind(int sockfd, sockaddr_in serv_addr, int _size){

    int ret_val;

    ret_val=bind(sockfd,(sockaddr *)&serv_addr,_size);
    if(ret_val<0){
        perror("Error In Binding bindwrapper func: ");
        exit(1);
    }
}

void Listen(int fd, int queue_size){
    char *ptr;
    if( (ptr=getenv("LISTENQ"))!= NULL ){
        queue_size= atoi(ptr);
    }

    if(listen(fd, queue_size) < 0){
        err_sys("Error in listen wrapper function");
    }
}

sockaddr_in Sock_Init(char* addr, int port){
    sockaddr_in serv_addr;
    socklen_t servlen=sizeof(serv_addr);

    bzero((char *) &serv_addr, servlen);

    serv_addr.sin_family= AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(addr);
    serv_addr.sin_port= htons(port);

    return serv_addr;
}

int Socket(){
    int sockfd= socket(AF_INET, SOCK_STREAM,0);
    if(sockfd<0){
        err_sys("Error creating socket");
    }
    return sockfd;
}

vector<pair<string, long long>> ListOfFiles(char* path=NULL){

    vector<pair<string,long long>> listoffiles;
    char cwd[512];

    bzero(cwd,sizeof(cwd));

    getcwd(cwd,sizeof(cwd));

    DIR *src_dir;

    if(path==NULL){
    src_dir=opendir(cwd);        
    }else{
        src_dir=opendir(path);    
    }

    if(src_dir!=nullptr){
        struct dirent *files;
            while((files = readdir(src_dir))!= NULL){
                    if(strcmp(files->d_name,".") && strcmp(files->d_name,"..") ){
                        if(is_regular_file(files->d_name)){
                            pair<string, long long> p;
                            string s(files->d_name);
                            p.first=s;
                            p.second=fsize(files->d_name);
                            listoffiles.push_back(p);
                            // cout<<files->d_name<<endl;
                    }
                }    
             }
    }
    closedir(src_dir);

    return listoffiles;
}
