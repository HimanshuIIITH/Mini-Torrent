#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<bits/stdc++.h>
// #include "sync.h"
using namespace std;

map<string,vector<int>> piece_info;//contains piece info for a file. Key is filename and value is pieces this client have.
pthread_mutex_t lock1; //lock declairation 
pthread_mutex_t lock2; 
pthread_mutex_t lock3; 
struct sr
{
	vector<string> ip_port; ///0th contain ip ist port and then indexes.
	FILE*fp;
	string filename;
	string file_size;
	int port;

};
struct arguments
{
	string ip;
	string port;
};

struct input1  // struct to pass socket_fd to threads
{
int server_fd;
struct sockaddr_in addr;
// int c;
// vector<int> v;
};

string tracker2_ip="127.0.0.1";
int tracker2_port=1026;

string tracker1_ip="127.0.0.1";
int tracker1_port=1025;

void recv_msg(int sockfd)
{
	char buffer[100];
	recv(sockfd,buffer,sizeof(buffer),0);
	cout<<"****Message from tracker*****"<<endl;
	printf("%s",buffer);
	cout<<endl;

}

void tracker_request_handler(vector<string>inputs,int sock_id)
{
    
	
	int count=inputs.size();
	// cout<<count<<endl;
	send(sock_id,&(count),sizeof(count),0);
	
   //sending input command to tracker.------------------------------------------------------------------------------------
	for(int i=0;i<count;i++)
	{

		char vrr[100];

		strcpy(vrr,inputs[i].c_str());
        // cout<<vrr<<endl;
		send(sock_id,vrr,sizeof(vrr),0);
		memset ( vrr, '\0',100);
	}

    if(inputs.size()>=3)
    {
					if(inputs[2]=="upload_file")
			    
				{
			         int f_size=stoi(inputs[5]);
			         int num_of_pieces=ceil(f_size/512.0);
			         for(int i=1;i<=num_of_pieces;i++)
			         {

			         	piece_info[inputs[4]].push_back(i);
			         	
			         }


				}

    }


    recv_msg(sock_id);



}


int connect_tracker()
{

	int sock_id1=socket(AF_INET,SOCK_STREAM,0);
	int prt1=tracker1_port;
	struct sockaddr_in server_add1;
	server_add1.sin_family=AF_INET;
	server_add1.sin_addr.s_addr = INADDR_ANY; 
	server_add1.sin_port=htons(prt1);
	server_add1.sin_addr.s_addr=inet_addr(tracker1_ip.c_str());


	int sock_id2=socket(AF_INET,SOCK_STREAM,0);
	int prt2=tracker2_port;
	struct sockaddr_in server_add2;
	server_add2.sin_family=AF_INET;
	server_add2.sin_addr.s_addr = INADDR_ANY; 
	server_add2.sin_port=htons(prt2);
	server_add2.sin_addr.s_addr=inet_addr(tracker2_ip.c_str());

	int status=-1;

	pthread_mutex_lock(&lock1);
	status=connect(sock_id1,(struct sockaddr*)&server_add1,sizeof(server_add1));
	pthread_mutex_unlock(&lock1);
	if(status==0)//if tracker1 is  live. 
	{
		return sock_id1;

	}
    
    pthread_mutex_lock(&lock1);
	status=connect(sock_id2,(struct sockaddr*)&server_add2,sizeof(server_add2));
	pthread_mutex_unlock(&lock1);
	if(status==0)//if tracker2 is live. 
	{
		return sock_id2;

	}

	return -1;

}


void*send_request(void*ip_port)
{  


	struct arguments* ip_port1=(struct arguments*)ip_port;


	//     // this socket will be used to comunicate to tracker-----------------------------------------------------------
 //    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
	// int prt=1025;
	// struct sockaddr_in server_add;
	// server_add.sin_family=AF_INET;
	// server_add.sin_addr.s_addr = INADDR_ANY; 
	// server_add.sin_port=htons(prt);
	// server_add.sin_addr.s_addr=inet_addr("127.0.0.1");
	// // upto here---------------------------------------------------------------------------------------------------
 //     pthread_mutex_lock(&lock1);
 //    connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to client.------------------------
 //     pthread_mutex_unlock(&lock1);


    while(1)//for taking input infinitely from  peer terminal.
    {



	    vector<string> inputs;
	    inputs.push_back(ip_port1->ip);//pushing ip at 0th location.
	    inputs.push_back(ip_port1->port);//pushing port at first location.second location is for command.
		string str;


        pthread_mutex_lock(&lock2); 
        cout<<"enter command :"<<endl;
		while(cin>>str && str!="q")
		{
            
			inputs.push_back(str);
		}
		pthread_mutex_unlock(&lock2); 

		int sockid=connect_tracker();
    	if(sockid==-1)
    	{
    		cout<<"----------------trackers are not up --------------------"<<endl;
    		sleep(1);
    		continue;
    	}


		tracker_request_handler(inputs,sockid);
		sleep(1);
		close(sockid);

    }
    pthread_exit(NULL);
}

void*receive_final(void*arg)
{   
	cout<<"entered in receive_final"<<endl;
	struct sr * sra=(struct sr*)arg;
	string ip=sra->ip_port[0];
	string port=sra->ip_port[1];

	cout<<ip<<" ip"<<endl;
	cout<<port<<" port"<<endl;
	char buffer[512];

	//connecting to particular peer-----------------------------------------------
	int server_fd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	int prt=stoi(port);
	addr.sin_port=htons(prt);
	addr.sin_addr.s_addr=inet_addr(ip.c_str());
	// int len=sizeof(addr);
	// bind(server_fd,(struct sockaddr*)&addr,sizeof(addr));
	// listen(server_fd,0);
	cout<<"waiting"<<endl;

	int j=connect(server_fd,(struct sockaddr*)&addr,sizeof(addr));
	if(j==0)
	{
		cout<<"connected"<<endl;
	}
	else
	{
		cout<<"error in final connect"<<endl;
	}
	//upto here---------------------------------------------------------------------
	string file_n;
	string file_s;
	file_n.assign(sra->filename);
    file_s.assign(sra->file_size);

	// char cmd[]="send_pieces";
	char cmd[100];
	strcpy(cmd,"send_pieces");
	char file_name1[100];
	strcpy(file_name1,file_n.c_str());
	char file_size[100];
	strcpy(file_size,file_s.c_str());
	int c=3;

	///sending command-----------------------------------------------
	// fflush(stdin);
	send(server_fd,&c,sizeof(c),0);
	send(server_fd,cmd,sizeof(cmd),0);
	// cout<<file_name1<<"dt"<<endl;
	send(server_fd,file_name1,sizeof(file_name1),0);
	// cout<<file_size<<"dt"<<endl;
	send(server_fd,file_size,sizeof(file_size),0);

	///sending pieces index to be retrived----------------------------
	int no_of_indexes=sra->ip_port.size()-2;
	cout<<no_of_indexes<<" index count sent"<<endl;
	send(server_fd,&no_of_indexes,sizeof(no_of_indexes),0);
	for(int i=2;i<=(no_of_indexes+1);i++)//iteration for index only not for ip and port
	{   
		// cout<<"three times"<<endl;
		int j=stoi(sra->ip_port[i]);
		cout<<j<<"index request to particular peer"<<endl;
		send(server_fd,&j ,sizeof(j),0);
		memset ( &j, '\0',sizeof(j));

	}

	FILE*fp1=sra->fp;
	fseek(fp1, 0 , SEEK_SET);
   
   for(int i=0;i<sra->ip_port.size();i++)
   {
   	cout<<sra->ip_port[i]<<" ip port vector"<<endl;
    
    }


	int n;
	for(int i=2;i<=(no_of_indexes+1);i++)
	{  
		// cout<<"three times"<<endl;
        
		n = recv( server_fd , buffer ,sizeof(buffer),0);
		// cout<<buffer<<endl;
		int sk;
		sk=(stoi(sra->ip_port[i])-1)*512;
       pthread_mutex_lock(&lock1); 
        rewind(fp1);
		fseek(fp1, sk , SEEK_SET);
		fwrite (buffer , sizeof (char), n, fp1);

		piece_info[file_n].push_back(stoi(sra->ip_port[i]));//storing downloaded pieces info into global map;

       pthread_mutex_unlock(&lock1);
		///////////////////////here we can update file piece info.....................
        cout<<buffer<<endl;
		memset ( buffer , '\0', 512);
		rewind(fp1);

	}

	

	// close(sockfd);
	close(server_fd);
	pthread_exit(NULL);
	// fclose(fp1);

}


void  downoad_pieces(string filename,string size,vector<vector<string>> ip_port)

{    
	cout<<"entered download piece"<<endl;
	cout<<filename<<" filename"<<endl;
	cout<<size<<"size"<<endl;
	cout<<ip_port[0][0]<< "ip from downloaded"<<endl;
	cout<<ip_port[0][1]<<"from port download"<<endl;
	int file_size=stoi(size);
	int num_args=ip_port.size();//total no of peers to be connected
	struct sr args[num_args];
	pthread_t tids[num_args];
	// empty file creation..............................................................
    
	int fsize,null_buff_size;
	fsize=file_size;
	if(fsize<=512)
	{
		null_buff_size=fsize;
	}
	else
	{
		null_buff_size=512;
	}
	char buffer_null[null_buff_size];
	memset(buffer_null,'\0',null_buff_size);
	char buffer[512];
	FILE *fop=fopen(filename.c_str(),"wb+"); 
	if(fop<0)
	{
		perror("unable to open file");
		pthread_exit(NULL);
	}

	int n=fsize;
	while(n>0)
	{
		int minus;
		if(n<512)
		{
		null_buff_size=n;
		minus=fwrite(buffer_null,sizeof(char),null_buff_size,fop);
		memset(buffer_null,'\0',null_buff_size);
		}
		else
		{
		minus=fwrite(buffer_null,sizeof(char),null_buff_size,fop);
		memset(buffer_null,'\0',null_buff_size);
		}
	    n=n-minus;
	}
	fseek(fop,0,SEEK_END);
	int file_size1=ftell(fop);
	rewind(fop);
	cout<<"file_size"<<file_size1<<endl;
	// upto here.......................................................................................
    for(int i=0;i<num_args;i++)//threading to download pieces from different clients.
	{

		// args[i].fp=fp1;
		args[i].ip_port=ip_port[i];//////////////////////
		args[i].filename.assign(filename);
		args[i].file_size.assign(size);
		args[i].fp=fop;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tids[i],&attr,&receive_final,&args[i]);

	}
    for(int i=0;i<num_args;i++)
	{

	pthread_join(tids[i],NULL);
	}

	for(int i=0;i<piece_info[filename].size();i++)
	{
      cout<<piece_info[filename][i]<<" piece finally stored "<<endl;
	}
	fclose(fop);
	pthread_exit(NULL);
	
	
}

////piece selection algo ------------------------------------------------------------------------
void piece_selection(vector<vector<string>> &ip_port)
{


	map< string , vector<vector<string>> > inter;//intermediate map.
	map< string , vector<vector<string>> >::iterator it;

	map<pair<string,string>,vector<string>> pair_vector;//key is (ip,port) and value is piece selected after random algo;
	map<pair<string,string>,vector<string>>::iterator it_pair;

	vector<vector<string>> result;//temp vector of string combine  row(key ,value) of pair_vector into vector of string
	cout<<ip_port.size()<<endl;

	for(int i=0;i<ip_port.size();i++)
	{
	    cout<<i<<endl;
		string ip;
		string port;
		vector<string>temp;
	    // cout<<"hi"<<endl;
		ip=ip_port[i][0];//////////////////////////
		// cout<<"hi"<<endl;
		port=ip_port[i][1];
		// cout<<"seg "<<endl;

		temp.push_back(ip);
		temp.push_back(port);
		for(int j=2;j<ip_port[i].size();j++)
		{
			
		    inter[ ip_port[i][j] ].push_back(temp);
	        // cout<<ip_port[i][j]<<endl;

		}
		// cout<<endl;

	}
	cout<<inter.size()<<endl;
	srand(time(0));
	for(it=inter.begin();it!=inter.end();it++)
	{

	     int size=it->second.size();
	     int r1=rand()%size;
	     // cout<<r1<<endl;

	    string ip1=it->second[r1][0];
	    string port1=it->second[r1][1];
	    pair_vector[make_pair(ip1,port1)].push_back(it->first);

	}

	// int c=0
	for(it_pair=pair_vector.begin();it_pair!=pair_vector.end();it_pair++)
	{
		vector<string> tmp;
		tmp.push_back( it_pair->first.first );
		tmp.push_back( it_pair->first.second );

		for(int i=0;i<it_pair->second.size();i++)
		{
	       tmp.push_back(it_pair->second[i]);

		}
	    result.push_back(tmp);
	}
   cout<<"before piece selection"<<endl;
	for(int i=0; i<ip_port.size(); i++)
   {

	    for(int j=0;j<ip_port[i].size();j++)
	    {
	    	cout<<ip_port[i][j]<<"  ";
	    }
	     cout<<endl;

   }

	
   ip_port=result;
   cout<<"after piece selection "<<endl;
   	for(int i=0;i<ip_port.size();i++)
   {

	    for(int j=0;j<ip_port[i].size();j++)
	    {
	    	cout<<ip_port[i][j]<<"  ";
	    }
	     cout<<endl;

   }


}



void request_piece_info(string filename,string size, vector<vector<string>> ip_port)
{
    cout<<"request info called"<<endl;
   // vector<vector<string>> ip_port_piece(ip_port);

   int client_having_file=ip_port.size();
   cout<<client_having_file<<endl;
   for(int i=0;i<client_having_file;i++)//making connection to each client.
   {
    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
    cout<<ip_port[i][1]<<endl;
	int prt=stoi(ip_port[i][1]);
	struct sockaddr_in server_add;
	server_add.sin_family=AF_INET;
	server_add.sin_addr.s_addr = INADDR_ANY; 
	server_add.sin_port=htons(prt);
	cout<<ip_port[i][0]<<endl;
	server_add.sin_addr.s_addr=inet_addr(ip_port[i][0].c_str());
	// upto here---------------------------------------------------------------------------------------------------
    connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to peer.------------------------

    int count=2;
    send(sock_id1,&count,sizeof(count),0);//no of argument we are sending to another peer.
    char cmd[100];
    strcpy(cmd,"send_piece_info");
    char file_name[100];
    // char file_name[]="help.txt";
    strcpy(file_name,filename.c_str());
     
    send(sock_id1,cmd,sizeof(cmd),0);//sending command.
    cout<<file_name<<"file name sent to peer while piece requesting"<<endl;
    // fflush(stdin);
    send(sock_id1,file_name,sizeof(file_name),0);//sending file name;

    int num_of_pieces;
    recv(sock_id1,&num_of_pieces,sizeof(num_of_pieces),0);//how many pieces a particular client have.
    cout<<num_of_pieces<<"pieces this client have"<<endl;

    for(int j=0;j<num_of_pieces;j++)
    {
       int piece;
       recv(sock_id1,&piece,sizeof(piece),0);
       cout<<piece<<"received pieces"<<endl;
       ip_port[i].push_back(to_string(piece));
       // fflush(stdin);
    }

    close(sock_id1);

   }

    piece_selection(ip_port);//after receiving pieces we will randomly pick a peer for particular peer.

    downoad_pieces(filename,size,ip_port); 


}


//accepting request at peer side...........................................................................
void*accept_request(void*args)
{   
    vector<string> inputs;//for input command vector of string.
	struct input1* argument=(struct input1*)args;
	struct sockaddr_in addr;
	int peer_fd;
	addr=argument->addr;
	peer_fd=argument->server_fd;
   	int len=sizeof(addr);
   	
	int sockfd = accept( peer_fd , (struct sockaddr *)&addr, (socklen_t*)&len);//
	
    int argc;//getting no of elements in command.
	recv(sockfd,&argc,sizeof(argc),0);
	cout<<argc<<" cmd args count"<<endl;

	
    // char vrr[100];
	for(int i=1;i<=argc;i++)//getting command and additional info.
	{   
		// cout<<"times"<<endl;
		char vrr[100];
		recv(sockfd,vrr,sizeof(vrr),0);
		// cout<<"hi";
		cout<<vrr<<endl;
		string str1(vrr);
		
		inputs.push_back(str1);
		// fflush(stdin);
		memset ( vrr, '\0',100);
		

	}
    // cout<<"hello12"<<endl;
	if(inputs[0]=="make_file")
	{

	      cout<<"make_file called here \n";
	      cout<<"file name "<<inputs[1]<<endl;
	      cout<<"file size "<<inputs[2]<<endl;

	      // make_file(inputs[1],inputs[2]);//craete file of retreived size

	      vector<vector<string>> ip_port;//containing ip and port info;
	      int client_having_file;
	      recv(sockfd,&client_having_file,sizeof(client_having_file),0);
	      cout<<client_having_file<<endl;
	      for(int i=1;i<=client_having_file;i++)//taking ip and port from tracker.
	      {
	      	vector<string> v;
	      	char vrr[100];
	      	recv(sockfd,vrr,sizeof(vrr),0);
	      	cout<<vrr<<endl;
			string str(vrr);
			v.push_back(str);
			// fflush(stdin);

			char vrr1[100];
	      	recv(sockfd,vrr1,sizeof(vrr1),0);
	      	cout<<vrr1<<endl;
			string str1(vrr1);
			v.push_back(str1);

	        ip_port.push_back(v);
	        // fflush(stdin);

	      }

	      request_piece_info(inputs[1],inputs[2], ip_port); //0file name ,1filesize requesting piece info from client.


	}
	else if(inputs[0]=="send_piece_info")
	{   

		string file_name=inputs[1];

		cout<<"chunk info requested for file : "<<file_name<<endl;
		cout<<"Chunk info send :"<<endl;
		
		int pieces;
		pieces=piece_info[file_name].size();
		// cout<<pieces<<endl;
		send(sockfd,&pieces,sizeof(pieces),0);
		for(int i=0;i<pieces;i++)
		{
			int piece;
			piece=piece_info[file_name][i];
			cout<<piece<<" ";
			send(sockfd,&piece,sizeof(piece),0);
		}
		cout<<endl;

	}
	else if(inputs[0]=="send_pieces")
	{
       // cout<<"send_pieces called"<<endl;
       string f_name=inputs[1];
       string f_size=inputs[2];
        
       FILE*fp1=fopen(f_name.c_str(),"rb");
       vector<int> indexes;//vector of received indexes.
       char buffer[512];
       int n;
       int num_of_indexes;

       recv(sockfd,&num_of_indexes,sizeof(num_of_indexes),0);

       cout<<"Chunks requested : "<<endl;


       for(int i=0;i<num_of_indexes;i++)//receiving indexes
       {
       	int j;
       	recv(sockfd,&j,sizeof(j),0);
       	cout<<j<<" ";
       	indexes.push_back(j);
       	memset(&j,'\0',sizeof(j));
       }
       cout<<endl;

       //   for(int i=0;i<num_of_indexes;i++)//receiving indexes
       // {
       // cout<<indexes[i]<<endl;
       // }
       
       // cout<<indexes.size()<<endl;
        for(int i=0;i<num_of_indexes;i++)//sending chunks of file
       {

       	// cout<<"final exicuted"<<endl;
       	int sk;
		sk=(indexes[i]-1)*512;
		rewind(fp1);
		fseek(fp1, sk , SEEK_SET);
		n=fread(buffer,1,sizeof(buffer),fp1);

		// cout<<"piece no"<<indexes[i]<<"piece size"<<endl;
		// cout<<buffer<<endl;
		send(sockfd,buffer,n,0);
		memset(buffer,'\0',512);
      
       }

      fclose(fp1);

	}
	else if(inputs[2]=="peer_authentication")//third index is command in this case.cmd(ip,port,cmd,groupid).
	{
		cout<<"peer "<<inputs[0]<<" "<<inputs[1]<<" wants to join group " <<inputs[3]<<endl;
		cout<<"permit peer by pressing 1/2 , 1 for permitting 2 for decline"<<endl;
		int p;
		// cin.tie(nullptr);
		
		pthread_mutex_lock(&lock2); 
		// cin>>p;
		scanf("%d",&p);
		// printf("%d",p);
		if(p==1)
		{ 
			int auth=1;
			send(sockfd,&auth,sizeof(auth),0);

		}
		 if(p==2)
		{
            int auth1=2;
			send(sockfd,&auth1,sizeof(auth1),0);
		}
		pthread_mutex_unlock(&lock2);
		close(sockfd );
		



	}
	// else if(inputs[0]=="msg")
	// {   
	// 	cout<<"****Message from tracker*****"<<endl;
	// 	for(int i=1;i<inputs.size();i++)
	// 	{

	// 		cout<<inputs[i]<<" ";

	// 	}
	// 	cout<<endl;
	// }
	else
	{
        
		cout<<"invalid input"<<endl;

	}


	close(sockfd );
	pthread_exit(0);

}




// here receive all incoming request infinitely listening(or infinitely creating thraeds according to the need)
void*receive_request(void*args)
{
   // cout<<"receiving"<<endl;
  int num_args=5;
  struct input1* argument=(struct input1*)args;

  struct input1 argument1=*argument;

// cout<<argument1.server_fd<<"first"<<endl;
   listen(argument1.server_fd,5);

   // infinitely listening for peer------------------------------------------------------------

    while(1)
   {         

   	        
   
   	        pthread_t tids[num_args];

			for(int i=0;i<num_args;i++)
			{
			
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&tids[i],&attr,&accept_request,&argument1);
			}

			for(int i=0;i<num_args;i++)
			{
			pthread_join(tids[i],NULL);
			}


    }
 // infinitely listening--------------------------------------------------------------



}

int main(int argc,char**argv)
{   
   
   if (pthread_mutex_init(&lock1, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
       if (pthread_mutex_init(&lock2, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
        if (pthread_mutex_init(&lock3, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
	// piece_info["help.txt"].push_back(2);
	// piece_info["help.txt"].push_back(4);


	char ip1[20];
	char port1[20];
	strcpy(ip1,argv[1]);
	strcpy(port1,argv[2]);
	string ip(ip1);
	string port(port1);
	struct input1 args;//struct to pass sockaddr_in to and socket_id to the receiver thread.
	struct arguments ip_port;//struct to pass ip and port.
	ip_port.ip.assign(ip);
	ip_port.port.assign(port);

	// struct arguments arguments1;
	// strcpy(arguments1.ip,ip);
	// strcpy(arguments1.port,port);
    

    // binding my program to given ip and port--------------------------------------------------------------------
     if (pthread_mutex_init(&lock1, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 


    int sock_id1=socket(AF_INET,SOCK_STREAM,0);//to avoid concurrency problems.
    // int sock_id2=sock_id1;
    int my_prt=stoi(port);
    // cout<<sock_id1<<"initialy"<<endl;

	struct sockaddr_in my_add;
	my_add.sin_family=AF_INET;
	my_add.sin_addr.s_addr = INADDR_ANY; 
	my_add.sin_port=htons(my_prt);
	my_add.sin_addr.s_addr=inet_addr(ip.c_str());
    if(bind(sock_id1,(struct sockaddr*)&my_add,sizeof(my_add))==0)
		cout<<"bounded"<<endl;
	else
		cout<<"not bounded"<<endl;
    // upto here--------------------------------------------------------------------------------------------------
    

    args.addr=my_add;
    args.server_fd=sock_id1;


	int num_args=2;
	pthread_t tids[num_args];

	pthread_attr_t attr;
	pthread_attr_init(&attr);
    // cout<<"hi"<<endl;
	pthread_create(&tids[0],&attr,&send_request,&ip_port);//thread for sending request to tracker.

	
	pthread_create(&tids[1],&attr,&receive_request,&args);//thread for receiving any request.
	pthread_join(tids[0],NULL);
	pthread_join(tids[1],NULL);
	pthread_mutex_destroy(&lock1); 
	pthread_mutex_destroy(&lock2); 
	pthread_exit(NULL);

}