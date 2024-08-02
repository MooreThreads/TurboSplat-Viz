#include<vector>
#include<queue>
#include<thread>
#include<functional>
class ThreadsPool
{
private:
	std::vector<std::function<void>> m_unordered_tasks;
	std::queue<std::function<void>> m_task_queue;
	std::vector < std::thread> m_threads;
	static void internel_ThreadRun();
public:
	ThreadsPool();
	virtual void Run(int thread_num);
	virtual void Stop();
	virtual void AddTask(const std::function<void>& func);
	virtual void EnqueueTask(const std::function<void>& func);
};