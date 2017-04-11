#include <iostream>
#include <vector>
#include <string>
using namespace std;

char buffer[1024];

template <typename Functor>
class CFunction
{
public:
	//1.����дΪ��ͨ��Ա���� ��ͨ��Ա�������������ڶ���
	static void invoke(typename Functor::ARG_TYPE val)
	{
		Functor *p = (Functor *)buffer;
		(*p)(val);//��_mybind()���������
	}
};

/*
//����дΪȫ�ֺ���
template <typename Functor>
void invoke(typename Functor::ARG_TYPE val)
{
	Functor *p = (Functor *)buffer;
	(*p)(val);//��_mybind()���������
}
*/

template <typename T, typename R, typename A>
class _mybind
{
public:
	typedef A ARG_TYPE;
	typedef R(T::*PFUNC)(A);
	_mybind(PFUNC pfunc, T *ptr):_pfunc(pfunc),_ptr(ptr){}
	R operator()(A arg)
	{
		(_ptr->*_pfunc)(arg);
	}
private:
	PFUNC _pfunc;
	T *_ptr;
};

template <typename T, typename R, typename A>
_mybind<T,R,A> mybind(R(T::*pfunc)(A), T *ptr)
{
	return _mybind<T,R,A>(pfunc, ptr);
}
//ģ��
template <typename T>
class myfunction
{

};
//��Է���ֵΪvoid����Ϊ�������͵ĺ������з���ȫ������
template<typename A>
class myfunction<void(A)>
{
public:
	typedef void(*PFUNC)(A);
	//�ṩ���캯����ģ�� ��ͬ����֮��
	template <typename Functor>
	myfunction(Functor &func)//תΪ�������͵ĵ�ʱ����ʽ���ɵ���ʱ���� 
	{	//�൱�ڴ˴���10: test = 10;(testΪ����)
		//func->buff�ö�λnew ��buffer�￪��func�����Functor����
		new (buffer) Functor(func);//�����ֲ�������

		//Functorʵ�����ⲿ��һ������ _mybind<R,T,A> ��������
		_pfunc = &CFunction<Functor>::invoke;
	}
	void operator()(A data)
	{
		//func(data)����Ŀ��
		(*_pfunc)(data);//��invoke����
	}
private:
	PFUNC _pfunc;
};

class Thread
{
public:
	//�߳��ﹹ�� ·������
	typedef myfunction<void(int)> Function;
	Thread(const Function &func):_func(func){}//const ��ͬ���͵�ת��
	//�ṩ myfunction���캯��ģ��
	
	void start()
	{
		_func(20);//��myfuntion()��������� 
	}
private:
	Function _func;
};

class ThreadPool
{
public:
	ThreadPool(string name=" "):_name(name){}
	void start(int numThreads)
	{
		for(int i=0; i<numThreads; ++i)
		{
			_threads.push_back(new Thread(mybind(&ThreadPool::RunInThread, this)));//������������̶߳���
			//Thread::Thread(Thread::Function &)��: 
			//���ܽ����� 1 �ӡ�_mybind<T,R,A>��
			//ת��Ϊ��Thread::Function &��
			_threads[i]->start();
		}
	}
private:
	void RunInThread(int val)
	{
		cout<<"threadPool: "<<_name<<endl;
		cout<<"runInThread val:"<<val<<endl;
	}
	string _name;
	vector<Thread*> _threads;
};

int main()
{
	ThreadPool pool("pool");
	pool.start(5);
	return 0;
}