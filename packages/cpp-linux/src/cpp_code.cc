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
        // Process any pending GTK events
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, TRUE);
        }
    }
}

static void edit_action(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    auto* builder = static_cast<GtkBuilder*>(user_data);
    auto* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));
    auto* row = gtk_list_box_get_selected_row(list);
    if (row) {
        int index = gtk_list_box_row_get_index(row);
        if (index >= 0 && index < g_todos.size()) {
            // Create dialog
            auto* dialog = gtk_dialog_new_with_buttons(
                "Edit Todo",
                GTK_WINDOW(gtk_builder_get_object(builder, "window")),
                GTK_DIALOG_MODAL,
                "_Cancel",
                GTK_RESPONSE_CANCEL,
                "_Save",
                GTK_RESPONSE_ACCEPT,
                NULL
            );

            // Create content area
            auto* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);

            // Create entry with current todo text
            auto* entry = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(entry), g_todos[index].text.c_str());
            gtk_container_add(GTK_CONTAINER(content_area), entry);

            // Create calendar with current date
            auto* calendar = gtk_calendar_new();
            time_t unix_time = g_todos[index].date / 1000;
            struct tm* timeinfo = localtime(&unix_time);
            gtk_calendar_select_month(GTK_CALENDAR(calendar), timeinfo->tm_mon, timeinfo->tm_year + 1900);
            gtk_calendar_select_day(GTK_CALENDAR(calendar), timeinfo->tm_mday);
            gtk_container_add(GTK_CONTAINER(content_area), calendar);

            gtk_widget_show_all(dialog);

            // Handle response
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            if (result == GTK_RESPONSE_ACCEPT) {
                const char* new_text = gtk_entry_get_text(GTK_ENTRY(entry));

                // Get new date
                guint year, month, day;
                gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
                GDateTime* datetime = g_date_time_new_local(year, month + 1, day, 0, 0, 0);
                gint64 new_date = g_date_time_to_unix(datetime) * 1000;
                g_date_time_unref(datetime);

                // Update todo
                g_todos[index].text = new_text;
                g_todos[index].date = new_date;

                // Update UI
                char date_str[64];
                time_t new_unix_time = new_date / 1000;
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&new_unix_time));

                auto* label = gtk_label_new((g_todos[index].text + " - " + date_str).c_str());
                auto* old_label = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(row))->data);
                gtk_container_remove(GTK_CONTAINER(row), old_label);
                gtk_container_add(GTK_CONTAINER(row), label);
                gtk_widget_show_all(GTK_WIDGET(row));

                // Notify about update using the helper function
                notify_callback(g_todoUpdatedCallback, g_todos[index].toJson());
            }

            gtk_widget_destroy(dialog);
        }
    }
}

static void delete_action(GSimpleAction* action, GVariant* parameter, gpointer user_data) {
    auto* builder = static_cast<GtkBuilder*>(user_data);
    auto* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));
    auto* row = gtk_list_box_get_selected_row(list);
    if (row) {
        int index = gtk_list_box_row_get_index(row);
        if (index >= 0 && index < g_todos.size()) {
            // Remove from UI
            gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));

            // Notify about deletion
            std::string json = g_todos[index].toJson();

            // Remove from vector
            g_todos.erase(g_todos.begin() + index);

            notify_callback(g_todoDeletedCallback, json);
        }
    }
}

static void on_add_clicked(GtkButton* button, gpointer user_data) {
    auto* builder = static_cast<GtkBuilder*>(user_data);
    auto* entry = GTK_ENTRY(gtk_builder_get_object(builder, "todo_entry"));
    auto* calendar = GTK_CALENDAR(gtk_builder_get_object(builder, "todo_calendar"));
    auto* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));

    const char* text = gtk_entry_get_text(entry);
    if (strlen(text) > 0) {
        TodoItem todo;
        uuid_generate(todo.id);
        todo.text = text;

        // Get date from calendar (GTK3 style)
        guint year, month, day;
        gtk_calendar_get_date(calendar, &year, &month, &day);
        GDateTime* datetime = g_date_time_new_local(year, month + 1, day, 0, 0, 0);
        todo.date = g_date_time_to_unix(datetime) * 1000; // Convert to milliseconds
        g_date_time_unref(datetime);

        g_todos.push_back(todo);

        // Create list item
        char date_str[64];
        time_t unix_time = todo.date / 1000;
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&unix_time));

        auto* row = gtk_list_box_row_new();
        auto* label = gtk_label_new((todo.text + " - " + date_str).c_str());
        gtk_container_add(GTK_CONTAINER(row), label);
        gtk_container_add(GTK_CONTAINER(list), row);
        gtk_widget_show_all(row);

        // Clear entry
        gtk_entry_set_text(entry, "");

        // Notify about addition using the helper function
        notify_callback(g_todoAddedCallback, todo.toJson());
    }
}

static void on_row_activated(GtkListBox* list_box, GtkListBoxRow* row, gpointer user_data) {
    GMenu* menu = g_menu_new();
    g_menu_append(menu, "Edit", "app.edit");
    g_menu_append(menu, "Delete", "app.delete");

    auto* popover = gtk_popover_new_from_model(GTK_WIDGET(row), G_MENU_MODEL(menu));
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_RIGHT);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
}

static void activate_handler(GtkApplication* app, gpointer user_data) {
    auto* builder = gtk_builder_new();

    // Add actions
    const GActionEntry app_actions[] = {
        { "edit", edit_action, NULL, NULL, NULL },
        { "delete", delete_action, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions,
                                  G_N_ELEMENTS(app_actions), builder);

    gtk_builder_add_from_string(builder,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<interface>"
        "  <object class=\"GtkWindow\" id=\"window\">"
        "    <property name=\"title\">Todo List</property>"
        "    <property name=\"default-width\">400</property>"
        "    <property name=\"default-height\">500</property>"
        "    <child>"
        "      <object class=\"GtkBox\">"
        "        <property name=\"visible\">true</property>"
        "        <property name=\"orientation\">vertical</property>"
        "        <property name=\"spacing\">6</property>"
        "        <property name=\"margin\">12</property>"
        "        <child>"
        "          <object class=\"GtkBox\">"
        "            <property name=\"visible\">true</property>"
        "            <property name=\"spacing\">6</property>"
        "            <child>"
        "              <object class=\"GtkEntry\" id=\"todo_entry\">"
        "                <property name=\"visible\">true</property>"
        "                <property name=\"hexpand\">true</property>"
        "                <property name=\"placeholder-text\">Enter todo item...</property>"
        "              </object>"
        "            </child>"
        "            <child>"
        "              <object class=\"GtkCalendar\" id=\"todo_calendar\">"
        "                <property name=\"visible\">true</property>"
        "              </object>"
        "            </child>"
        "            <child>"
        "              <object class=\"GtkButton\" id=\"add_button\">"
        "                <property name=\"visible\">true</property>"
        "                <property name=\"label\">Add</property>"
        "              </object>"
        "            </child>"
        "          </object>"
        "        </child>"
        "        <child>"
        "          <object class=\"GtkScrolledWindow\">"
        "            <property name=\"visible\">true</property>"
        "            <property name=\"vexpand\">true</property>"
        "            <child>"
        "              <object class=\"GtkListBox\" id=\"todo_list\">"
        "                <property name=\"visible\">true</property>"
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

    gtk_widget_show_all(GTK_WIDGET(window));
}

void hello_gui() {
    GtkApplication* app = gtk_application_new("com.example.todo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate_handler), nullptr);
    g_application_run(G_APPLICATION(app), 0, nullptr);
    g_object_unref(app);
}

} // namespace cpp_code
