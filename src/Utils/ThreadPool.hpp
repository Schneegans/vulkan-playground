////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Math/math.hpp"
#include "stl_helpers.hpp"

#include <condition_variable>
#include <queue>
#include <thread>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This is based on https://github.com/SaschaWillems/Vulkan/blob/master/base/threadpool.hpp       //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Thread {

 public:
  Thread() { mWorker = std::thread(&Thread::queueLoop, this); }

  ~Thread()
  {
    if (mWorker.joinable()) {
      wait();
      mQueueMutex.lock();
      mDestroying = true;
      mCondition.notify_one();
      mQueueMutex.unlock();
      mWorker.join();
    }
  }

  // Add a new job to the thread's queue
  void addJob(std::function<void()> function)
  {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    mJobQueue.push(std::move(function));
    mCondition.notify_one();
  }

  // Wait until all work items have been finished
  void wait()
  {
    std::unique_lock<std::mutex> lock(mQueueMutex);
    mCondition.wait(lock, [this]() { return mJobQueue.empty(); });
  }

 private:
  // Loop through all remaining jobs
  void queueLoop()
  {
    while (true) {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCondition.wait(lock, [this] { return !mJobQueue.empty() || mDestroying; });
        if (mDestroying) {
          break;
        }
        job = mJobQueue.front();
      }

      job();

      {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mJobQueue.pop();
        mCondition.notify_one();
      }
    }
  }

  bool                              mDestroying = false;
  std::thread                       mWorker;
  std::queue<std::function<void()>> mJobQueue;
  std::mutex                        mQueueMutex;
  std::condition_variable           mCondition;
};

class ThreadPool {

 public:
  // Sets the number of threads to be allocted in this pool
  void setThreadCount(math::uint32 count)
  {
    mThreads.clear();
    for (auto i = 0; i < count; i++) {
      mThreads.push_back(makeUnique<Thread>());
    }
  }

  // Wait until all threads have finished their work items
  void wait()
  {
    for (auto& thread : mThreads) {
      thread->wait();
    }
  }

 private:
  std::vector<std::unique_ptr<Thread>> mThreads;
};
}
