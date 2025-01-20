#include <string>
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#pragma comment(lib, "comctl32.lib")

namespace cpp_code {

std::string hello_world(const std::string& input) {
    return "Hello from C++! You said: " + input;
}

void hello_gui() {
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("SampleWindow");
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(
        TEXT("SampleWindow"),
        TEXT("Sample Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    // Create Button
    HWND hwndButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("Click Me"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 100, 30,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Create DateTimePicker
    HWND hwndDate = CreateWindow(
        DATETIMEPICK_CLASS,
        TEXT(""),
        WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT,
        10, 50, 100, 30,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Create Edit Control
    HWND hwndEdit = CreateWindow(
        TEXT("EDIT"),
        TEXT("Edit me"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        10, 90, 100, 30,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Create ListView
    HWND hwndListView = CreateWindow(
        WC_LISTVIEW,
        TEXT(""),
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        10, 130, 600, 200,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Add ListView Columns
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    TCHAR column1[] = TEXT("Column 1");
    TCHAR column2[] = TEXT("Column 2");
    TCHAR column3[] = TEXT("Column 3");
    
    lvc.cx = 100;
    lvc.pszText = column1;
    ListView_InsertColumn(hwndListView, 0, &lvc);

    lvc.cx = 100;
    lvc.pszText = column2;
    ListView_InsertColumn(hwndListView, 1, &lvc);

    lvc.cx = 100;
    lvc.pszText = column3;
    ListView_InsertColumn(hwndListView, 2, &lvc);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

} // namespace cpp_code
