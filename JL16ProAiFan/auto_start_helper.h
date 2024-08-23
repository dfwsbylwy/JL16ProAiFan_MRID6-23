#pragma once

bool create_auto_start_task_for_this_user(bool runElevated = FALSE);
bool delete_auto_start_task_for_this_user();
//bool is_auto_start_task_active_for_this_user(std::wstring* path);