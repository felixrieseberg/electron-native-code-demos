#include <windows.h>
#include <windowsx.h>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include <commctrl.h>
#include <shellscalingapi.h>
#include <thread>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using TodoCallback = std::function<void(const std::string &)>;

static TodoCallback g_todoAddedCallback;
static TodoCallback g_todoUpdatedCallback;
static TodoCallback g_todoDeletedCallback;

// Helper function to get the current time in milliseconds since epoch
int64_t getCurrentTimeMillis()
{
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}

struct TodoItem
{
  GUID id;
  std::wstring text;
  int64_t date;

  std::string toJson() const
  {
    OLECHAR *guidString;
    StringFromCLSID(id, &guidString);
    std::wstring widGuid(guidString);
    CoTaskMemFree(guidString);

    // Convert wide string to narrow for JSON
    std::string guidStr(widGuid.begin(), widGuid.end());
    std::string textStr(text.begin(), text.end());

    return "{"
           "\"id\":\"" + guidStr + "\","
           "\"text\":\"" + textStr + "\","
           "\"date\":" + std::to_string(date) +
           "}";
  }
};

namespace cpp_code
{

  std::string hello_world(const std::string &input)
  {
    return "Hello from C++! You said: " + input;
  }

  void setTodoAddedCallback(TodoCallback callback)
  {
    g_todoAddedCallback = callback;
  }

  void setTodoUpdatedCallback(TodoCallback callback)
  {
    g_todoUpdatedCallback = callback;
  }

  void setTodoDeletedCallback(TodoCallback callback)
  {
    g_todoDeletedCallback = callback;
  }

  LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  // Helper function to scale a value based on DPI
  int Scale(int value, UINT dpi)
  {
    return MulDiv(value, dpi, 96); // 96 is the default DPI
  }

  // Helper function to convert SYSTEMTIME to milliseconds since epoch
  int64_t SystemTimeToMillis(const SYSTEMTIME &st)
  {
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10000;
  }

  void ShowEditControls(HWND hwnd, const TodoItem &todo)
  {
    HWND hEdit = GetDlgItem(hwnd, 1);
    HWND hDatePicker = GetDlgItem(hwnd, 4);
    HWND hAddButton = GetDlgItem(hwnd, 2);
    HWND hSaveButton = GetDlgItem(hwnd, 5);

    // Set text and date
    SetWindowTextW(hEdit, todo.text.c_str());

    // Convert milliseconds to SYSTEMTIME
    FILETIME ft;
    SYSTEMTIME st;
    ULARGE_INTEGER uli;
    uli.QuadPart = (todo.date * 10000) + 116444736000000000ULL;
    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;
    FileTimeToSystemTime(&ft, &st);
    DateTime_SetSystemtime(hDatePicker, GDT_VALID, &st);

    // Show save button, hide add button
    ShowWindow(hAddButton, SW_HIDE);
    ShowWindow(hSaveButton, SW_SHOW);
  }

  void ResetControls(HWND hwnd)
  {
    HWND hEdit = GetDlgItem(hwnd, 1);
    HWND hDatePicker = GetDlgItem(hwnd, 4);
    HWND hAddButton = GetDlgItem(hwnd, 2);
    HWND hSaveButton = GetDlgItem(hwnd, 5);

    // Clear text
    SetWindowTextW(hEdit, L"");

    // Reset date to current
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);
    DateTime_SetSystemtime(hDatePicker, GDT_VALID, &currentTime);

    // Show add button, hide save button
    ShowWindow(hAddButton, SW_SHOW);
    ShowWindow(hSaveButton, SW_HIDE);
  }

  void hello_gui()
  {
    // Launch GUI in a separate thread
    std::thread guiThread([]()
                          {
            // Enable Per-Monitor DPI awareness
            SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

            // Initialize Common Controls
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
            InitCommonControlsEx(&icex);

            // Register window class
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = L"TodoApp";
            RegisterClassExW(&wc);

            // Get the DPI for the monitor
            UINT dpi = GetDpiForSystem();

            // Create window
            HWND hwnd = CreateWindowExW(
                0, L"TodoApp", L"Todo List",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                Scale(500, dpi), Scale(500, dpi),
                nullptr, nullptr,
                GetModuleHandle(nullptr), nullptr
            );

            if (hwnd == nullptr) {
                return;
            }

            // Create the modern font with DPI-aware size
            HFONT hFont = CreateFontW(
                -Scale(14, dpi),           // Height (scaled)
                0,                         // Width
                0,                         // Escapement
                0,                         // Orientation
                FW_NORMAL,                // Weight
                FALSE,                    // Italic
                FALSE,                    // Underline
                FALSE,                    // StrikeOut
                DEFAULT_CHARSET,          // CharSet
                OUT_DEFAULT_PRECIS,       // OutPrecision
                CLIP_DEFAULT_PRECIS,      // ClipPrecision
                CLEARTYPE_QUALITY,        // Quality
                DEFAULT_PITCH | FF_DONTCARE, // Pitch and Family
                L"Segoe UI"               // Font face name
            );

            // Create input controls with scaled positions and sizes
            HWND hEdit = CreateWindowExW(0, WC_EDITW, L"", 
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                Scale(10, dpi), Scale(10, dpi),
                Scale(250, dpi), Scale(25, dpi),
                hwnd, (HMENU)1, GetModuleHandle(nullptr), nullptr);
            SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create date picker
            HWND hDatePicker = CreateWindowExW(0, DATETIMEPICK_CLASSW, L"",
                WS_CHILD | WS_VISIBLE | DTS_SHORTDATECENTURYFORMAT,
                Scale(270, dpi), Scale(10, dpi),
                Scale(100, dpi), Scale(25, dpi),
                hwnd, (HMENU)4, GetModuleHandle(nullptr), nullptr);
            SendMessageW(hDatePicker, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hButton = CreateWindowExW(0, WC_BUTTONW, L"Add",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                Scale(380, dpi), Scale(10, dpi),
                Scale(50, dpi), Scale(25, dpi),
                hwnd, (HMENU)2, GetModuleHandle(nullptr), nullptr);
            SendMessageW(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hListBox = CreateWindowExW(0, WC_LISTBOXW, L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                Scale(10, dpi), Scale(45, dpi),
                Scale(460, dpi), Scale(400, dpi),
                hwnd, (HMENU)3, GetModuleHandle(nullptr), nullptr);
            SendMessageW(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Create edit button (initially hidden)
            HWND hSaveButton = CreateWindowExW(0, WC_BUTTONW, L"Save",
                WS_CHILD | BS_PUSHBUTTON,  // Note: Not visible initially
                Scale(380, dpi), Scale(10, dpi),
                Scale(50, dpi), Scale(25, dpi),
                hwnd, (HMENU)5, GetModuleHandle(nullptr), nullptr);
            SendMessageW(hSaveButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Add context menu for the listbox
            HMENU hContextMenu = CreatePopupMenu();
            AppendMenuW(hContextMenu, MF_STRING, 6, L"Edit");
            AppendMenuW(hContextMenu, MF_STRING, 7, L"Delete");

            // Store menu handle in window's user data
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContextMenu);

            ShowWindow(hwnd, SW_SHOW);

            // Message loop
            MSG msg = {};
            while (GetMessage(&msg, nullptr, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Clean up
            DeleteObject(hFont); });

    // Detach the thread so it runs independently
    guiThread.detach();
  }

  // Global vector to store todos
  static std::vector<TodoItem> g_todos;

  void NotifyCallback(const TodoCallback &callback, const std::string &json)
  {
    if (callback)
    {
      callback(json);
      // Process pending messages
      MSG msg;
      while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  std::wstring FormatTodoDisplay(const std::wstring &text, const SYSTEMTIME &st)
  {
    wchar_t dateStr[64];
    GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, nullptr, dateStr, 64);
    return text + L" - " + dateStr;
  }

  LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    static GUID editingTodoId = GUID_NULL;

    switch (uMsg)
    {
    case WM_CONTEXTMENU:
    {
      HWND hListBox = GetDlgItem(hwnd, 3);
      if ((HWND)wParam == hListBox)
      {
        int index = SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
        if (index != LB_ERR)
        {
          POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
          HMENU hMenu = (HMENU)GetWindowLongPtr(hwnd, GWLP_USERDATA);
          TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        }
      }
      break;
    }

    case WM_COMMAND:
    {
      HWND hListBox = GetDlgItem(hwnd, 3);
      int cmd = LOWORD(wParam);

      switch (cmd)
      {
      case 6:
      { // Edit
        int index = SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
        if (index != LB_ERR && index < g_todos.size())
        {
          editingTodoId = g_todos[index].id;
          ShowEditControls(hwnd, g_todos[index]);
        }
        break;
      }

      case 7:
      { // Delete
        int index = SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
        if (index != LB_ERR && index < g_todos.size())
        {
          GUID todoId = g_todos[index].id;
          SendMessageW(hListBox, LB_DELETESTRING, index, 0);
          g_todos.erase(g_todos.begin() + index);

          OLECHAR *guidString;
          StringFromCLSID(todoId, &guidString);
          std::wstring widGuid(guidString);
          CoTaskMemFree(guidString);
          std::string guidStr(widGuid.begin(), widGuid.end());
          NotifyCallback(g_todoDeletedCallback, guidStr);
        }
        break;
      }

      case 5:
      { // Save button
        wchar_t buffer[256];
        GetDlgItemTextW(hwnd, 1, buffer, 256);

        if (wcslen(buffer) > 0)
        {
          auto it = std::find_if(g_todos.begin(), g_todos.end(),
                                 [&](const TodoItem &todo)
                                 { return todo.id == editingTodoId; });

          if (it != g_todos.end())
          {
            SYSTEMTIME st;
            HWND hDatePicker = GetDlgItem(hwnd, 4);
            DateTime_GetSystemtime(hDatePicker, &st);

            it->text = buffer;
            it->date = SystemTimeToMillis(st);

            int index = std::distance(g_todos.begin(), it);
            std::wstring displayText = FormatTodoDisplay(buffer, st);
            SendMessageW(hListBox, LB_DELETESTRING, index, 0);
            SendMessageW(hListBox, LB_INSERTSTRING, index, (LPARAM)displayText.c_str());

            NotifyCallback(g_todoUpdatedCallback, it->toJson());
          }
          ResetControls(hwnd);
          editingTodoId = GUID_NULL;
        }
        break;
      }

      case 2:
      { // Add button
        wchar_t buffer[256];
        GetDlgItemTextW(hwnd, 1, buffer, 256);

        if (wcslen(buffer) > 0)
        {
          SYSTEMTIME st;
          HWND hDatePicker = GetDlgItem(hwnd, 4);
          DateTime_GetSystemtime(hDatePicker, &st);

          TodoItem todo;
          CoCreateGuid(&todo.id);
          todo.text = buffer;
          todo.date = SystemTimeToMillis(st);

          g_todos.push_back(todo);

          std::wstring displayText = FormatTodoDisplay(buffer, st);
          SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)displayText.c_str());

          ResetControls(hwnd);
          NotifyCallback(g_todoAddedCallback, todo.toJson());
        }
        break;
      }
      }
      break;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
  }

} // namespace cpp_code
