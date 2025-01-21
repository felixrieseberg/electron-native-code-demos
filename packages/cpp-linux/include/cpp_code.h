#pragma once
#include <string>
#include <functional>

namespace cpp_code {

std::string hello_world(const std::string& input);
void hello_gui();

// Callback function types
using TodoCallback = std::function<void(const std::string&)>;

// Callback setters
void setTodoAddedCallback(TodoCallback callback);
void setTodoUpdatedCallback(TodoCallback callback); 
void setTodoDeletedCallback(TodoCallback callback);

} // namespace cpp_code 
