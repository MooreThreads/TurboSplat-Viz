#include"threadspool.h"

void AddTask(const std::function<void>& func);
void EnqueueTask(const std::function<void>& func);