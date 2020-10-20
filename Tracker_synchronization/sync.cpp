
#include "sync.h"


struct arguments
{
	int ip;
	int port;
};

struct client_info
{
string ip;
string port;
bool is_shared;// file is shared or not;
};
struct credentials
{
string userid;
string password;
bool login_status; //if user is logged in or not
};

struct group
{
 map<string,bool> client_subscribed;                //contains info of the subscribed clients.key is user id.
 map< string, vector<client_info>> file_map_clients;  //file name is key here.value vector of  client info struct.
 // map<string ,vector<vector<string>>> sha_clients;   //map for sah and client.
 vector<string> admin;                   //contains info of group admin. 0 location is ip  and 1 is port.
 map<string,int> size_info   ;           //contains file size of each file;   

};

// map<string,group> groups ;//key is group name and value is group struct.
// map<string,credentials> login_info; //key is ip+port string ,value is credentials struct.


struct client_info rec_client_info(int sockfd)
{
	struct client_info s;
	char buffer[20];
	memset(buffer,'\0',20);
	bool is_shared;

	recv(sockfd,buffer,sizeof(buffer),0);
    s.ip.assign(buffer);
    recv(sockfd,buffer,sizeof(buffer),0);
    s.port.assign(buffer);
    recv(sockfd,&is_shared,sizeof(bool),0);
    s.is_shared=is_shared;
    return s;
}
struct credentials rec_credentials(int sockfd)
{
	struct credentials s;
	char buffer[20];
	memset(buffer,'\0',20);
	bool login_status;

	recv(sockfd,buffer,sizeof(buffer),0);
    s.userid.assign(buffer);
    recv(sockfd,buffer,sizeof(buffer),0);
    s.password.assign(buffer);
    recv(sockfd,&login_status,sizeof(bool),0);
    s.login_status=login_status;
    return s;
}



struct group rec_group(int sockfd)
{
	int sz1,sz2,sz3,sz4,sz5;
	struct group g;

	recv(sockfd,&sz1,sizeof(int),0);
	map<string,bool> client_subscribed;
	for(int i=1;i<=sz1;i++)//receiving client_subscribed.
	{
		string id;
		bool sub;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		id.assign(buffer);
		recv(sockfd,&sub,sizeof(bool),0);
		client_subscribed[id]=sub;
	}
	g.client_subscribed=client_subscribed;


	recv(sockfd,&sz2,sizeof(int),0);
	map< string, vector<client_info>> file_map_clients;
	for(int i=1;i<=sz2;i++)//receiving client_subscribed.
	{
		string name;

		bool sub;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		name.assign(buffer);
		recv(sockfd,&sz3,sizeof(int),0);
		vector<client_info>v;
		for(int i=1;i<=sz3;i++)
		{
			v.push_back(rec_client_info(sockfd));
		}
		file_map_clients[name]=v;

	}
	g.file_map_clients=file_map_clients;


	recv(sockfd,&sz4,sizeof(int),0);
	// cout<<sz4<<"admin_sz"<<endl;
	vector<string> admin; 
	for(int i=1;i<=sz4;i++)//receiving admin
	{
		string  adm;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		adm.assign(buffer);
		admin.push_back(adm);
	}
	g.admin=admin;

	recv(sockfd,&sz5,sizeof(int),0);

	map<string,int> size_info   ;           
	for(int i=1;i<=sz5;i++)
	{
		string file;
		int sz;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		file.assign(buffer);
		recv(sockfd,&sz,sizeof(sz),0);

		size_info[file]=sz;

	}
	g.size_info=size_info;

return g;
}

map<string,group> rec_groups(int sockfd)
{
	map<string,group> g;
	int sz;
	recv(sockfd,&sz,sizeof(sz),0);

	for(int i=1;i<=sz;i++)
	{
        string group_name;
        struct group gp;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		group_name.assign(buffer);
		gp=rec_group(sockfd);
		g[group_name]=gp;

	}
	return g;

}

map<string,credentials> rec_login_info(int sockfd)
{
	map<string,credentials>mp;
	int sz;
	recv(sockfd,&sz,sizeof(sz),0);
	for(int i=1;i<=sz;i++)
	{
		string id;
		struct credentials cd;
		char buffer[20];
		recv(sockfd,buffer,sizeof(buffer),0);
		id.assign(buffer);
		cd=rec_credentials(sockfd);
		mp[id]=cd;

	}
	return mp;


}

void send_client_info(int sock_id1,struct client_info s)
{
	string ip;
	string port;
	bool is_shared;
	char buffer[20];

	ip=s.ip;
	port=s.port;
	is_shared=s.is_shared;
	memset(buffer,'\0',20);

	strcpy(buffer,ip.c_str());
    send(sock_id1,buffer,sizeof(buffer),0);
    strcpy(buffer,port.c_str());
    send(sock_id1,buffer,sizeof(buffer),0);
    send(sock_id1,&is_shared,sizeof(bool),0);

    return ;
}

void send_credentials(int sock_id1,struct credentials s)
{
	string id;
	string pwd;
	bool login_status;
	char buffer[20];

	id=s.userid;
	pwd=s.password;
	login_status=s.login_status;
	memset(buffer,'\0',20);

	strcpy(buffer,id.c_str());
    send(sock_id1,buffer,sizeof(buffer),0);
    strcpy(buffer,pwd.c_str());
    send(sock_id1,buffer,sizeof(buffer),0);
    send(sock_id1,&login_status,sizeof(bool),0);

    return ;
}


void send_group(int sock_id1,struct group s)
{
	int sz1,sz2,sz3,sz4,sz5;
	sz1=s.client_subscribed.size();
	sz2=s.file_map_clients.size();

	sz4=s.admin.size();
	sz5=s.size_info.size();

	send(sock_id1,&sz1,sizeof(int),0);//sending client_subscribed.
	for(auto it:s.client_subscribed)
	{
		string id;
		bool sub;
		id=it.first;
		sub=it.second;
		char buffer[20];
		strcpy(buffer,id.c_str());
		send(sock_id1,buffer,sizeof(buffer),0);
		send(sock_id1,&sub,sizeof(sub),0);
	}

	send(sock_id1,&sz2,sizeof(int),0);// sending file_map_clients.
	for(auto it:s.file_map_clients)
	{
		int sz3=it.second.size();
		string name;
		name=it.first;
		char buffer[20];
		strcpy(buffer,name.c_str());
		send(sock_id1,buffer,sizeof(buffer),0);
		send(sock_id1,&sz3,sizeof(int),0);

		for(auto st:it.second)
		{
           send_client_info(sock_id1,st);
		}
	}

	send(sock_id1,&sz4,sizeof(int),0);//sending admin.
	for(auto v:s.admin)
	{
      string admin=v;
      char buffer[20];
      strcpy(buffer,admin.c_str());
      send(sock_id1,buffer,sizeof(buffer),0);

	}

	send(sock_id1,&sz5,sizeof(int),0);
	for(auto pr:s.size_info)
	{
       string name=pr.first;
       int sz=pr.second;
       char buffer[20];
       strcpy(buffer,name.c_str());
       send(sock_id1,buffer,sizeof(buffer),0);
       send(sock_id1,&sz,sizeof(int),0);

	}
	return ;

}

void send_groups(int sock_id1,map<string,group>g)
{
    int sz;
    sz=g.size();
    send(sock_id1,&sz,sizeof(int),0);

    for(auto p:g)
    {
    	string group_name;
    	group_name=p.first;
    	char buffer[20];
    	strcpy(buffer,group_name.c_str());
    	send(sock_id1,buffer,sizeof(buffer),0);
    	send_group(sock_id1,p.second);

    }
	return ;

}

void send_login_info(int sock_id1,map<string,credentials> login_info)
{
	int sz;
	sz=login_info.size();
	send(sock_id1,&sz,sizeof(int),0);
	for(auto p:login_info)
	{
		string id;
		id=p.first;
		char buffer[20];
		strcpy(buffer,id.c_str());
		send(sock_id1,buffer,sizeof(buffer),0);
		send_credentials(sock_id1,p.second);
	}

	return;
}
