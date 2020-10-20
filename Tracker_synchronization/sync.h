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

using namespace std;

struct client_info rec_client_info(int sockfd);
struct credentials rec_credentials(int sockfd);
struct group rec_group(int sockfd);
map<string,group> rec_groups(int sockfd);
map<string,credentials> rec_login_info(int sockfd);

void send_client_info(int sock_id1,struct client_info s);
void send_credentials(int sock_id1,struct credentials s);
void send_group(int sock_id1,struct group s);
void send_groups(int sock_id1,map<string,group>g);
void send_login_info(int sock_id1,map<string,credentials> login_info);

void sync_tracker_rec(int sockfd);
void sync_tracker_send(int sock_id1);

