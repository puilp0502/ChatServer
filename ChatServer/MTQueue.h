#include "stdafx.h"


template <typename T>
class node{ //���ø����� ���� ��� Ŭ����
public:
	T value;
	node<T>* next;
	unsigned long long int MsgId;
	int thread_taken;
};

template <typename T>
class Queue{ //���� �����忡�� ���� ���� ������ ť Ŭ����
private:
	int count;
	unsigned long long int LastMsgId;
	node<T>* front;
	HANDLE queueMutex;
	int threadcount = 0;
public:
	Queue(){
		LastMsgId = 0;
		count = 0;
		front = NULL;
		queueMutex = CreateMutex(NULL, FALSE, NULL); //Mutex doesn't have its name, nor does it belong to the creator,
													 //which will be the main in this case.
		threadcount = 0;
	}
	~Queue(){
		DWORD mutex = WaitForSingleObject(queueMutex, INFINITE);
		node<T>* n = front; //�μ� ��� ����
		node<T>* b = front; //���� ��� ����
		switch (mutex){
		case WAIT_OBJECT_0:
			while (count-->0){
				b = n->next;
				free(n);
				n = b;
			}
			ReleaseMutex(queueMutex);
			break;
		case WAIT_ABANDONED: //ť�� �ı��Ǳ� ���� �̹� �������� ���
			return;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;
		}
		
	}
	HANDLE getMutex(){ // Mutex Getter.
		return queueMutex;
	}

	void add(T data){//ť�� ���� �߰��մϴ�.
		DWORD mutex = WaitForSingleObject(queueMutex, INFINITE);
		node<T>* n = new node<T>();
		switch (mutex){
		case WAIT_OBJECT_0: //�θ� �����尡 �������� �޾��� ��
			n->value = data;
			n->next = NULL;
			n->MsgId = LastMsgId++;
			n->thread_taken = 0;
			if (count == 0){
				front = n;
			}
			else{
				node<T>* tmp = front;
				node<T>* rear = NULL;
				while (tmp != NULL){
					rear = tmp;
					tmp = tmp->next;
				}
				rear->next = n;
			}
			count++;
			ReleaseMutex(queueMutex);
			break;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;

		}
	}
	T pop(){ //ť���� ���� ��������, �� ��Ʈ���� �����մϴ�.
		DWORD result = WaitForSingleObject(queueMutex, INFINITE);
		T val;
		node<T>* f = front;
		switch (result){
		case WAIT_OBJECT_0:
			if (front == NULL){
				//fprintf(stderr, "queue is empty!");
				ReleaseMutex(queueMutex);
				return NULL;
			}
			front = f->next;
			val = f->value;
			count--;
			delete f;
			ReleaseMutex(queueMutex);
			return val;
			break;
		default:
			TCHAR buffer[1024];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0x0800, buffer, 1024, NULL);
			std::cout << buffer << std::endl;
			break;
		}
	}

	T getFirst(){ //pop()�� ������, ť���� ��Ʈ���� �������� �ʽ��ϴ�.
		if (front == NULL){
			fprintf(stderr, "queue is empty!");
			return NULL; //-1
		}
		return front->value;
	}
	/*node<T>* getEntry(unsigned long long MsgId){
		unsigned long long count=0;
		unsigned long long thisId;
		node<T>* entry = front;
		thisId = entry->MsgId;
		while (++count < thisId){
			entry = entry->next;
			thisId = entry->MsgId;
		}
		return entry;
	}*/
	T getEntry(unsigned long long entryMsgId, int threadCount){
		int ret = WaitForSingleObject(queueMutex, INFINITE);
		node<T>* scan_target = front;
		T val; //��Ʈ���� �μ� �� ����
		switch (ret){
		case WAIT_OBJECT_0:
			while (1){
				if (scan_target == NULL){
					ReleaseMutex(queueMutex);
					return "";
				}
				if (scan_target->MsgId >= entryMsgId){
					if (scan_target->thread_taken >= threadCount - 1){ //�ν��� ��
						val = pop();
						ReleaseMutex(queueMutex);
						return val;
					}
					ReleaseMutex(queueMutex);
					return scan_target->value;
				}
				scan_target = scan_target->next;
			}
			break;
		default:
			break;
		}
	}
	T getLast(){ //�������� �߰��� ���� �����ɴϴ�. �׽�Ʈ��.
		if (front == NULL){
			fprintf(stderr, "queue is empty!");
			return NULL; //-1
		}
		node<T>* tmp = front;
		node<T>* rear = NULL;
		while (tmp != NULL){
			rear = tmp;
			tmp = tmp->next;
		}
		return rear->value;
	}
	bool isEmpty(){
		if (count) return false;
		return true;
	}
	unsigned long long getLastMsgId(){
		return LastMsgId;
	}
};