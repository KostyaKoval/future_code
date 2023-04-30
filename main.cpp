#include <iostream>
#include <mutex>
#include <thread>
#include <future>
#include <cstring>
#include <climits>

#define THREADS_COUNT   4

/* Number of elements in single portion for multi-thread execution */
#define PORTION_SIZE    10000000

/* Number of elements in whole array */
#define ARRAY_SIZE      (PORTION_SIZE * THREADS_COUNT) 


/* Function for execution in separate thread (async) */
long long task_for_sum(int a_task, int *a_array, int a_size)
{
  static std::mutex v_mutex;
  long long v_sum = 0;

  for (int i = 0; i < a_size; i++)
      v_sum += a_array[i];

  v_mutex.lock();
  std::cout << "task #" << a_task << "(thread id #" << std::this_thread::get_id() << ") finished, "
    "sum is " << v_sum << std::endl;
  v_mutex.unlock();

  return v_sum;
}

/* Function for execution in separate thread (thread) */
void task_for_sum_thread(int a_task, int *a_array, int a_size, long long & v)
{
  v = task_for_sum(a_task, a_array, a_size);
}

/* Function for array initializantion (with using random values) */
static void init_array(int *a_array, int a_size)
{
  srand((unsigned int) time(NULL));

  for (int i = 0; i < a_size; i++)
    a_array[i] = rand();
}

int main()
{
  int *a = new int[ARRAY_SIZE];
  long long res = 0;
  clock_t t1, t2;

  /* Initialize test arrays */
  init_array(a, ARRAY_SIZE);

  /* Multi-thread computing (async) */
  std::future<long long> futures[THREADS_COUNT];

  t1 = clock();
  for (int i = 0; i < THREADS_COUNT; i++)
    futures[i] = std::async(std::launch::async, task_for_sum, i, &a[i * PORTION_SIZE], PORTION_SIZE);
  res = 0;
  for (int i = 0; i < THREADS_COUNT; i++)
    res += futures[i].get();
  t2 = clock();

  std::cout << std::endl << THREADS_COUNT << "-thread working (async): " << t2 - t1 << " ticks, result: " << res << std::endl << std::endl;

  /* Multi-thread computing (threads) */
  std::thread threads[THREADS_COUNT];
  long long p[THREADS_COUNT];

  t1 = clock();
  for (int i = 0; i < THREADS_COUNT; i++)
    threads[i] = std::thread(task_for_sum_thread, i, &a[i * PORTION_SIZE], PORTION_SIZE, std::ref(p[i]));
  for (int i = 0; i < THREADS_COUNT; i++)
    threads[i].join();
  res = 0;
  for (int i = 0; i < THREADS_COUNT; i++)
    res += p[i];
  t2 = clock();

  std::cout << std::endl << THREADS_COUNT << "-thread working (threads): " << t2 - t1 << " ticks, result: " << res << std::endl;

  /* Single-thread computing */
  t1 = clock();
  res = 0;
  for (int i = 0; i < ARRAY_SIZE; i++)
      res += a[i];
  t2 = clock();

  std::cout << std::endl << "1-thread working: " << t2 - t1 << " ticks, result: " << res << std::endl;

  delete [] a;

  return 0;
}
