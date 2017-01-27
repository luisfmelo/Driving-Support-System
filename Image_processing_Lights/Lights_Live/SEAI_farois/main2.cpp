#define WIN32_LEAN_AND_MEAN
#include "opencv2/opencv.hpp"
#include <windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <array>
#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h" // Library described above
#include <string.h>
#include <atlstr.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "4000" 
#define DEFAULT_BUFFER_LENGTH	512

using namespace cv;
using namespace std;

class Client {
public:
	Client(char* servername)
	{
		szServerName = servername;
		ConnectSocket = INVALID_SOCKET;
	}

	bool Start() {
		WSADATA wsaData;

		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			printf("WSAStartup failed: %d\n", iResult);
			return false;
		}

		struct addrinfo	*result = NULL,
			*ptr = NULL,
			hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(szServerName, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
		{
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return false;
		}

		ptr = result;

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("Error at socket(): %d\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return false;
		}

		// Connect to server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("Unable to connect to server!\n");
			WSACleanup();
			return false;
		}

		return true;
	};

	// Free the resouces
	void Stop() {
		int iResult = shutdown(ConnectSocket, SD_SEND);

		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed: %d\n", WSAGetLastError());
		}

		closesocket(ConnectSocket);
		WSACleanup();
	};

	// Send message to server
	bool Send(char* szMsg)
	{

		int iResult = send(ConnectSocket, szMsg, strlen(szMsg), 0);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed: %d\n", WSAGetLastError());
			Stop();
			return false;
		}

		return true;
	};

	// Receive message from server
	bool Recv()
	{
		char recvbuf[DEFAULT_BUFFER_LENGTH];
		int iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFFER_LENGTH, 0);

		if (iResult > 0)
		{
			char msg[DEFAULT_BUFFER_LENGTH];
			memset(&msg, 0, sizeof(msg));
			strncpy(msg, recvbuf, iResult);

			printf("Received: %s\n", msg);

			return true;
		}


		return false;
	}

private:
	char* szServerName;
	SOCKET ConnectSocket;
};

int main(int argc , char* argv[]){

	// Comunicação TCP
	
	Client client("192.168.113.11");

	if (!client.Start())
		return 1;

	
	

	//VideoCapture cap("../data/noite.mp4"); // video teste
	VideoCapture cap(0); // captura
	if (!cap.isOpened())  // check if we succeeded
		return -1;
	
	/*
	*HSV típico tem valores H = [0; 360], S = [0;100], V=[0;100].
	*OpenCV tem valores H = [0; 180], S = [0;255], V=[0;255]
	*Estes multiplicadores permitem adaptar valores normais a OpenCV.
	*/
	double HUE_MULTIPLIER = 0.5;
	double SATURATION_MULTIPLIER = 2.55;
	double VALUE_MULTIPLIER = 2.55;
	
	//Comunicação série
	//Comentado para testes sem microcontrolador
	
	printf("Inicializar\n\n");
	int i = 0;
	Serial* SP = new Serial("\\\\.\\COM3");    // porta a que estiver conectado
	if (SP->IsConnected())
		printf("Está conectado\n\n");
	
	int dataLengthIn = 3;
	int dataLengthOut = 8;
	//char incomingData[1] = "";   // don't forget to pre-allocate memory
	char goingData[8] = "";
	int readResult = 0;
	bool writeResult;
	


	// Comunicação TCP

	//Limites de filtragem
	// Roxo azulado para o vídeo noite.mp4
	//Scalar low_red = cvScalar(280 * HUE_MULTIPLIER, 30 * SATURATION_MULTIPLIER, 40 * VALUE_MULTIPLIER); //Faróis
	//Scalar high_red = cvScalar(360 * HUE_MULTIPLIER, 100 * SATURATION_MULTIPLIER, 100 * VALUE_MULTIPLIER); //Faróis
	// Amarelo para teste na sala I005
	Scalar low_red = cvScalar(40 * HUE_MULTIPLIER, 0 * SATURATION_MULTIPLIER, 70 * VALUE_MULTIPLIER); //Faróis
	Scalar high_red = cvScalar(70 * HUE_MULTIPLIER, 50 * SATURATION_MULTIPLIER, 100 * VALUE_MULTIPLIER); //Faróis
	
	// Preparar HSV
	Mat hsv, grayscale, img_inRange, post_Roi_exesquerda, post_Roi_midesquerda, post_Roi_centro, post_Roi_middireita, post_Roi_exdireita;
	
	// BIT CODING
	// 0-4 - Pares de focos (0 é mais à direita, 4 é mais à esquerda; 0 é sem deteção, 1 é com deteção)
	// 5 - Luminosidade (0 é escuro, 1 é claro)
	// 6 - Velocidade (1 e 0 são o que quiseres)
	int msg = 0, mask_exesq = 16, mask_midesq = 8, mask_centro = 4, mask_middir = 2, mask_exdir = 1;
	int timeoutCount = 0;
	// Comenta isto se quiseres "não velocidade"
	int mask_velocidade = 64;
	//msg = msg | mask_velocidade;
	int placeholder;
	
	for (;;){
		

		/* ---------------------------------------------------------------------------------- */
		//Preparação Processamento de imagem para deteção de trânsito
		Mat frame;
		cap >> frame; // get a new frame from camera

		cvtColor(frame, hsv, COLOR_BGR2HSV);
		cvtColor(frame, grayscale, COLOR_BGR2GRAY);
		GaussianBlur(hsv, hsv, Size(7, 7), 1.5, 1.5);

		//Região de interesse
		Rect Rec_exesquerda(180, 125, 45, 100);
		Mat Roi_exesquerda = hsv(Rec_exesquerda);
		Rect Rec_midesquerda(225, 125, 45, 100);
		Mat Roi_midesquerda = hsv(Rec_midesquerda);
		Rect Rec_centro(270, 125, 45, 100);
		Mat Roi_centro = hsv(Rec_centro);
		Rect Rec_middireita(315, 125, 45, 100);
		Mat Roi_middireita = hsv(Rec_middireita);
		Rect Rec_exdireita(360, 125, 45, 100);
		Mat Roi_exdireita = hsv(Rec_exdireita);
		Rect Roi_luminosidade(0, 0, grayscale.cols, 150); //Região de interesse - orientada para o céu
		Mat Roi_mat = grayscale(Roi_luminosidade);//Região de interesse - orientada para o céu


		// Faróis
		inRange(hsv, low_red, high_red, img_inRange);
		inRange(Roi_exesquerda, low_red, high_red, post_Roi_exesquerda);
		inRange(Roi_midesquerda, low_red, high_red, post_Roi_midesquerda);
		inRange(Roi_centro, low_red, high_red, post_Roi_centro);
		inRange(Roi_middireita, low_red, high_red, post_Roi_middireita);
		inRange(Roi_exdireita, low_red, high_red, post_Roi_exdireita);

		//Informar outros sistemas
		msg = 0;
		msg = msg | mask_velocidade;
		if (countNonZero(post_Roi_exesquerda) > 10) {
			//putText(frame, "Carro a esquerda", Point(0, 10), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | mask_exesq;
		}if (countNonZero(post_Roi_midesquerda) > 10) {
			//putText(frame, "Carro a frente", Point(0, 20), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | mask_midesq;
		}if (countNonZero(post_Roi_centro) > 10) {
			//putText(frame, "Carro a direita", Point(0, 30), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | mask_centro;
		}if (countNonZero(post_Roi_middireita) > 10) {
			//putText(frame, "Carro a direita", Point(0, 30), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | mask_middir;
		}if (countNonZero(post_Roi_exdireita) > 10) {
			//putText(frame, "Carro a direita", Point(0, 30), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | mask_exdir;
		}
		
		
		/* ---------------------------------------------------------------------------------- */ 
		//Preparação Processamento de imagem para deteção de luminosidade
		/*
		0: Binary
		1: Binary Inverted
		2: Threshold Truncated
		3: Threshold to Zero
		4: Threshold to Zero Inverted
		*/

		threshold(Roi_mat, Roi_mat, 40, 0, 3);

		//Preparar mensagem
		//Forçado a noite para demonstração
		//msg = msg | 32;
		//char TCPmsg[4];
		//strcat(TCPmsg, "F");
		string TCPmsg = "F";
		int numPixels = Roi_mat.total();
		if (countNonZero(Roi_mat) > (0.5*numPixels)) {
			putText(frame, "Dia", Point(0, 30), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			TCPmsg = TCPmsg + "d";
		}
		else if (countNonZero(Roi_mat) < (0.5*numPixels)) {
			putText(frame, "Noite", Point(0, 30), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));
			msg = msg | 32;
			TCPmsg = TCPmsg + "n";
		};
		
		/* ---------------------------------------------------------------------------------- */
		// Comunicação
		
		char buffer[2];
		int bufferSize = 2;
		_itoa(msg, buffer, 10);
		putText(frame, to_string(msg), Point(0, 40), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 0, 255, 255));

		/// Show your results
		rectangle(frame, Rec_exesquerda, Scalar(255, 255, 255), 0, 0, 0);
		rectangle(frame, Rec_midesquerda, Scalar(255, 255, 255), 0, 0, 0);
		rectangle(frame, Rec_centro, Scalar(255, 255, 255), 0, 0, 0);
		rectangle(frame, Rec_middireita, Scalar(255, 255, 255), 0, 0, 0);
		rectangle(frame, Rec_exdireita, Scalar(255, 255, 255), 0, 0, 0);
		imshow("Original", frame);
		imshow("Processed", hsv);
		imshow("Filtered", img_inRange);
		//Sleep(500);

		

		
		//Comentado para testes sem microcontrolador
		//Comunicação
		
		if (i > 15)
		{
			//writeResult = SP->WriteData(buffer, bufferSize);
			writeResult = SP->WriteDatabin(&msg, bufferSize);
			//Sleep(100);
			char incomingData[3] = "";
			readResult = SP->ReadData(incomingData, dataLengthIn);
			printf("Bytes read: (-1 means no data available) %i\n", readResult);
			//cout << incomingData << "\n";
			//Receive msg
			
			int faroisAuto  = ( (int) incomingData & 0b00000001);
			int faroisDia = ((((int) incomingData >> (1)) & 0b00000001));
			int faroisMinimos = ((((int)incomingData >> (2)) & 0b00000001));
			int faroisMaximos = ((((int)incomingData >> (3)) & 0b00000001));
			int faroisI2CError = ((((int)incomingData >> (4)) & 0b00000001));
			int faroisVideoError = ((((int)incomingData >> (5)) & 0b00000001));
			
			cout << "\n" << (char) incomingData << "\n";
			cout << faroisAuto << ", " << faroisDia << ", " << faroisMinimos << ", " << faroisMaximos << ", " << faroisI2CError << ", " << faroisVideoError << "\n";
					
			//Comunicação TCP
			/* DESCRIÇÃO DE MENSAGENS
				Fe - Erro de I2C (do microcontrolador)
				Fm - Modo manual
				Fa - Modo automático
				Fd - Dia (pelo algoritmo de luminosidade)
				Fn - Noite (pelo algoritmo de luminosidade)
				Ft - Timeout do microcontrolador.
			*/
			Sleep(2);
			if (!faroisAuto) {
				//strcat(TCPmsg, "a");
				TCPmsg = TCPmsg + "a";
			}
			if (faroisAuto) {
				//strcat(TCPmsg, "m");
				TCPmsg = TCPmsg + "m";
			}
			if (faroisI2CError) {
				//strcat(TCPmsg, "e");
				TCPmsg = TCPmsg + "e";
			}
			
			
			if (readResult == 0)
				timeoutCount++;
			else
				timeoutCount = 0;

			if (timeoutCount == 5) {
				//strcat(TCPmsg, "d");
				TCPmsg = TCPmsg + "d";
				//string str(TCPmsg);
				//client.Send(TCPmsg);
				//client.Send((char*)str.c_str());
				cout << "timeout from MCU. Exiting...";
				return -1;
			}
			client.Send((char*)TCPmsg.c_str());
			i = 0;
		}
		else
		{
			i++;
		}
		
		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}