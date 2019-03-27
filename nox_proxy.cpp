// nox_base.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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

	puts("�ΐ푊�肩��̐ڑ��҂�������|�[�g�ԍ�:");
	scanf_s("%d", &port);
	puts("nox player��IP�A�h���X:");
	scanf_s("%s", nox_ip, 64);
	puts("nox player�̃|�[�g�ԍ�:");
	scanf_s("%d", &nox_port);
	
	
	open_sock = socket(AF_INET, SOCK_STREAM, 0);
	//���b�X�����̃|�[�g�ȂǏ��
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	sockaddr_size = sizeof(client);
	//android�G�~�����[�^(nox)��IP,�|�[�g��ݒ�B
	server.sin_family = AF_INET;
	server.sin_port = htons(nox_port);
	inet_pton(AF_INET, nox_ip, &server.sin_addr);

	bind(open_sock, (struct sockaddr *)&addr, sizeof(addr));
	listen(open_sock, 5);
	int len;
	printf("�|�[�g[%d]�ŁA�ΐ푊�肩��̐ڑ��ҋ@��...\n", port, WSAGetLastError());
	
	
	inet_ntop(server.sin_family, &server.sin_addr, str, sizeof(str));
	strcpy_s(ip_str[0], str);

	int client_len = sizeof(client);
	//�ΐ푊�肩��̐ڑ����F���[�v
	while((opponent_sock = accept(open_sock, (struct sockaddr *)&client, &sockaddr_size))) {
		//�ΐ푊�肪�ڑ��B
		printf("�ΐ푊���[%s]���ڑ����Ă��܂����B\n", inet_ntop(client.sin_family, &client.sin_addr, str, sizeof(str)));
		strcpy_s(ip_str[1], str);
		nox_sock = socket(AF_INET, SOCK_STREAM, 0);
		//
		if (connect(nox_sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
			printf("Nox[%s]�֐ڑ����܂����B�p�P�b�g�̒��p���J�n�B\n", ip_str[0]);
			u_long val = 1;
			//�m���u���b�L���O�\�P�b�g�ɐݒ�
			ioctlsocket(opponent_sock, FIONBIO, &val);
			ioctlsocket(nox_sock, FIONBIO, &val);
			while (true) {
				//�@�ΐ푊�肩��̃p�P�b�g��M
				if ((len = recv(opponent_sock, recv_buf, sizeof(recv_buf), 0)) > 0) {
					//�ΐ푊�� �� nox �փp�P�b�g���p
					send(nox_sock, recv_buf, len, 0);
				} else if(WSAGetLastError() != WSAEWOULDBLOCK) {
					//�ΐ푊�肩��̐ڑ����؂ꂽ���B2�̃\�P�b�g�����B
					closesocket(opponent_sock);
					closesocket(nox_sock);
					break;
				}
				//�Anox����̃p�P�b�g��M
				if ((len = recv(nox_sock, recv_buf, sizeof(recv_buf), 0)) > 0) {
					//nox �� �ΐ푊�� �փp�P�b�g���p
					send(opponent_sock, recv_buf, len, 0);
 				} else if (WSAGetLastError() != WSAEWOULDBLOCK) {
					//nox����̐ڑ����؂ꂽ���B2�̃\�P�b�g�����B
					closesocket(opponent_sock);
					closesocket(nox_sock);
					break;
				}
				Sleep(1);
			}
		} else {
			puts("Nox�ւ̐ڑ������s�B\n");
		}
	}
	WSACleanup();
    return 0;
}

