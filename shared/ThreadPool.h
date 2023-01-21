#pragma once

#include <queue>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace advancedfx {

	class CThreadPool {
	public:
		class CTask {
		public:
			virtual ~CTask() {
			}

			virtual void Execute() = 0;
		};

		static size_t GetDefaultThreadCount() {
			int hardware_concurrency = std::thread::hardware_concurrency();

			if (hardware_concurrency < 1) hardware_concurrency = 1;
			else if (32 < hardware_concurrency) hardware_concurrency = 32;

			return hardware_concurrency - 1;
		}

		CThreadPool(size_t thread_count) {
			m_Threads.resize(thread_count);
			for (size_t i = 0; i < m_Threads.size(); i++) {
				m_Threads[i] = std::thread(&CThreadPool::ThreadFunc, this);
			}
		}

		~CThreadPool() {
			Shutdown();
		}

		void QueueTask(class CTask* task) {
			if (m_Threads.size() == 0) {
				task->Execute();
				delete task;
				return;
			}

			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_Queue.push(task);
			m_QueueCv.notify_one();
		}

		void QueueTasks(const std::vector<class CTask*> & tasks) {
			if (m_Threads.size() == 0) {
				for (size_t i = 0; i < tasks.size(); i++) {
					CTask* task = tasks[i];
					task->Execute();
					delete task;
				}
				return;
			}

			std::unique_lock<std::mutex> lock(m_QueueMutex);
			for (size_t i = 0; i < tasks.size(); i++) {
				m_Queue.push(tasks[i]);
			}
			m_QueueCv.notify_one();
		}

		size_t GetThreadCount() {
			return m_Threads.size();
		}

	private:
		std::mutex m_QueueMutex;
		std::queue<CTask*> m_Queue;
		std::condition_variable m_QueueCv;
		bool m_Shutdown = false;

		std::vector<std::thread> m_Threads;

		void ThreadFunc() {
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			while (!m_Shutdown || !m_Queue.empty()) {
				if (!m_Queue.empty()) {
					CTask* task = m_Queue.front();
					m_Queue.pop();
					if (!m_Queue.empty()) m_QueueCv.notify_one();
					lock.unlock();
					task->Execute();
					delete task;
					lock.lock();
				}
				else {
					m_QueueCv.wait(lock);
				}
			}
		}

		void Shutdown() {
			if (m_Shutdown) return;

			{
				std::unique_lock<std::mutex> lock(m_QueueMutex);
				m_Shutdown = true;
				m_QueueCv.notify_all();
			}
			for (size_t i = 0; i < m_Threads.size(); i++) {
				m_Threads[i].join();
			}
		}
	};

} // namespace advancedfx {
