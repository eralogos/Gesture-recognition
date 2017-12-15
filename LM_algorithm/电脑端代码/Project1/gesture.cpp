/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include "Leap.h"
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//���÷��͵���ݮ�ɵĻ�������С
#define BUF_SIZE 10
#include<string.h>
#include<WS2tcpip.h>
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll


using namespace Leap;

//���������˶���صı����ͳ���
//�������������˶������������ֵ
#define XHorizonDistance 100
#define YHorizonDistance 100
#define ZHorizonDistance 100
//�����ʾ�������˶�����ı�����ÿֻ�ֶ�Ӧÿһ����ţ��ݶ�����⵽5ֻ��
float countr[5] = { 0 };
float countd[5] = { 0 };
float countb[5] = { 0 };
//���ڴ洢��һ֡���˶�λ�ã���ŷ�ʽͬ��
float prex[5] = { 0 };
float prey[5] = { 0 };
float prez[5] = { 0 };
//����˶��ﵽ��ֵ����movΪ1���棩������Ϊ0
int mov;
//����ÿ���յ�10֡��ת���Ʋ�����ݮ�ɷ���һ������
#define RO_MAXSIZE 10
//̽�⵽����ת��ת����֡���ļ���
int ro1, ro2;

//����״̬��أ�����ͬ��
Vector normal[5];
Vector direction[5];

//��ֱ����ָ��
int extendfin;

//�Ƿ�̽�⵽��Ȧ���ƣ�0Ϊ��ת��1Ϊ��ת��-1Ϊû̽�⵽
int rotation;

//���巢�͵���ݮ�ɵĻ�����
char bufSend[BUF_SIZE];
char prebufSend[BUF_SIZE];


class SampleListener : public Listener {
public:
	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
	virtual void onExit(const Controller&);
	virtual void onFrame(const Controller&);
	virtual void onFocusGained(const Controller&);
	virtual void onFocusLost(const Controller&);
	virtual void onDeviceChange(const Controller&);
	virtual void onServiceConnect(const Controller&);
	virtual void onServiceDisconnect(const Controller&);

private:
};

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
const std::string stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };

void SampleListener::onInit(const Controller& controller) {
	std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
	std::cout << "Connected" << std::endl;
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
	// Note: not dispatched when running in a debugger.
	std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
	std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
	// Get the most recent frame and report some basic information
	
	const Frame frame = controller.frame();

	/*
	std::cout << "Frame id: " << frame.id()
		<< ", timestamp: " << frame.timestamp()
		<< ", hands: " << frame.hands().count()
		<< ", extended fingers: " << frame.fingers().extended().count()
		<< ", tools: " << frame.tools().count()
		<< ", gestures: " << frame.gestures().count() << std::endl;
	*/

	//������ʼ��
	rotation = -1;
	extendfin = -1;
	mov = 0;
	
	//socket��ʼ�������ӹ���ֻ�ڵ�һ������onframeʱ��
	static WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//������������������ӹ���ֻ�ڵ�һ������onframeʱ��
	static sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("192.168.137.100");
	sockAddr.sin_port = htons(1234);

	static SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	static int co = connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	if (co == SOCKET_ERROR)
	{
		printf("����ʧ��");
	}
	
	//��ʼ����������ȫ��N����
	memset(bufSend, 'N', BUF_SIZE);

	HandList hands = frame.hands();
	int i = 0;
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
		//std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
		//	<< ", palm position: " << hand.palmPosition() << std::endl;
		

		//�ó�ÿֻ�ֵ��˶�״̬
		if (hand.palmPosition().x < prex[i] && countr[i]>=0&& countr[i]<XHorizonDistance)
			countr[i]+= prex[i]- hand.palmPosition().x;
		else if (hand.palmPosition().x < prex[i] && countr[i] < 0)
			countr[i] =0;
		else if (hand.palmPosition().x > prex[i] && countr[i] <= 0 && countr[i]>-XHorizonDistance)
			countr[i]-= hand.palmPosition().x- prex[i];
		else if (hand.palmPosition().x > prex[i] && countr[i] > 0)
			countr[i] =0;
		prex[i] = hand.palmPosition().x;

		if (hand.palmPosition().y < prey[i] && countd[i]>=0 && countd[i]<YHorizonDistance)
			countd[i]+= prey[i]- hand.palmPosition().y;
		else if (hand.palmPosition().y < prey[i] && countd[i] < 0)
			countd[i] =0;
		else if (hand.palmPosition().y > prey[i] && countd[i] <= 0 && countd[i]>-YHorizonDistance)
			countd[i]-= hand.palmPosition().y- prey[i];
		else if (hand.palmPosition().y > prey[i] && countd[i] > 0)
			countd[i] =0;
		prey[i] = hand.palmPosition().y;

		if (hand.palmPosition().z < prez[i] && countb[i]>=0 && countb[i]<ZHorizonDistance)
			countb[i]+= prez[i]- hand.palmPosition().z;
		else if (hand.palmPosition().z < prez[i] && countb[i] < 0)
			countb[i] =0;
		else if (hand.palmPosition().z > prez[i] && countb[i] <= 0 && countb[i]>-ZHorizonDistance)
			countb[i]-= hand.palmPosition().z- prez[i];
		else if (hand.palmPosition().z > prez[i] && countb[i] > 0)
			countb[i] =0;
		prez[i] = hand.palmPosition().z;
		//printf("%f   %f   %f\n", countr[i], countd[i], countb[i]);
		


		//�ж����Ʒ���
		
		if (countr[i] >= XHorizonDistance)
		{
			mov = 1;
			bufSend[1] = 'l';
			countr[i] = 0;
			printf("hand%d �����˶�\n",i);
		}
		if (countr[i] <= -XHorizonDistance)
		{
			mov = 1;
			bufSend[1] = 'r';
			countr[i] = 0;
			printf("hand%d �����˶�\n", i);
		}
		if (countd[i] >= YHorizonDistance)
		{
			mov = 1;
			bufSend[2] = 'd';
			countd[i] = 0;
			printf("hand%d �����˶�\n", i);
		}
		if (countd[i] <= -YHorizonDistance)
		{
			mov = 1;
			bufSend[2] = 'u';
			countd[i] = 0;
			printf("hand%d �����˶�\n", i);
		}
		if (countb[i] >= ZHorizonDistance)
		{
			mov = 1;
			bufSend[3] = 'f';
			countb[i] = 0;
			printf("hand%d ��ǰ�˶�\n", i);
		}
		if (countb[i] <= -ZHorizonDistance)
		{
			mov = 1;
			bufSend[3] = 'b';
			countb[i] = 0;
			printf("hand%d ����˶�\n", i);
		}

		
		

		// Get the hand's normal vector and direction
		//��ȡ���Ƶ�ָ��ͳ���
		normal[i] = hand.palmNormal();
		direction[i] = hand.direction();



		// Calculate the hand's pitch, roll, and yaw angles
		//std::cout << std::string(2, ' ') << "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
		//	<< "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
		//	<< "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

		// Get the Arm bone
		Arm arm = hand.arm();
		//std::cout << std::string(2, ' ') << "Arm direction: " << arm.direction()
		//	<< " wrist position: " << arm.wristPosition()
		//	<< " elbow position: " << arm.elbowPosition() << std::endl;

		// Get fingers
		const FingerList fingers = hand.fingers();

		//�õ���չ����ָ�б�
		FingerList extendedFingerList = hand.fingers().extended();
		//��ȡ��չ����ָ��
		extendfin = extendedFingerList.count();
		//printf("%d\n", extendfin);

		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			/*
			std::cout << std::string(4, ' ') << fingerNames[finger.type()]
				<< " finger, id: " << finger.id()
				<< ", length: " << finger.length()
				<< "mm, width: " << finger.width() << std::endl;
			*/
			// Get finger bones
			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);
				/*
				std::cout << std::string(6, ' ') << boneNames[boneType]
					<< " bone, start: " << bone.prevJoint()
					<< ", end: " << bone.nextJoint()
					<< ", direction: " << bone.direction() << std::endl;
				*/
			}
		}
		i++;
	}


	//˫���жϣ�����չ����
	/*
	if (frame.hands().count() == 2&&countr[0]+countr[1]==0&&countd[0]+countd[1]==-2*YHorizonDistance)
	{
		printf("boom\n");
		countr[0] = countr[1] = countd[0] = countd[1] = 0;
	}
	*/

	// Get tools
	const ToolList tools = frame.tools();
	for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
		const Tool tool = *tl;
		//std::cout << std::string(2, ' ') << "Tool, id: " << tool.id()
		//	<< ", position: " << tool.tipPosition()
		//	<< ", direction: " << tool.direction() << std::endl;
	}
	

	
	// Get gestures
	const GestureList gestures = frame.gestures();
	for (int g = 0; g < gestures.count(); ++g) {
		Gesture gesture = gestures[g];

		switch (gesture.type()) {
			
		case Gesture::TYPE_CIRCLE:
		{
			

			CircleGesture circle = gesture;
			std::string clockwiseness;
			
			//�ж���ת����
			if (circle.pointable().direction().angleTo(circle.normal()) <= PI / 2) {
				clockwiseness = "clockwise";
				rotation = 1;
			}
			else {
				clockwiseness = "counterclockwise";
				rotation = 0;
			}

			// Calculate angle swept since last frame
			float sweptAngle = 0;
			if (circle.state() != Gesture::STATE_START) {
				CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
				sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
			}
			/*
			std::cout << std::string(2, ' ')
				<< "Circle id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", progress: " << circle.progress()
				<< ", radius: " << circle.radius()
				<< ", angle " << sweptAngle * RAD_TO_DEG
				<< ", " << clockwiseness << std::endl;
			*/
			break;
		}
		
		case Gesture::TYPE_SWIPE:
		{
			SwipeGesture swipe = gesture;
			/*
			std::cout << std::string(2, ' ')
				<< "Swipe id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", direction: " << swipe.direction()
				<< ", speed: " << swipe.speed() << std::endl;
			*/
			break;
		}
		case Gesture::TYPE_KEY_TAP:
		{
			KeyTapGesture tap = gesture;
			/*
			std::cout << std::string(2, ' ')
				<< "Key Tap id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", position: " << tap.position()
				<< ", direction: " << tap.direction() << std::endl;
			*/
			break;
		}
		
		case Gesture::TYPE_SCREEN_TAP:
		{
			ScreenTapGesture screentap = gesture;
			/*
			std::cout << std::string(2, ' ')
				<< "Screen Tap id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", position: " << screentap.position()
				<< ", direction: " << screentap.direction() << std::endl;
			*/
			break;
		}
		
		default:
			//std::cout << std::string(2, ' ') << "Unknown gesture type." << std::endl;
			break;
		}
		
	}
	
	//��ȡ��һ��̽�⵽�����Ƶĳ��򣬰���Ϣ��ֵ�ڻ���������λ��
	if (normal[0].y > 0.8)
		bufSend[4] = 'u';
	else if (normal[0].y < -0.8)
		bufSend[4] = 'd';
	else if (normal[0].x > 0.8)
		bufSend[4] = 'r';
	else if (normal[0].x < -0.8)
		bufSend[4] = 'l';
	else if (normal[0].z > 0.8)
		bufSend[4] = 'f';
	else if (normal[0].z < -0.8)
		bufSend[4] = 'b';
	else bufSend[4] = '0';


	//�������
	if (mov==1)
	{
		ro1 = 0;
		ro2 = 0;
		if (extendfin == 5)
		{
			bufSend[0] = '1';
			printf("%s\n", bufSend);
			send(sock, bufSend, BUF_SIZE, 0);
		}
		else if (extendfin == 1 && bufSend[1] != 'N')
		{
			bufSend[0] = '4';
			printf("%s\n", bufSend);
			send(sock, bufSend, BUF_SIZE, 0);
		}

	}
	else if (rotation == 0)
	{
		bufSend[0] = '2';
		ro2 = 0;
		ro1++;
		if (ro1 == RO_MAXSIZE)
		{
			printf("%s\n", bufSend);
			send(sock, bufSend, BUF_SIZE, 0);
			ro1 = 0;
		}
		

	}
	else if (rotation == 1)
	{
		bufSend[0] = '3';
		ro1 = 0;
		ro2++;
		if (ro2 == RO_MAXSIZE)
		{
			printf("%s\n", bufSend);
			send(sock, bufSend, BUF_SIZE, 0);
			ro2 = 0;
		}

	}
	for (int i = 0; i < BUF_SIZE; i++)
		prebufSend[i] = bufSend[i];

	

	/*
	if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
		std::cout << std::endl;
	}
	*/
}

void SampleListener::onFocusGained(const Controller& controller) {
	std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
	std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
	std::cout << "Device Changed" << std::endl;
	const DeviceList devices = controller.devices();

	for (int i = 0; i < devices.count(); ++i) {
		std::cout << "id: " << devices[i].toString() << std::endl;
		std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
	}
}

void SampleListener::onServiceConnect(const Controller& controller) {
	std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
	std::cout << "Service Disconnected" << std::endl;
}


static WSADATA wsaData;
static sockaddr_in sockAddr;


int main(int argc, char** argv) {

	//closesocket(sock);  //�ر��׽���
	memset(prebufSend, '0', BUF_SIZE);

	// Create a sample listener and controller
	SampleListener listener;
	Controller controller;

	// Have the sample listener receive events from the controller
	controller.addListener(listener);
	
	if (argc > 1 && strcmp(argv[1], "--bg") == 0)
		controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
	
	// Keep this process running until Enter is pressed
	std::cout << "Press Enter to quit..."<< std::endl;
		std::cin.get();
	
	// Remove the sample listener when done
	controller.removeListener(listener);

	//system("pause");

	return 0;
}
