#include <iostream>
#include <thread>
#include<string>
#include<winsock.h>
#include <iterator>
#include <map>
#include<windows.h>
#include <sstream>

#define SERVER_PORT_NO 80
#define maxClient 80

using namespace std;

class fileData {
public:
	int clientPortNumbers;
	int clientFD;
	char* clientIPaddr ;

public:
	fileData() {
		
		clientPortNumbers = NULL;
		clientFD = NULL;
		clientIPaddr = NULL;
	}
	fileData(int pNum, int cFD, char* cIP)
	{
		clientPortNumbers = pNum;
		clientFD = cFD;
		clientIPaddr = cIP;

	}

	friend ostream& operator << (ostream& out, const fileData &obj);
};

ostream& operator << (ostream& out, const fileData &obj)
{
	out << "\t\t" << obj.clientPortNumbers << "\t\t" << obj.clientFD << "\t\t\t" << obj.clientIPaddr << "\t\t";
	return out;
}

multimap<string, fileData> FileData;
multimap<string, fileData>::iterator itr;

string convert_client_info_to_string(fileData client, string fileName) {
	ostringstream oss;
	oss <<fileName<<" "<< client.clientPortNumbers << " " << client.clientFD << " " << client.clientIPaddr << " ";
	return oss.str();
}

void showData(int connfd) {
	
	int x = 1;
	cout << "Sr No.\tKEY\t\t\t\tPort Number\tFile Descriptor\t\tIP address\n";
	for (itr = FileData.begin(); itr != FileData.end(); ++itr) {

		string cdata = convert_client_info_to_string(itr->second,itr->first);
		if (itr->second.clientFD != connfd) {
			send(connfd, cdata.c_str(), cdata.length(), 0);
		}
		//Sleep(5000);
		cout <<x <<'\t' << itr->first << itr->second << "\n";
		x++;
		
	}if (itr == FileData.end()) {

		string b = "break";
		send(connfd, b.c_str(), b.length(), 0);
	}
}

int detect_code(string x)
{

	int code;
	char part[30];
	stringstream s;

	s << x;

	s.getline(part, 30, ' ');

	if (strcmp(part, "file::") == 0) {
		code = 1;
	}
	else if (strcmp(part, "/exit") == 0)
	{
		code = 2;
	}
	else
	{
		code = 0;
	}
	return code;
}



void in(int connfd, fileData obj)
{
	while (1) {
		char buffer[255] = { 0 };
	
		recv(connfd, buffer, 255, 0);
		string x(buffer);
		cout << x << "\n";
		if (detect_code(x) == 1) {
			int len = x.length();
			len = len - 6;
			x.erase(x.begin()+0, x.end() - len);
			FileData.insert(pair<string, fileData>(x, obj));
		}
		else if (!(strcmp(x.c_str(), "Send port number"))) {
			string str = to_string(obj.clientPortNumbers);
			send(connfd, str.c_str(), str.length(), 0);
		}
		else if (!(strcmp(x.c_str(), "Show data")))
		{
			showData(connfd);
		}
		else {


		cout << "From Client: " << x << endl;
		if (!strcmp(buffer, "e")) break;
		}
	}
}





int main() {
	int cfd[100];
	char* cIPaddr[100];
	int iterator = 0;
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << "WSA failed to initialize\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else {
		cout << "Socket initialized\n";
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		return -1;
	}


	struct sockaddr_in addr;

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT_NO);

	if (bind(fd, (struct sockaddr*) & addr, sizeof(addr)) == -1) {
		perror("Bind failed on socket\n");
		return -1;
	}


	int backlog = 10;
	if (listen(fd, backlog) == -1) {
		perror("Listen Failed on server: \n");
		return -1;
	}


	int connfd;
	struct sockaddr_in cliaddr;
	int cliaddr_len = sizeof(struct sockaddr);
	

	while (1) {
		
		connfd = accept(fd, (struct sockaddr*) & cliaddr, &cliaddr_len);
		char* IPaddr = inet_ntoa(cliaddr.sin_addr);

		iterator++;

		if (connfd <= 0) {
			perror("accept failed on socket: \n");

		}

		
		thread T_in(in, connfd, fileData(cliaddr.sin_port, connfd, IPaddr));
		
		T_in.detach();
	}

	closesocket(fd);
	//close(connfd);
	return 0;
}