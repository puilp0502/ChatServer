#include "stdafx.h"


template <typename T>
class node{ //템플릿으로 만든 노드 클래스
public:
	T value;
	node<T>* next;
	unsigned long long int MsgId;
	int thread_taken;
};

template <typename T>
class Queue{ //다중 스레드에서 동시 접근 가능한 큐 클래스
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
		node<T>* n = front; //부술 노드 저장
		node<T>* b = front; //다음 노드 저장
		switch (mutex){
		case WAIT_OBJECT_0:
			while (count-->0){
				b = n->next;
				free(n);
				n = b;
			}
			ReleaseMutex(queueMutex);
			break;
		case WAIT_ABANDONED: //큐가 파괴되기 전에 이미 끝나버린 경우
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

	void add(T data){//큐에 값을 추가합니다.
		DWORD mutex = WaitForSingleObject(queueMutex, INFINITE);
		node<T>* n = new node<T>();
		switch (mutex){
		case WAIT_OBJECT_0: //부른 쓰레드가 소유권을 받았을 때
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
	T pop(){ //큐에서 값을 가져오고, 그 엔트리를 삭제합니다.
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

	T getFirst(){ //pop()과 같지만, 큐에서 엔트리를 제거하지 않습니다.
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
		T val; //엔트리를 부술 때 쓰임
		switch (ret){
		case WAIT_OBJECT_0:
			while (1){
				if (scan_target == NULL){
					ReleaseMutex(queueMutex);
					return "";
				}
				if (scan_target->MsgId >= entryMsgId){
					if (scan_target->thread_taken >= threadCount - 1){ //부숴도 됨
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
	T getLast(){ //마지막에 추가된 값을 가져옵니다. 테스트용.
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