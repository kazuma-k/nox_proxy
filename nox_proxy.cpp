// nox_base.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>

#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")



int main(){
	WSADATA wsaData;
	struct sockaddr_in addr;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int sockaddr_size;
	SOCKET open_sock, opponent_sock, nox_sock;
	char ip_str[2][64];
	char recv_buf[10000];
	char str[INET_ADDRSTRLEN];
	char nox_ip[64];
	int nox_port;
	int port;
	
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	puts("対戦相手からの接続待ちをするポート番号:");
	scanf_s("%d", &port);
	puts("nox playerのIPアドレス:");
	scanf_s("%s", nox_ip, 64);
	puts("nox playerのポート番号:");
	scanf_s("%d", &nox_port);
	
	
	open_sock = socket(AF_INET, SOCK_STREAM, 0);
	//リッスン側のポートなど情報
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	sockaddr_size = sizeof(client);
	//androidエミュレータ(nox)のIP,ポートを設定。
	server.sin_family = AF_INET;
	server.sin_port = htons(nox_port);
	inet_pton(AF_INET, nox_ip, &server.sin_addr);

	bind(open_sock, (struct sockaddr *)&addr, sizeof(addr));
	listen(open_sock, 5);
	int len;
	printf("ポート[%d]で、対戦相手からの接続待機中...\n", port, WSAGetLastError());
	
	
	inet_ntop(server.sin_family, &server.sin_addr, str, sizeof(str));
	strcpy_s(ip_str[0], str);

	int client_len = sizeof(client);
	//対戦相手からの接続承認ループ
	while((opponent_sock = accept(open_sock, (struct sockaddr *)&client, &sockaddr_size))) {
		//対戦相手が接続。
		printf("対戦相手の[%s]が接続してきました。\n", inet_ntop(client.sin_family, &client.sin_addr, str, sizeof(str)));
		strcpy_s(ip_str[1], str);
		nox_sock = socket(AF_INET, SOCK_STREAM, 0);
		//
		if (connect(nox_sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
			printf("Nox[%s]へ接続しました。パケットの中継を開始。\n", ip_str[0]);
			u_long val = 1;
			//ノンブロッキングソケットに設定
			ioctlsocket(opponent_sock, FIONBIO, &val);
			ioctlsocket(nox_sock, FIONBIO, &val);
			while (true) {
				//①対戦相手からのパケット受信
				if ((len = recv(opponent_sock, recv_buf, sizeof(recv_buf), 0)) > 0) {
					//対戦相手 → nox へパケット中継
					send(nox_sock, recv_buf, len, 0);
				} else if(WSAGetLastError() != WSAEWOULDBLOCK) {
					//対戦相手からの接続が切れた時。2つのソケットを閉じる。
					closesocket(opponent_sock);
					closesocket(nox_sock);
					break;
				}
				//②noxからのパケット受信
				if ((len = recv(nox_sock, recv_buf, sizeof(recv_buf), 0)) > 0) {
					//nox → 対戦相手 へパケット中継
					send(opponent_sock, recv_buf, len, 0);
 				} else if (WSAGetLastError() != WSAEWOULDBLOCK) {
					//noxからの接続が切れた時。2つのソケットを閉じる。
					closesocket(opponent_sock);
					closesocket(nox_sock);
					break;
				}
				Sleep(1);
			}
		} else {
			puts("Noxへの接続を失敗。\n");
		}
	}
	WSACleanup();
    return 0;
}

