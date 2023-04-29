#include <iostream>
#include <mutex>
#include <thread>
#include <future>
#include <cstring>

#define THREADS_COUNT   10

/* Number of elements in single portion for multi-thread execution */
#define PORTION_SIZE    1000000

/* Number of elements in whole array */
#define ARRAY_SIZE      (PORTION_SIZE * THREADS_COUNT) 


/* Function-comparator for usage in qsort() */
int compare_func(const void* x, const void* y)
{
  int cmp = *static_cast<const int*>(x) - *static_cast<const int*>(y);
  return (cmp < 0) ? -1 : ((cmp > 0) ? 1 : 0);
}

/* Function for execution in separate thread */
int task_for_max(int a_task, int *a_array, int a_size)
{
  static std::mutex m_cout;

  qsort(a_array, a_size, sizeof(int), compare_func);

  m_cout.lock();
  std::cout << "task #" << a_task << "(thread id #" << std::this_thread::get_id() << ") finished, "
    "maximum is " << a_array[a_size - 1] << std::endl;
  m_cout.unlock();

  return a_array[a_size - 1];
}

/* Function for array initializantion (with using random values) */
static void init_array(int *a_array, int a_size)
{
  srand((unsigned int) time(NULL));

  for (int i = 0; i < a_size; i++)
    a_array[i] = (int) (rand() * rand());
}

int main()
{
  int *a1 = new int[ARRAY_SIZE], *a2 = new int[ARRAY_SIZE];
  int res1, res2;
  clock_t t1, t2;

  /* Initialize test arrays */
  init_array(a1, ARRAY_SIZE);
  memcpy(a2, a1, ARRAY_SIZE * sizeof(int));

  /* Multi-thread sorting */
  t1 = clock();
  std::future<int> futures[THREADS_COUNT];
  for (int i = 0; i < THREADS_COUNT; i++)
    futures[i] = std::async(std::launch::async, task_for_max, i, &a1[i * PORTION_SIZE], PORTION_SIZE);

  int results[THREADS_COUNT];
  for (int i = 0; i < THREADS_COUNT; i++)
    results[i] = futures[i].get();

  qsort(results, THREADS_COUNT, sizeof(int), compare_func);
  res1 = results[THREADS_COUNT - 1];
  t2 = clock();
  std::cout << std::endl << THREADS_COUNT << "-thread sorting: " << t2 - t1 << " ticks, result: " << res1 << std::endl;

  /* Single-thread sorting */
  t1 = clock();
  qsort(a2, ARRAY_SIZE, sizeof(int), compare_func);
  res2 = a2[ARRAY_SIZE - 1];
  t2 = clock();
  std::cout << std::endl << "1-thread sorting: " << t2 - t1 << " ticks, result: " << res2 << std::endl;

  delete [] a1;
  delete [] a2;

  return 0;
}
