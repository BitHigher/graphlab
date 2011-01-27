#include <graphlab/parallel/pthread_tools.hpp>
#include <graphlab/parallel/queued_rwlock.hpp>
#include <graphlab/logger/assertions.hpp>
#include <vector>
#include <queue>
using namespace graphlab;

std::queue<size_t> gqueue;
std::vector<size_t> counter;
queued_rw_lock lock;
rwlock rwlock2;

void fn(void) {
  queued_rw_lock::request req;
  
  for (size_t i = 0; i < 100000; ++i) {
  /*  if (rand() % 4 == 0) {
      // read
      lock.readlock(&req);
      if (!gqueue.empty()) {
        counter[gqueue.front()]++;
        gqueue.pop();
      }
      lock.rdunlock(&req);
    }*/
    
    lock.writelock(&req);
    gqueue.push(i);
    lock.wrunlock(&req);
  }
  
  std::cout << "inserted" << std::endl;
  size_t j = 0;
  while(1) {
    lock.readlock(&req);
    if (gqueue.empty()) {
      lock.rdunlock(&req);
      break;
    }
    else {
      lock.rdunlock(&req);
      lock.writelock(&req);
      if (!gqueue.empty()) {
        counter[gqueue.front()]++;
        gqueue.pop();
      }
      ++j;
      if (j % 10000 == 0) {
        std::cout << gqueue.size() << std::endl;
      }
      lock.wrunlock(&req);
    }

  }
}




void fn2(void) {
  
  for (size_t i = 0; i < 100000; ++i) {
  /*  if (rand() % 4 == 0) {
      // read
      lock.readlock(&req);
      if (!gqueue.empty()) {
        counter[gqueue.front()]++;
        gqueue.pop();
      }
      lock.rdunlock(&req);
    }*/
    
    rwlock2.writelock();
    gqueue.push(i);
    rwlock2.unlock();
  }
  
  std::cout << "inserted" << std::endl;
  size_t j = 0;
  while(1) {
    rwlock2.readlock();
    if (gqueue.empty()) {
      rwlock2.unlock();
      break;
    }
    else {
      rwlock2.unlock();
      rwlock2.writelock();
      if (!gqueue.empty()) {
        counter[gqueue.front()]++;
        gqueue.pop();
      }
      ++j;
      if (j % 10000 == 0) {
        std::cout << gqueue.size() << std::endl;
      }
      rwlock2.unlock();
    }

  }
}


int main(int argc, char** argv) {
    thread_group group;
    counter.resize(100000, 0);
    size_t nthreads = 4;
    for (size_t i = 0;i < nthreads ; ++i) {
      launch_in_new_thread(group, fn2);
    }
    group.join();
    ASSERT_TRUE(gqueue.empty());
    for (size_t i = 0;i < 100000; ++i) {
      ASSERT_EQ(counter[i], nthreads);
    }
  std::cout << 3 * nthreads * 100000 << " acquired and released." << std::endl;
}