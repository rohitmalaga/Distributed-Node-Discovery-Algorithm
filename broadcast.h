#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <vector>
#include <sstream>
using namespace std;

void* SocketHandler(void*);
void* ClientHandler (void*);
void* BroadcastHandler (void*);
void server(int currentNodeIs,int totalNodes,fstream& fs);
int client(int nodePort,const string nodeHost,int currentNodeIs, int msgType);
vector<string> split(const string& s, const string& delim);
bool readFile(string filename);