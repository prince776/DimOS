#pragma once
#include <kernel/cpp/vector.hpp>
#include <kernel/process/kthread.h>

template <typename T> class RRScheduler {
  public:
    static RRScheduler<T>& get() {
        static RRScheduler<T> scheduler;
        return scheduler;
    }
    /**
     * @brief Get the Next task index to schedule
     * if returned value == tasks.size(), then no
     * task was scheduled/
     *
     * @param tasks
     * @param prevTask
     * @return size_t
     */
    size_t getNext(Vector<T>& tasks, T& prevTask);

  private:
    RRScheduler() = default;
    RRScheduler(const RRScheduler&);
    void operator=(const RRScheduler&);
};

template <typename T> size_t RRScheduler<T>::getNext(Vector<T>& tasks, T& prevTask) {
    size_t prevIndex = tasks.size();
    for (size_t i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == prevTask.id) {
            prevIndex = i;
            break;
        }
    }
    bool isIdxValid = prevIndex < tasks.size();
    if (isIdxValid && prevTask.state == TaskState::DO_NOT_DISTURB) {
        return prevIndex;
    }
    if (prevTask.state != TaskState::COMPLETED) {
        prevTask.state = TaskState::RUNNING;
    }

    if (prevTask.state == TaskState::COMPLETED && isIdxValid) { // remove the task
        bool swapToEnd = prevIndex != (tasks.size() - 1);
        if (swapToEnd) {
            swap(tasks[prevIndex], tasks[tasks.size() - 1]);
        }
        // TODO: shared var update(or not?)
        tasks.pop_back();
    }
    if (!tasks.size()) {
        return 0;
    }
    size_t nextIdx = (prevIndex + 1) % tasks.size(), tried = 0;
    while (!isRunnableState(tasks[nextIdx].state) && tried < tasks.size()) {
        nextIdx = (nextIdx + 1) % tasks.size();
        tried++;
    }
    if (tried == tasks.size()) {
        return tasks.size();
    }
    return nextIdx;
}
