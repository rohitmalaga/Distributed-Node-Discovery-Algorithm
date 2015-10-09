#include "broadcast.h"

bool spanning,spDone,bdDone,parentSet = false;
int thisNode ;
int broadcastParent = -1;
int value = 0;
const char * logFileName;

vector<int> spanList,templist;
vector<string> lines;

struct fileInput {
	int nodePort[50];
	string nodeHostname[50];
	string nodeHostList[50];
	int broadcastNode;
}record;

void* SocketHandler(void*);
void* ClientHandler (void*);
void* BroadcastHandler (void*);
void server(int currentNodeIs,int totalNodes,fstream& fs);
int client(int nodePort,const string nodeHost,int currentNodeIs, int msgType);
vector<string> split(const string& s, const string& delim);
bool readFile(string filename);

/*
 * Function: readFile
 * ----------------------------
 *   Reads the data from file line by line
 *
 *   filename: The name of the file to be opened 
 *
 *   returns: true/false 
 */
bool readFile(string filename){
    ifstream file;
    string line;

    file.open(filename.c_str());

    if(!file.is_open()){
        return false;
    }

    while (getline(file, line)) {
        lines.push_back(line);
    }

    return true;
}
 /*
 * Function: split
 * ----------------------------
 *  This function splits the string using a delimiter and stores the values in vector 
 *
 *   @s: string which is to be split,
 *   @delim : the delimiter used in string,
 * 
 *   returns: the vector which contains the split strings. 
 */

vector<string> split(const string& s, const string& delim) {
    vector<string> result;
	const bool keep_empty = true;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
        string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}


int main(int argv, char* argc[]){

    int totalNoNodes;
    thisNode = atoi(argc[1]); /*Need to be careful while writing config file, always nodes must start from 0 to the number of nodes*/

	const char * str1 = "--Start of Node Info--";
	const char * str2 = "--Broadcast Node--";
	int  nodePorts[100];
	int x=0;
	vector<string> nodeHostnames,nodenumbers;
	
	if (argv != 2 && ((sizeof record.nodePort[0]/sizeof(int))>= thisNode)){
        cerr << "Please enter the node number of the machine you want to run" << endl;
        return 0;
    }
	if(!readFile("config_rohit.txt")){
	    cerr << "Unable to open the config file" << endl;
        return 0;
	}
	
	
	totalNoNodes = atoi(lines[0].c_str());
	
    for(int i = 0; i < lines.size(); ++i){
		if(lines[i]==str1){
			const vector<string> words = split(lines[i+2], ";");
			const vector<string> host = split(lines[i+1], ";");
			const vector<string> nodes = split(lines[i+3], ";");
			for (std::vector<string>::const_iterator j = words.begin(),k=host.begin(),l=nodes.begin(); j != words.end(),k!=host.end(),l!=nodes.end(); ++j,++k,++l,x++){
			   nodePorts[x] = atoi(j->c_str());
			   nodeHostnames.push_back(k->c_str());
			   nodenumbers.push_back(l->c_str());
			}
		}

		if((lines[i]==str2)){
			record.broadcastNode= atoi(lines[i+1].c_str());
		}
    }
	for(int i=0;i<totalNoNodes;i++){
		record.nodePort[i] = nodePorts[i];
		record.nodeHostname[i] =nodeHostnames[i];
		record.nodeHostList[i]=nodenumbers[i];
	}

	logFileName = record.nodeHostname[thisNode].c_str();

	fstream fs;
	fs.open (logFileName, std::fstream::in | std::fstream::out | std::fstream::app);	
	
	cout << "totalNoNodes" << totalNoNodes<<endl;
	fs << "totalNoNodes" << totalNoNodes<<endl;
	cout <<" broadcast node is "<< record.broadcastNode <<endl;
	fs <<" broadcast node is "<< record.broadcastNode <<endl;

	server(thisNode,totalNoNodes,fs);
	fs.close();
	return 0;
}

 /*
 * Function: server
 * ----------------------------
 *   function to works as server and listens all the requests that are sent to it.
 *   @currentNodeIs: The number of the current node,
 *   @totalNodes : The total number of nodes,
 *
 *   returns: none 
 */
 
void server(int currentNodeIs,int totalNodes,fstream& fs){
   
    struct sockaddr_in my_addr;

    int hsock;
    int * p_int ;
    int err;
	bool sptreeDisv = false;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;
    pthread_t thread_id1=0;
	pthread_t thread_id2=0;
	
    int val,noOfNeibours=0,intValue;
	int nodeNbour[totalNodes];// total node size is declared because, the neighbour nodes may not exceed this. 
	istringstream iss(record.nodeHostList[currentNodeIs]);
	

    while (iss >> val){ 
       stringstream strValue;
       strValue << val;
	   strValue >> intValue;
	   nodeNbour[noOfNeibours]= intValue;
	   noOfNeibours++;
    }
	if (!fs.is_open())
		fs.open (logFileName, std::fstream::in | std::fstream::out | std::fstream::app);
		

	cout << "----------------------------------------------------------" <<endl;
	fs << "----------------------------------------------------------" <<endl;
	cout<<"The Port is (server)= "<< record.nodePort[currentNodeIs]<< endl;
	fs<<"The Port is (server)= "<< record.nodePort[currentNodeIs]<< endl;
	cout<<"The neighbour nodes are(server) ="<< record.nodeHostList[currentNodeIs]<< endl;
	fs<<"The neighbour nodes are(server) ="<< record.nodeHostList[currentNodeIs]<< endl;
	cout<<"The host-name is (server) = "<<record.nodeHostname[currentNodeIs]<< endl;
	fs<<"The host-name is (server) = "<<record.nodeHostname[currentNodeIs]<< endl;
	cout << "----------------------------------------------------------" <<endl <<endl;
	fs << "----------------------------------------------------------" <<endl <<endl;
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        cout <<"Error initializing socket"<< endl<< "Error Number is = "<< errno <<endl;
        fs <<"Error initializing socket"<< endl<< "Error Number is = "<< errno <<endl;
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        cout <<"Error setting options"<< endl<< "Error Number is = "<< errno <<endl;
        fs <<"Error setting options"<< endl<< "Error Number is = "<< errno <<endl;
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(record.nodePort[currentNodeIs]);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;
    
    if( bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port\n Error Number is = %d\n",errno);
        goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening\n Error Number is = %d\n",errno);
        goto FINISH;
    }
    addr_size = sizeof(sockaddr_in);

    while(true){
		cout << "----------------------------------------------------------" <<endl;
		fs << "----------------------------------------------------------" <<endl;
		cout << endl<<"Waiting for a connection"<< endl;
		fs << endl<<"Waiting for a connection"<< endl;
		if(spanList.size()!= 0){
			for(int j=0;j<spanList.size();j++){
				cout<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
				fs<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
			}
		}
		if(((!sptreeDisv) & (currentNodeIs == 0) &(!spDone))){ //Always the first node will initializes the spanning tree
		    cout << "Going to sleep for a while, so that all node become active meanwhile"<< endl;
		    fs << "Going to sleep for a while, so that all node become active meanwhile"<< endl;
			sleep(10);
			sptreeDisv = true;
			spanning = true;
			for (int i=0;i<= (noOfNeibours-1);i++){
				cout << "----------------------------------------------------------"<<endl;
				fs << "----------------------------------------------------------"<<endl;
				cout << "Messaging to node[number] = " <<nodeNbour[i]<< endl;
				fs << "Messaging to node[number] = " <<nodeNbour[i]<< endl;
				cout << "Messaging to node[Host]:[port] = " <<record.nodeHostname[nodeNbour[i]]<<":"<<record.nodePort[nodeNbour[i]]<< endl;
				fs << "Messaging to node[Host]:[port] = " <<record.nodeHostname[nodeNbour[i]]<<":"<<record.nodePort[nodeNbour[i]]<< endl;
				pthread_create(&thread_id1,0,&ClientHandler,(void *)&nodeNbour[i]);
				sleep(1);
				pthread_detach(thread_id1);
				cout << "The Number of neighbours are = "<<spanList.size() <<endl; 
				fs << "The Number of neighbours are = "<<spanList.size() <<endl; 
				}
			spDone = true;
			cout << "----------------------------------------------------------" <<endl <<endl;
			fs << "----------------------------------------------------------" <<endl <<endl;
		}

		if((spanning)&(currentNodeIs != 0) &(!spDone)){
			for (int i=0;i<= (noOfNeibours-1);i++){
				cout << "----------------------------------------------------------"<<endl;
				fs << "----------------------------------------------------------"<<endl;
				cout << "Messaging to node[number] = " <<nodeNbour[i]<< endl;
				fs << "Messaging to node[number] = " <<nodeNbour[i]<< endl;
				cout << "Messaging to node[Host]:[port] = " <<record.nodeHostname[nodeNbour[i]]<<":"<<record.nodePort[nodeNbour[i]]<< endl;
				fs << "Messaging to node[Host]:[port] = " <<record.nodeHostname[nodeNbour[i]]<<":"<<record.nodePort[nodeNbour[i]]<< endl;
				
				pthread_create(&thread_id1,0,&ClientHandler,(void *)&nodeNbour[i]);
				sleep(1);
				pthread_detach(thread_id1);
				cout << "The Number of neighbours are = "<<spanList.size() <<endl;
				fs << "The Number of neighbours are = "<<spanList.size() <<endl;
			}
			spDone = true;
		}
		if((spDone) &(thisNode == record.broadcastNode) &(!bdDone)){
			cout << "-----------------------------------------------------"<<endl;
			fs << "-----------------------------------------------------"<<endl;
			cout << " we are going to start the broadcast functionality  "<<endl;
			fs << " we are going to start the broadcast functionality  "<<endl;
			cout << "-----------------------------------------------------"<<endl;
			fs << "-----------------------------------------------------"<<endl;
			if(spanList.size()!= 0){
				for(int j=0;j<spanList.size();j++){
					cout<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
					fs<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
					templist.push_back(spanList[j]);
					pthread_create(&thread_id2,0,&BroadcastHandler,(void *)&spanList[j]);
					sleep(1);
					pthread_detach(thread_id2);
				}
			}
			bdDone = true;
		}
		if((spDone) &(parentSet)&(!(thisNode == record.broadcastNode))&(!bdDone)){
			cout << "-----------------------------------------------------"<<endl;
			fs << "-----------------------------------------------------"<<endl;
			cout << " we are going to start the broadcast functionality 2  "<<endl;
			fs << " we are going to start the broadcast functionality 2  "<<endl;
			cout << "-----------------------------------------------------"<<endl;
			fs << "-----------------------------------------------------"<<endl;
			if(spanList.size()!= 0){
				for(int j=0;j<spanList.size();j++){
					cout<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
					fs<< "++++the neighbour is+++++++ = "<< spanList[j]<<endl;
					if(spanList[j]!= broadcastParent)
						templist.push_back(spanList[j]);
					else{
						cout<< "++++the parent is+++++++ = "<< spanList[j]<<endl;
						fs<< "++++the parent is+++++++ = "<< spanList[j]<<endl;
						}
					if(templist.size()!= 0){
						pthread_create(&thread_id2,0,&BroadcastHandler,(void *)&spanList[j]);
						sleep(1);
						pthread_detach(thread_id2);
					}
				}
			}
			bdDone = true;
		}
		if(((parentSet) ||(thisNode == record.broadcastNode) )&((templist.empty()) || (value == templist.size())) &(spanning)){
			cout << "THE PARENT IS  "<< broadcastParent << endl;
			fs << "THE PARENT IS  "<< broadcastParent << endl;
			if (broadcastParent!= -1){
				client(record.nodePort[broadcastParent],record.nodeHostname[broadcastParent],broadcastParent,2);
				}
			parentSet = false;
			cout << "THE BROADCAST IS FINISH ON THIS NODE "<< value<<endl;
			fs << "THE BROADCAST IS FINISH ON THIS NODE "<< value<<endl;
		}
		
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1){
            cout << "#################################################"<< endl<< "Received connection from:" <<inet_ntoa(sadr.sin_addr)<< endl;
            fs << "#################################################"<< endl<< "Received connection from:" <<inet_ntoa(sadr.sin_addr)<< endl;
            pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
			sleep(1);
            pthread_detach(thread_id);
        }
        else{
            fprintf(stderr, "Error accepting %d\n", errno);
        }
    }
    fs.close();
FINISH:
   free(csock);
   return;
}

 /*
 * Function: BroadcastHandler
 * ----------------------------
 *   This function take cares of broadcasting the broadcast message to all its neighbours.
 *   @nodeIs: The pointer to the node/host values.
 *
 *   returns: none 
 */
void* BroadcastHandler(void* nodeIs){

	int node = *((int *) nodeIs);
	int returnNode;
	returnNode = client(record.nodePort[node],record.nodeHostname[node],node,1);
}

 /*
 * Function: ClientHandler
 * ----------------------------
 *   This function take cares of the spanning tree message to all its neighbours.
 *   @nodeIs: The pointer to the node/host values.
 *
 *   returns: none 
 */
void* ClientHandler(void* nodeIs){

	int node = *((int *) nodeIs);
	int somevalue;
	
	cout << "The node to which message has to be sent is  = "<< node<<endl;
	somevalue = client(record.nodePort[node],record.nodeHostname[node],node,0);
	cout << "+++++++++++++++++++++ Neighbouring Node is : "<< somevalue<<"  +++++++++++++++++++++++++++++"<< endl;
	if(somevalue != -1)
		spanList.push_back(somevalue);
}

 /*
 * Function: SocketHandler
 * ----------------------------
 *   This function take cares of the all request that are on server, so that server can be ready for any incoming connections.
 *   @lp: The pointer to the socket buffer message.
 *
 *   returns: none 
 */
void* SocketHandler(void* lp){
    int *csock = (int*)lp;

    char buffer[100];
    int buffer_len = 100;
    int bytecount;
	char * pch;
	int clientNode;
	int msgIs = 0; // 0 spanning, 1 broadcast
	fstream fs;
	
	if (!fs.is_open())
	fs.open (logFileName, std::fstream::in | std::fstream::out | std::fstream::app);

    memset(buffer, 0, buffer_len);
    if((bytecount = recv(*csock, buffer, buffer_len, 0))== -1){
        fprintf(stderr, "Error receiving data\n Error is = %d\n", errno);
        goto FINISH;
    }
	
	pch = strtok (buffer,"-");
	clientNode = atoi(pch);
	pch = strtok (NULL, "-");
	strcpy(buffer,pch);

	if(strcmp(buffer,"SPANNING TREE MESSAGE")== 0){
		cout << "Its a SPANNING TREE MESSAGE" << endl;
		fs << "Its a SPANNING TREE MESSAGE" << endl;
		}
	if(strcmp(buffer,"BROADCAST MESSAGE")== 0){
		cout << "Its a BROADCAST MESSAGE" << endl;
		fs << "Its a BROADCAST MESSAGE" << endl;
		msgIs = 1;
		broadcastParent = clientNode;
		}
	if((strcmp(buffer,"BACK")== 0)||(strcmp(buffer,"BKCK")==0)){
		cout << "Its a BROADCAST ACKNOLEGMENT" << endl;
		fs << "Its a BROADCAST ACKNOLEGMENT" << endl;
		msgIs = 2;
		value++;
	}
	
	if(spanning)
		strcpy(buffer, "NACK");
	else{
        strcpy(buffer, "ACKM");
		spanning = true;
		spanList.push_back(clientNode);
	}
	cout << "The client node from which the request has come = "<<clientNode << endl;
	cout << "The value is = "<<value << endl;
	cout << "The msgIs is = "<<msgIs << endl;
	fs << "The client node from which the request has come = "<<clientNode << endl;
	
	if(spDone & msgIs &(!parentSet)){
		parentSet = true;
	}
	cout <<"Received bytes from client(server): "<< bytecount << endl << "Received message from client(server): "<<buffer<< endl;
	fs <<"Received bytes from client(server): "<< bytecount << endl << "Received message from client(server): "<<buffer<< endl;

    cout << "The Server message to client is :" << buffer << endl;
    fs << "The Server message to client is :" << buffer << endl;
	if(msgIs == 0){
		if((bytecount = send(*csock, buffer, strlen(buffer), 0))== -1){
			fprintf(stderr, "Error sending data %d\n", errno);
			goto FINISH;
		}
	}
	 fs.close();
	FINISH:
    free(csock);
    return 0;
}
 /*
 * Function: client
 * ----------------------------
 *   This function sends the message to the assigned hostname to its port.
 *   @nodePort: The port number to which the message has to be send.
 *   @nodeHost: The host address to which the message has to be send.
 *   @currentNodeIs: The node number of the host to which the message going to reach.
 *   @msgType: 1= broadcase, 0 = spanning tree message
 *
 *   returns: its node number. 
 */

int client(int nodePort,const string nodeHost,int currentNodeIs, int msgType){

    const char* host_name= nodeHost.c_str();
    struct sockaddr_in my_addr;

    char buffer[] = "SPANNING TREE MESSAGE";
    char buffer_0[] = "BROADCAST MESSAGE";
    char nack[] = "NACK";
    char ack[] = "ACKM";
    char back[] = "BACK";
	char rcv_buffer[5];

    int bytecount;
    int buffer_len=0,rcv_buffer_len=0;

    int hsock;
    int * p_int;
    int err;
	fstream fs;
	
	if (!fs.is_open())
	fs.open (logFileName, std::fstream::in | std::fstream::out | std::fstream::app);
	
	cout<<"The Message is to(client): "<< currentNodeIs << endl;
	fs<<"The Message is to(client): "<< currentNodeIs << endl;
	cout<<"The Message is sent to Host:Port:(client) "<<nodeHost<<":"<<nodePort<< endl;
	fs<<"The Message is sent to Host:Port:(client) "<<nodeHost<<":"<<nodePort<< endl;
	
	if(msgType == 1){
		cout << "This is broadcast message(received on server)"<< endl;
		fs << "This is broadcast message(received on server)"<< endl;
		}
	else if(msgType == 2){
		cout << "This is broadcast finish message(received on server)"<< endl;
		fs << "This is broadcast finish message(received on server)"<< endl;
		}
	else{
		cout << "This is spanning tree message(received on server)"<< endl;
		fs << "This is spanning tree message(received on server)"<< endl;
		}

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        cout << "Error initializing socket"<<endl<< "Error is = "<< errno<< endl;
        fs << "Error initializing socket"<<endl<< "Error is = "<< errno<< endl;
        goto FINISH;
    }
   
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        cout << "Error setting options"<<endl<<" Error Number is = "<<errno<<endl;
        fs << "Error setting options"<<endl<<" Error Number is = "<<errno<<endl;
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(nodePort);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name);

    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket\n Error number is = %d\n", errno);
            goto FINISH;
        }
    }
	
	if(msgType == 1)
	    strcpy(buffer,buffer_0);
	else if(msgType == 2)
		strcpy(buffer,back);
	if(true){
		std::stringstream sstm;
		sstm << thisNode << "-" << buffer;
		strcpy(buffer,sstm.str().c_str());
	}

    buffer_len = strlen(buffer);
    buffer[strlen(buffer)]='\0';
	
	cout <<" The message send from this node is: "<< buffer<<endl;
	fs <<" The message send from this node is: "<< buffer<<endl;
    
    if( (bytecount=send(hsock, buffer, strlen(buffer),0))== -1){
        fprintf(stderr, "Error sending data\n Error Number is = %d\n", errno);
        goto FINISH;
    }

	if(msgType == 0){
		if((bytecount = recv(hsock, rcv_buffer, 4, 0))== -1){
			fprintf(stderr, "Error receiving data\n Error Number is = %d\n", errno);
			goto FINISH;
		}
		cout << "THE MESSAGE is : "<< rcv_buffer<< endl;
		fs << "THE MESSAGE is : "<< rcv_buffer<< endl;
		if (strcmp(rcv_buffer,ack)== 0){
		   cout<< "*****Its an ACKNOLEGMENT*****"<< endl;
		   fs<< "*****Its an ACKNOLEGMENT*****"<< endl;
		   }
		else if(strcmp(rcv_buffer,nack)== 0){
			cout << "*****Its a NEGATIVE ACKNOLEGMENT*****"<< endl;
			fs << "*****Its a NEGATIVE ACKNOLEGMENT*****"<< endl;
			}
		else{
			cout <<"Received bytes(client):" << bytecount<< endl<< " , Received string(client): "<<buffer<< endl;
			fs <<"Received bytes(client):" << bytecount<< endl<< " , Received string(client): "<<buffer<< endl;
			}
	}
    close(hsock);
	  fs.close();
    if ((strcmp(rcv_buffer,ack)== 0) || (msgType == 1))
		return currentNodeIs;
	else
		return -1;
	
FINISH:
   close(hsock);
return -1;
;
}
