#pragma once

#include <vector>
#include <unordered_map>

class TeamTasks {
public:
  // Получить статистику по статусам задач конкретного разработчика
  const TasksInfo& GetPersonTasksInfo(const string& person) const {
    return m_data.at(person);
  }
  // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
  void AddNewTask(const string& person) {
    m_data[person][TaskStatus::NEW]++;
  }

  // Обновить статусы по данному количеству задач конкретного разработчика,
  // подробности см. ниже
  tuple<TasksInfo, TasksInfo> PerformPersonTasks(
          const string& person, int task_count) {
    TasksInfo & tasksInfo = m_data[person];
    TasksInfo untouched;
    TasksInfo updated;

    for (int i = 0; i < status_flow.size(); i++) {
      auto status = status_flow[i];
      if (tasksInfo.find(status) == tasksInfo.end()) {
        continue;
      }

      int pers_task = tasksInfo[status];
      int push_task = task_count <= pers_task ? task_count : pers_task;
      int left_task = status != TaskStatus::DONE ? pers_task - push_task : pers_task;
      task_count -= push_task;

      untouched[status] = left_task;
      if (status != TaskStatus::DONE) {
        updated[status_flow[i + 1]] = push_task;
      }
    }

    for (auto status : status_flow) {
      tasksInfo[status] = untouched[status] + updated[status];
    }

    ClearZeros(tasksInfo);
    ClearZeros(untouched);
    ClearZeros(updated);

    untouched.erase(TaskStatus::DONE);
    return {updated, untouched};
  }

private:
  void ClearZeros(TasksInfo & tasksInfo) {
    for (int i = 0; i < 4; i++) {
      auto status = status_flow[i];

      if (auto it = tasksInfo.find(status); it != tasksInfo.end() && it->second == 0) {
        tasksInfo.erase(it);
      }
    }
  }

  std::unordered_map<std::string, TasksInfo> m_data;
  std::vector<TaskStatus> status_flow = {
          TaskStatus::NEW,          // новая
          TaskStatus::IN_PROGRESS,  // в разработке
          TaskStatus::TESTING,      // на тестировании
          TaskStatus::DONE          // завершена
  };
};