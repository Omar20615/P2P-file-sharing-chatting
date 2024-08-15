#include <iostream>
#include <string.h>
#include<string>
#include <sstream>
#include<thread>
#include<winsock.h>
#include<filesystem>
#include<map>
#include<conio.h>
#include <fstream>




void uploadFile(int connfd);
void listenPeers(int portnumber);
void out(int connfd);
void in(int connfd);
void in_Peer(int connfd);
void out_Peer(int connfd);
void downloadFile(int connfd);



#define SERVER_PORT_NO 80
int thisClientPortNumber;
using namespace std;
namespace fs = std::filesystem;
bool request_client = true;
//bool downloadFile = false;
int choice = 2;
string fname;






class fileData {
public:
	int clientPortNumbers;
	int clientFD;
	string clientIPaddr;

public:
	fileData() {

		clientPortNumbers = NULL;
		clientFD = NULL;
		//clientIPaddr = NULL;
	}
	fileData(int pNum, int cFD, char* cIP)
	{
		clientPortNumbers = pNum;
		clientFD = cFD;
		clientIPaddr = cIP;

	}

	friend ostream& operator << (ostream& out, const fileData& obj);
};
ostream& operator << (ostream& out, const fileData& obj)
{
	out << "\t\t" << obj.clientPortNumbers << "\t\t" << obj.clientFD << "\t\t\t" << obj.clientIPaddr << "\t\t";
	return out;
}

multimap<string, fileData> FileData;
multimap<string, fileData>::iterator itr;


void convert_string_to_client_info(string str) {
	fileData client;
	istringstream iss(str);
	string filename;
	//cout << "Str : " << str << "\n";
	iss.ignore();
	string x;
	getline(iss, filename, ' ');

	iss >> client.clientPortNumbers; iss.ignore();

	iss >> client.clientFD; iss.ignore();

	getline(iss, client.clientIPaddr, ' ');

	FileData.insert(pair<string, fileData>(filename, client));
}
void fetchPortnumber(int connfd) {
	string x = "Send port number";
	char buff[100] = { 0 };
	send(connfd, x.c_str(), x.length(), 0);
	recv(connfd, buff, 100, 0);
	thisClientPortNumber = atoi(buff);
	cout << buff << "\n";
}
void showData() {

	int x = 1;
	cout << "Sr No.\tKEY\t\t\t\tPort Number\tFile Descriptor\t\tIP address\n";
	for (itr = FileData.begin(); itr != FileData.end(); ++itr) {
		cout << x << '\t' << itr->first << itr->second << "\n";
		x++;
	}
}
void createPeer() {

	showData();
	int choice1;
	if (choice == 2) {
		cout << "Select the peer with whom you want to chat : ";
	}
	else if (choice == 1) {

		cout << "Enter the number of device to download desired file : ";
	}
	cin >> choice1;

	//downloadFile = true;
	int x = 1;
	itr = FileData.begin();
	for (int x = 1; x < choice1; x++) {
		itr++;

	}

	cout << itr->first << itr->second << "\n";
	fname = itr->first;

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		//return -1;
	}


	struct sockaddr_in addr;

	addr.sin_addr.s_addr = inet_addr((itr->second.clientIPaddr).c_str());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(itr->second.clientPortNumbers);
	memset(&(addr.sin_zero), 0, 8);

	if (connect(fd, (struct sockaddr*) & addr, sizeof(addr)) == -1) {
		perror("Socket Connect failed\n");
		//return -1;
	}



	if (choice == 2) {

		thread T_out(out_Peer, fd);
		thread T_in(in_Peer, fd);

		T_in.join();
		T_out.join();

	}
	else if (choice == 1) {
		thread T_in(downloadFile, fd);

		T_in.join();
	}

	closesocket(fd);
}

void in(int connfd)
{
	fetchPortnumber(connfd);
	thread lisPeers(listenPeers, thisClientPortNumber);
	lisPeers.detach();
	while (1) {
		char buffer[1000] = { 0 };

		recv(connfd, buffer, 1000, 0);
		string x(buffer);

		if (request_client == true) {
			int check = 0;
			FileData.clear();
			while (1) {
				if (check == 0) {
					convert_string_to_client_info(x);
					check++;
				}
				else {


					char buffer[1000] = { 0 };
					recv(connfd, buffer, 1000, 0);

					string x(buffer);
					if (!(strcmp(x.c_str(), "break"))) {
						break;
					}
					convert_string_to_client_info(x);
				}
			}

			request_client = false;

			createPeer();

		}
		else {
			cout << "From Server: " << buffer << endl;
			if (!strcmp(buffer, "e")) break;
		}
	}
}


void out(int connfd) {
	while (1) {
		char buffer[100];

		string data;

		string sample;

		getline(cin, data);

		if (!(strcmp(data.c_str(), "Send files"))) {


			string filenames[100];
			int value = 0;

			string path = ".\\Files\\";
			cout << "Here are the all files present in directory\n";
			for (const auto& entry : fs::directory_iterator(path)) {

				//cout << entry.path().filename() << endl;
				sample += entry.path().filename().string();
				filenames[value] = ".\\Files\\" + sample;
				sample = "";
				cout << filenames[value] << "\n";
				send(connfd, ("file:: " + filenames[value]).c_str(), ("file:: " + filenames[value]).length(), 0);
				value++;

			}

		}
		else if (!(strcmp(data.c_str(), "Send data"))) {

			request_client = true;
		}
		else {


			send(connfd, data.c_str(), data.length(), 0);
			cout << "To Server: " << data << endl;
			if (!strcmp(buffer, "e"))break;
		}
	}
}





void listenPeers(int portnumber) {

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");

	}


	struct sockaddr_in addr;

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portnumber);

	if (bind(fd, (struct sockaddr*) & addr, sizeof(addr)) == -1) {
		perror("Bind failed on socket\n");

	}


	int backlog = 10;
	if (listen(fd, backlog) == -1) {
		perror("Listen Failed on server: \n");

	}


	int connfd;
	struct sockaddr_in cliaddr;
	int cliaddr_len = sizeof(struct sockaddr);


	while (1) {

		connfd = accept(fd, (struct sockaddr*) & cliaddr, &cliaddr_len);
		char* IPaddr = inet_ntoa(cliaddr.sin_addr);


		if (connfd <= 0) {
			perror("accept failed on socket: \n");
		}


		if (choice == 2) {

			thread T_out(out_Peer, connfd);
			thread T_in(in_Peer, connfd);

			T_in.join();
			T_out.join();

		}
		else if (choice == 1) {
			thread T_out(uploadFile, connfd);
			T_out.join();
		}


	}

	closesocket(fd);
}


void in_Peer(int connfd)
{

	while (1) {
		char buffer[100] = { 0 };


		recv(connfd, buffer, 100, 0);
		string dat(buffer);

		cout << "From Peer: " << buffer << endl;
		if (!strcmp(buffer, "e")) break;
	}
}

void out_Peer(int connfd) {
	while (1) {
		char buffer[100] = { 0 };
		cin.getline(buffer, 100);


		send(connfd, buffer, strlen(buffer), 0);
		cout << "To Peer: " << buffer << endl;
		if (!strcmp(buffer, "e"))break;
	}
}

void downloadFile(int connfd) {

	send(connfd, fname.c_str(), fname.length(), 0);

	string filename2 = ".\\Output.txt";
	fstream MyWriteFile;
	MyWriteFile.open(filename2.c_str(), ios::out);
	while (1) {
		char buffer[500] = { 0 };
		recv(connfd, buffer, 500, 0);
		string myText(buffer);
		//cout << myText << "\n";
		if (!(strcmp(myText.c_str(), "stop"))) {
			cout << "File successfully downloaded";
			break;
		}
		MyWriteFile << myText << "\n";

	}
	MyWriteFile.close();

}
void uploadFile(int connfd) {

	char buffer[500] = { 0 };
	recv(connfd, buffer, 500, 0);

	string filename(buffer);
	ifstream MyReadFile;
	string mydata;
	MyReadFile.open(filename.c_str());
	while (getline(MyReadFile, mydata)) {

		//cout << mydata << "\n";
		send(connfd, mydata.c_str(), mydata.length(), 0);
		Sleep(500);

	}
	mydata = "stop";
	send(connfd, mydata.c_str(), mydata.length(), 0);
	cout << "File sent\n";
	MyReadFile.close();

}










int main() {
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

	cout << "Select the choice\n1->Send files\n2->Chat with a peer\n";
	cin >> choice;





	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("Socket Creation failed\n");
		return -1;
	}


	struct sockaddr_in sa_addr;

	sa_addr.sin_family = AF_INET;
	sa_addr.sin_port = htons(80);
	sa_addr.sin_addr.s_addr = inet_addr("127.0.0.1");			//Assigning local address of the machine to the server
	memset(&(sa_addr.sin_zero), 0, 8);					//8 is size of sin_zero and 0 is the value that we are setting to it


	if (connect(fd, (struct sockaddr*) & sa_addr, sizeof(sa_addr)) == -1) {
		perror("Socket Connect failed\n");
		return -1;
	}




	thread T_out(out, fd);
	thread T_in(in, fd);

	T_in.join();
	T_out.join();


	closesocket(fd);
	//close(fd);
	return 0;
}

