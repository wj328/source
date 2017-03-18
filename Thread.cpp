#include <iostream>
#include <vector>
#include <string>
using namespace std;

char buffer[1024];

template <typename Functor>
class CFunction
{
public:
	//1.不能写为普通成员方法 普通成员方法调用依赖于对象
	static void invoke(typename Functor::ARG_TYPE val)
	{
		Functor *p = (Functor *)buffer;
		(*p)(val);//调_mybind()运算符重载
	}
};

/*
//不能写为全局函数
template <typename Functor>
void invoke(typename Functor::ARG_TYPE val)
{
	Functor *p = (Functor *)buffer;
	(*p)(val);//调_mybind()运算符重载
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
//模板
template <typename T>
class myfunction
{

};
//针对返回值为void参数为任意类型的函数进行非完全特例化
template<typename A>
class myfunction<void(A)>
{
public:
	typedef void(*PFUNC)(A);
	//提供构造函数的模板 不同类型之间
	template <typename Functor>
	myfunction(Functor &func)//转为此类类型的的时候隐式生成的临时对象 
	{	//相当于此处的10: test = 10;(test为对象)
		//func->buff用定位new 在buffer里开用func构造的Functor对象
		new (buffer) Functor(func);//带出局部的数据

		//Functor实例化外部的一个东西 _mybind<R,T,A> 带出类型
		_pfunc = &CFunction<Functor>::invoke;
	}
	void operator()(A data)
	{
		//func(data)最终目的
		(*_pfunc)(data);//调invoke函数
	}
private:
	PFUNC _pfunc;
};

class Thread
{
public:
	//线程里构造 路径函数
	typedef myfunction<void(int)> Function;
	Thread(const Function &func):_func(func){}//const 不同类型的转换
	//提供 myfunction构造函数模板
	
	void start()
	{
		_func(20);//调myfuntion()运算符重载 
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
			_threads.push_back(new Thread(mybind(&ThreadPool::RunInThread, this)));//往容器里添加线程对象
			//Thread::Thread(Thread::Function &)”: 
			//不能将参数 1 从“_mybind<T,R,A>”
			//转换为“Thread::Function &”
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