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
#include<iterator>
#include "sync.h"
using namespace std;



struct input1  // struct to pass socket_fd to threads
{
int server_fd;
struct sockaddr_in addr;
int c;
vector<int> v;
};

struct client_info
{
string ip;
string port;
// string is_shared;// file is shared or not;
bool is_shared;
};

struct group
{
 map<string,bool> client_subscribed;                //contains info of the subscribed clients.key is user id.
 map< string, vector<client_info>> file_map_clients;  //file name is key here.value vector of  client info struct.
 // map<string ,vector<vector<string>>> sha_clients;   //map for sah and client.
 vector<string> admin;                   //contains info of group admin. 0 location is ip  and 1 is port.
 map<string,int> size_info   ;           //contains file size of each file;   

};


 
struct credentials
{
string userid;
string password;
bool login_status; //if user is logged in or not
};



pthread_mutex_t lock1; //lock declairation 
map<string,group> groups ;//key is group name and value is group struct.
map<string,credentials> login_info; //key is ip+port string ,value is credentials struct.

string tracker2_ip="127.0.0.1";
int tracker2_port=1026;

string tracker1_ip="127.0.0.1";
int tracker1_port=1025;


int timer=2;

// pthread_mutex_t lock1; //lock declairation 
pthread_mutex_t lock2; 
pthread_mutex_t lock3; 



void call_print()
{

	cout<<"function"<<endl;
}

void pt()
{

	cout<<"login status"<<endl;

	for(auto p:login_info)
	{

		cout<<"key "<<p.first<<" cred "<<p.second.userid<<" "<<p.second.password<<" "<<p.second.login_status<<endl;
	}


}

void send_msg(int sockfd,string st)
{

   char buffer[100];
   strcpy(buffer,st.c_str());
   send(sockfd,buffer,sizeof(buffer),0);

}


//receiving data from tracker2.
void sync_tracker_rec(int sockfd)
{  
	// cout<<"------------login info before--------------"<<endl;
	// pt();
	map<string,group> groups1;
	map<string,credentials> login_info1;
	groups1=rec_groups(sockfd);
	login_info1=rec_login_info(sockfd);


	pthread_mutex_lock(&lock2);

	for(auto p:groups1)
	{
		groups[p.first]=p.second;
	}

	for(auto p1:login_info1)
	{
		login_info[p1.first]=p1.second;
	}
	pthread_mutex_unlock(&lock2);
	// cout<<"--------------login info after---------------"<<endl;
	// pt();
}

//sending data to tracker2.
void sync_tracker_send(int sock_id1)
{

	send_groups(sock_id1,groups);
    send_login_info(sock_id1,login_info);

}


// handling peer request here
//asking admin what to do about group join request/////
int ask_admin(vector<string> inputs,string ip,string port)
{
    
        inputs[2]="peer_authentication";

    	    // this socket will be used to comunicate to admin of the group------------------------------------
	    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
		int prt=stoi(port);
		struct sockaddr_in server_add;
		server_add.sin_family=AF_INET;
		server_add.sin_addr.s_addr = INADDR_ANY; 
		server_add.sin_port=htons(prt);
		server_add.sin_addr.s_addr=inet_addr(ip.c_str());
		// upto here---------------------------------------------------------------------------------------------------
	   
	    connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to group.------------------------

        int cmd_size=inputs.size();
        send(sock_id1,&cmd_size,sizeof(cmd_size),0);

        for(int i=0;i<inputs.size();i++)
        {
          char buffer[100];
          strcpy(buffer,inputs[i].c_str());
          send(sock_id1,buffer,sizeof(buffer),0);
          memset(buffer,'\n',sizeof(buffer));

        }

        int auth;
        recv(sock_id1,&auth,sizeof(auth),0);
        cout<<auth<<endl;

      close(sock_id1);


	     return auth;

}


//syncing to tracker 2 in every 2 seconds.
void sync_tracker1()
{   
	// cout<<"sync tracker called"<<endl;
	int ct=2;

	while(ct--)
	{

		

		// this socket will be used to comunicate to tracker2-----------------------------------------------------------
	    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
		int prt=tracker2_port;
		struct sockaddr_in server_add;
		server_add.sin_family=AF_INET;
		server_add.sin_addr.s_addr = INADDR_ANY; 
		server_add.sin_port=htons(prt);
		server_add.sin_addr.s_addr=inet_addr(tracker2_ip.c_str());
		// upto here---------------------------------------------------------------------------------------------------
		int status;
		status=connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to client.------------------------
		if(status!=0)
		{
			// cout<<"tracker 2 not live";
			sleep(1);
			continue;
		}
		// cout<<status<<"stat"<<endl;

		int arg_ct=3;
		send(sock_id1,&arg_ct,sizeof(int),0);//argc

		char buffer[100];
		memset ( buffer, '\0',sizeof(buffer));
		strcpy(buffer,tracker1_ip.c_str());
		// printf("%s ip", buffer);
		// cout<<endl;
		send(sock_id1,buffer,sizeof(buffer),0);//ip
		fflush(stdin);

		string port1=to_string(tracker1_port);
		strcpy(buffer,port1.c_str());
		// printf("%s port", buffer);
		// cout<<endl;
		send(sock_id1,buffer,sizeof(buffer),0);//port
		fflush(stdin);

		string cmd="sync";
		strcpy(buffer,cmd.c_str());
		// printf("%s command", buffer);	
		send(sock_id1,buffer,sizeof(buffer),0);//command
		fflush(stdin);

		sync_tracker_send(sock_id1);//sending database to tracker2.

		close(sock_id1);


	}



}




string request_handler(vector<string> inputs,int sockfd)
{

// pthread_mutex_lock(&lock2);
// standard convention for inputs is:inputs[0]=ip,inputs[1]=port,inputs[3]=commands.
string ret="";
string command(inputs[2]);
// strcpy(command,inputs[2]);//because third location contains command info;

	if(command=="create_user")
	{       string ret="";
			struct credentials cred;
			string key=inputs[0]+inputs[1];
			// cred.userid=inputs[3];
			// // cout<<cred.userid<<endl;
			// cred.password=inputs[4];
			// cred.login_status=false;

			login_info[key].userid.assign(inputs[3]);
			// cout<<cred.userid<<endl;
			login_info[key].password.assign(inputs[4]);
			login_info[key].login_status=false;

			pthread_mutex_lock(&lock1);
			// login_info[key]=cred;
			cout<<login_info[key].userid<<endl;
			pthread_mutex_unlock(&lock1);
			cout<<"user created"<<endl;
			send_msg(sockfd,"user created");
			return ret ;
	}
	else if(command=="login")
	{   
		    string ret="";
			string key=inputs[0]+inputs[1];
			if (login_info.find(key) == login_info.end()) 
	            { 
	            	cout<<"signup first"<<endl; 
	            	send_msg(sockfd,"signup first");
	            	return ret;
	            }
			
			string userid=inputs[3];
			string password=inputs[4];
			if(userid==(login_info[key].userid) && password==login_info[key].password)
			{
				login_info[key].login_status=true;
				cout<<"you are logged in"<<endl;
				sync_tracker1();//tracker sync.
				send_msg(sockfd,"you are logged in");
				return ret;
			}
				
			else
			{
				cout<<"signup first"<<endl;
				send_msg(sockfd,"signup first");

			}
				
			

			return ret;

	}
	else if(command=="create_group")
	{   

		string ret="";
        string key=inputs[0]+inputs[1];
		// if(login_info[key].login_status==false)//login check.
  //       {
  //      	cout<<"login first"<<endl;

  //      	return ret;
  //       }



	       // string ret="";
		   // string key=inputs[0]+inputs[1];
	       string userid=login_info[key].userid;
	       
	       string group_id=inputs[3];
	       // cout<<group_id<<endl;
	       struct group g;
	       g.admin.push_back(inputs[0]);//ip of admin.
	       g.admin.push_back(inputs[1]);//port of admin.
	       g.client_subscribed[userid]=true;
	       groups[group_id]=g;

	    //     for(auto const& it:groups)
	    // {
	    // 	 cout<<it.first<<endl;
	    // }

	       cout<<"group created"<<endl;
	       send_msg(sockfd,"group created");
	       sync_tracker1();//tracker sync.
	       return ret;
	}
	else if(command=="join_group")
	{  
      
        // based on the admin response tracker will let  peers  join the group.
         

          string group_id=inputs[3];

          int approval = ask_admin(inputs,groups[group_id].admin[0],groups[group_id].admin[1]);

          cout<<approval<<endl;
          // cout<<"we are here"<<endl;

          if(approval==2)
	      { 

	      	string ret="";
	      	cout<<" ************ admin blocked you from joining group********************* "<<endl;
	      	send_msg(sockfd,"admin blocked you from joining group");
	      	return ret;
	      }
          if(approval==1){
           string ret="";
	       string key=inputs[0]+inputs[1];
	       string userid=login_info[key].userid;
	       string group_id=inputs[3];
	       groups[group_id].client_subscribed[userid]=true;
	       cout<<groups[group_id].client_subscribed[userid]<<endl;
	       cout<<"************group joined**************"<<endl;
	       sync_tracker1();//tracker sync.
	       send_msg(sockfd,"group joined");

	       return ret;
	       }


	}
	else if(command=="leave_group")
	{
      send_msg(sockfd,"group left");
	}
	else if(command=="list_groups")
	{


	    string group_list=" ";
        string key=inputs[0]+inputs[1];

	    if(login_info[key].login_status==false)//login check.
        {
       	cout<<"login first"<<endl;
       	send_msg(sockfd,"login first");
       	return group_list;
        }

	    cout<<group_list<<endl;;
	    for(auto const& it:groups)
	    {
	    	group_list=group_list+" "+it.first;
	    }
	    cout<<group_list<<endl;
	    send_msg(sockfd,group_list);
	    
	    return group_list;
	}
	else if(command=="list_file")
	{    
     cout<<"list_file called"<<endl;
     map< string, vector<client_info>>::iterator it; 
     string key=inputs[0]+inputs[1];
	 string userid=login_info[key].userid;
	 // cout<<userid<<endl;
	 string group_id=inputs[3];
     // cout<<groups[group_id].client_subscribed.size()<<endl;
	 if(groups[group_id].client_subscribed.find(userid)!=groups[group_id].client_subscribed.end())//program is not entering in else block in any case bug.
	 {  

	 	string files="";
        for(it=groups[group_id].file_map_clients.begin();it!=groups[group_id].file_map_clients.end();it++)
        {

        	cout<<it->first<<endl;
        	files+=files+" "+it->first;
        }
        send_msg(sockfd,files);
       

	 }
	 else if(groups[group_id].client_subscribed.find(userid)==groups[group_id].client_subscribed.end())
	 {

	 	cout<<"*******join the group first************";
	 	send_msg(sockfd,"join the group first");
	 }








	}
	else if(command=="logout")
	{

       string key=inputs[0]+inputs[1];
       login_info[key].login_status=false;
       send_msg(sockfd,"you are logged out now");
       sync_tracker1();//tracker sync.
      

	}
	else if(command=="upload_file")
	{
     //we can update sha info also here
      string name_file=inputs[4];
      string name_group=inputs[3];
      string size=inputs[5];// taking size also because we are storing file size on tracker.
      struct client_info client_info1 ;
      client_info1.ip=inputs[0];
      client_info1.port=inputs[1];
      client_info1.is_shared=true;
      groups[name_group].file_map_clients[name_file].push_back(client_info1);

      groups[name_group].size_info[name_file]=stoi(size);
      cout<<"file uploaded"<<endl;
      send_msg(sockfd,"file uploaded");
      sync_tracker1();//tracker sync.

	}
	else if (command=="download_file")
	{  
        cout<<inputs[0]<<"new"<<endl;
		cout<<inputs[1]<<"new"<<endl;
		cout<<"******download_function hit,here tracker will issue make_filecommand to the peer******"<<endl;
		send_msg(sockfd,"downloading file ");
		string group_id=inputs[3];
		string file_name=inputs[4];

        int file_size=groups[group_id].size_info[file_name];//fetching file size from tracker
        cout<<file_size<<endl;

		vector<string> mk_file(3);
		mk_file[0]="make_file";
		mk_file[1]=file_name;
		mk_file[2]=to_string(file_size);




	    // this socket will be used to comunicate to peer who has requested info------------------------------------
	    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
		int prt=stoi(inputs[1]);
		struct sockaddr_in server_add;
		server_add.sin_family=AF_INET;
		server_add.sin_addr.s_addr = INADDR_ANY; 
		server_add.sin_port=htons(prt);
		server_add.sin_addr.s_addr=inet_addr(inputs[0].c_str());
		// upto here---------------------------------------------------------------------------------------------------
	     // pthread_mutex_lock(&lock1);
	    connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to client.------------------------

        

        int count=3;//three argument back to client 1=command,2=filename,3=size;
        int peer_having_file=groups[group_id].file_map_clients[file_name].size();//no of pears having particular file

	    send(sock_id1,&count,sizeof(count),0);
		
	      //sending input command to tracker.------------------------------------------------------------------------------------
		for(int i=0;i<count;i++)
		{

			char vrr[100];

			strcpy(vrr,mk_file[i].c_str());
	        cout<<vrr<<endl;
			send(sock_id1,vrr,sizeof(vrr),0);
		}

		send(sock_id1,&(peer_having_file),sizeof(peer_having_file),0);//sending no of client havinfg file.

		for(int i=0;i<peer_having_file;i++)
		{

			char vrr[100];

			strcpy(vrr,groups[group_id].file_map_clients[file_name][i].ip.c_str());//sending ip of a peer having file
	        cout<<vrr<<endl;
			send(sock_id1,vrr,sizeof(vrr),0);

			char vrr1[100];

			strcpy(vrr1,groups[group_id].file_map_clients[file_name][i].port.c_str());//sending port of a peer having file
	        cout<<vrr1<<endl;
			send(sock_id1,vrr1,sizeof(vrr1),0);

		}
         

      
      
	}
	else if (command=="sync")
	{
		// cout<<"sync entered"<<endl;
		sync_tracker_rec(sockfd);


	}
	else
	{
		cout<<"invalid command"<<endl;
		send_msg(sockfd,"invalid input");
	}
// pthread_mutex_unlock(&lock2);
return ret;
}




void*rec(void*thread_in)  // function corresponding to each thraed
{   
	struct sockaddr_in  peer_addr;//sock address struct for connected peer.
	// vector<string> inputs;
	input1*in=(input1*)thread_in;
	int fd=in->server_fd;
	struct sockaddr_in addr1=in->addr;
	int count=0;
	int len=sizeof(addr1);
    // cout<<fd<<endl;
	int sockfd = accept( fd , (struct sockaddr *)&peer_addr, (socklen_t*)&len);
	// cout<<sockfd<<"new"<<endl;

    // getting ip of client and pushing in input vector at 0th and first location-------------------
    // char ip[100]; 
    // inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, 50); 
    // printf("connection established with IP : %s and PORT : %d\n",ip, ntohs(peer_addr.sin_port)); 
    // int port1=ntohs(peer_addr.sin_port);
    // string ip1(ip);
    // string port=to_string(port1);
    // cout<<port<<endl;
    // cout<<ip1<<endl;
   
    
    // getting ip of client-------------------------------------------------------------------------

  while(1)
    {

		int argc;
		vector<string> inputs;

	    // cout<<"hello"<<endl;
	    int status;
		status=recv(sockfd,&argc,sizeof(argc),0);

		if(status==0 || status==-1)break;

		// cout<<argc<<"argument count"<<endl;
		// cout<<"recv called"<<endl;

		if(argc==0  || argc==2)//if peer simply press q then this loop will terminate.Hence will stop receiving from this client.
			break;
		
		
	    
		
	     //getting inputs from client.oth location for ip first for port second for command.
		for(int i=1;i<=argc;i++)
		{   
			char vrr[100];
			recv(sockfd,vrr,sizeof(vrr),0);
			// cout<<vrr<<endl;
			string str(vrr);
			
			inputs.push_back(str);
			memset ( vrr, '\0',100);
			fflush(stdin);

		}
	     // cout<<"connected to "<<inputs[0]<<" "<<inputs[1]<<endl;
		// calling handler function argument is inputs vector of string.
		request_handler(inputs,sockfd);
   }

	close(sockfd);
    pthread_exit(NULL);

}





//syncing to tracker 2 in every 2 seconds.
void*sync_tracker(void*arguments)
{   
	cout<<"sync tracker called"<<endl;

	while(1)
	{

		sleep(2);

		// this socket will be used to comunicate to tracker2-----------------------------------------------------------
	    int sock_id1=socket(AF_INET,SOCK_STREAM,0);
		int prt=tracker2_port;
		struct sockaddr_in server_add;
		server_add.sin_family=AF_INET;
		server_add.sin_addr.s_addr = INADDR_ANY; 
		server_add.sin_port=htons(prt);
		server_add.sin_addr.s_addr=inet_addr(tracker2_ip.c_str());
		// upto here---------------------------------------------------------------------------------------------------
		int status;
		status=connect(sock_id1,(struct sockaddr*)&server_add,sizeof(server_add)); //connecting to client.------------------------
		if(status!=0)
		{
			// cout<<"tracker 2 not live";
			continue;
		}
		// cout<<status<<"stat"<<endl;

		int arg_ct=3;
		send(sock_id1,&arg_ct,sizeof(int),0);//argc

		char buffer[100];
		memset ( buffer, '\0',sizeof(buffer));
		strcpy(buffer,tracker1_ip.c_str());
		// printf("%s ip", buffer);
		// cout<<endl;
		send(sock_id1,buffer,sizeof(buffer),0);//ip
		fflush(stdin);

		string port1=to_string(tracker1_port);
		strcpy(buffer,port1.c_str());
		// printf("%s port", buffer);
		// cout<<endl;
		send(sock_id1,buffer,sizeof(buffer),0);//port
		fflush(stdin);

		string cmd="sync";
		strcpy(buffer,cmd.c_str());
		// printf("%s command", buffer);	
		send(sock_id1,buffer,sizeof(buffer),0);//command
		fflush(stdin);

		sync_tracker_send(sock_id1);//sending database to tracker2.

		close(sock_id1);


	}



}



int main()
{   
   int num_args=5;
   struct input1 args;


	 if (pthread_mutex_init(&lock1, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
	


// socket creation -------------------------------------------------------------

	int server_fd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	int prt=tracker1_port;
	addr.sin_port=htons(prt);

	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int len=sizeof(addr);
	if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr))==0)
		cout<<"bounded"<<endl;
	else
		cout<<"not bounded"<<endl;
	
	listen(server_fd,5);
// socket creation ------------------------------------------------------------------


	// tracker sync------------------------------------------------------------------
	int id=1;
	pthread_t th1;
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_create(&th1,&attr1,&sync_tracker,&id);



// infinitely listening for peer------------------------------------------------------------

    while(1)
   {         
   	        
   	        pthread_t tids[num_args];

			for(int i=0;i<num_args;i++)
			{
			
			args.server_fd=server_fd;
			args.addr=addr;
			// args.c=0;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&tids[i],&attr,&rec,&args);
			}

			for(int i=0;i<num_args;i++)
			{
			pthread_join(tids[i],NULL);
			}


    }
 // infinitely listening--------------------------------------------------------------

    pthread_join(th1,NULL);
    pthread_mutex_destroy(&lock1); 
	pthread_exit(NULL);

}



