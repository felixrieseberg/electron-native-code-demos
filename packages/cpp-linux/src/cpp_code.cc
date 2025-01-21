#include <gtk/gtk.h>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include <uuid/uuid.h>
#include <ctime>

using TodoCallback = std::function<void(const std::string&)>;

static TodoCallback g_todoAddedCallback;
static TodoCallback g_todoUpdatedCallback;
static TodoCallback g_todoDeletedCallback;

// Helper function to get current time in milliseconds since epoch
int64_t getCurrentTimeMillis() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}

struct TodoItem {
    uuid_t id;
    std::string text;
    int64_t date;

    std::string toJson() const {
        char uuid_str[37];
        uuid_unparse(id, uuid_str);

        return "{"
               "\"id\":\"" + std::string(uuid_str) + "\","
               "\"text\":\"" + text + "\","
               "\"date\":" + std::to_string(date) +
               "}";
    }
};

// Global vector to store todos
static std::vector<TodoItem> g_todos;

namespace cpp_code {

void setTodoAddedCallback(TodoCallback callback) {
    g_todoAddedCallback = callback;
}

void setTodoUpdatedCallback(TodoCallback callback) {
    g_todoUpdatedCallback = callback;
}

void setTodoDeletedCallback(TodoCallback callback) {
    g_todoDeletedCallback = callback;
}

static void notify_callback(const TodoCallback& callback, const std::string& json) {
    if (callback) {
        callback(json);
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, TRUE);
        }
    }
}

static void on_add_clicked(GtkButton* button, gpointer user_data) {
    auto* builder = static_cast<GtkBuilder*>(user_data);
    auto* entry = GTK_ENTRY(gtk_builder_get_object(builder, "todo_entry"));
    auto* calendar = GTK_CALENDAR(gtk_builder_get_object(builder, "todo_calendar"));
    auto* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));

    const char* text = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (strlen(text) > 0) {
        TodoItem todo;
        uuid_generate(todo.id);
        todo.text = text;

        GDateTime* date = gtk_calendar_get_date(calendar);
        todo.date = g_date_time_to_unix(date) * 1000; // Convert to milliseconds

        g_todos.push_back(todo);

        // Create list item
        char date_str[64];
        time_t unix_time = todo.date / 1000;
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&unix_time));

        auto* row = gtk_list_box_row_new();
        auto* label = gtk_label_new((todo.text + " - " + date_str).c_str());
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        gtk_list_box_append(list, row);

        // Clear entry
        gtk_editable_set_text(GTK_EDITABLE(entry), "");

        notify_callback(g_todoAddedCallback, todo.toJson());
    }
}

static void on_row_activated(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data) {
    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Edit", "app.edit");
    g_menu_append(menu, "Delete", "app.delete");

    auto* popover = GTK_POPOVER(gtk_popover_menu_new_from_model(G_MENU_MODEL(menu)));
    gtk_popover_set_position(popover, GTK_POS_RIGHT);
    gtk_widget_set_parent(GTK_WIDGET(popover), GTK_WIDGET(row));
    gtk_popover_popup(popover);

    g_object_unref(menu);
}

static void activate_handler(GtkApplication* app, gpointer user_data) {
    auto* builder = gtk_builder_new();
    gtk_builder_add_from_string(builder,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<interface>"
        "  <object class=\"GtkApplicationWindow\" id=\"window\">"
        "    <property name=\"title\">Todo List</property>"
        "    <property name=\"default-width\">400</property>"
        "    <property name=\"default-height\">500</property>"
        "    <child>"
        "      <object class=\"GtkBox\">"
        "        <property name=\"orientation\">vertical</property>"
        "        <property name=\"spacing\">6</property>"
        "        <property name=\"margin-start\">12</property>"
        "        <property name=\"margin-end\">12</property>"
        "        <property name=\"margin-top\">12</property>"
        "        <property name=\"margin-bottom\">12</property>"
        "        <child>"
        "          <object class=\"GtkBox\">"
        "            <property name=\"spacing\">6</property>"
        "            <child>"
        "              <object class=\"GtkEntry\" id=\"todo_entry\">"
        "                <property name=\"hexpand\">true</property>"
        "                <property name=\"placeholder-text\">Enter todo item...</property>"
        "              </object>"
        "            </child>"
        "            <child>"
        "              <object class=\"GtkCalendar\" id=\"todo_calendar\">"
        "              </object>"
        "            </child>"
        "            <child>"
        "              <object class=\"GtkButton\" id=\"add_button\">"
        "                <property name=\"label\">Add</property>"
        "              </object>"
        "            </child>"
        "          </object>"
        "        </child>"
        "        <child>"
        "          <object class=\"GtkScrolledWindow\">"
        "            <property name=\"vexpand\">true</property>"
        "            <child>"
        "              <object class=\"GtkListBox\" id=\"todo_list\">"
        "                <property name=\"selection-mode\">single</property>"
        "              </object>"
        "            </child>"
        "          </object>"
        "        </child>"
        "      </object>"
        "    </child>"
        "  </object>"
        "</interface>", -1, nullptr);

    auto* window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    auto* button = GTK_BUTTON(gtk_builder_get_object(builder, "add_button"));
    auto* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));

    gtk_window_set_application(window, app);

    g_signal_connect(button, "clicked", G_CALLBACK(on_add_clicked), builder);
    g_signal_connect(list, "row-activated", G_CALLBACK(on_row_activated), nullptr);

    gtk_window_present(window);
}

void hello_gui() {
    GtkApplication* app = gtk_application_new("com.example.todo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate_handler), nullptr);
    g_application_run(G_APPLICATION(app), 0, nullptr);
    g_object_unref(app);
}

} // namespace cpp_code
