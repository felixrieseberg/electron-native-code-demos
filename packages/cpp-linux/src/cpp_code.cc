#include <gtk/gtk.h>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include <uuid/uuid.h>
#include <ctime>
#include <thread>
#include <memory>

using TodoCallback = std::function<void(const std::string &)>;

namespace cpp_code
{

  // Basic functions
  std::string hello_world(const std::string &input)
  {
    return "Hello from C++! You said: " + input;
  }

  // Data structures
  struct TodoItem
  {
    uuid_t id;
    std::string text;
    int64_t date;

    std::string toJson() const
    {
      char uuid_str[37];
      uuid_unparse(id, uuid_str);
      return "{"
             "\"id\":\"" +
             std::string(uuid_str) + "\","
                                     "\"text\":\"" +
             text + "\","
                    "\"date\":" +
             std::to_string(date) +
             "}";
    }

    static std::string formatDate(int64_t timestamp)
    {
      char date_str[64];
      time_t unix_time = timestamp / 1000;
      strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&unix_time));
      return date_str;
    }
  };

  // Forward declarations
  static void update_todo_row_label(GtkListBoxRow *row, const TodoItem &todo);
  static GtkWidget *create_todo_dialog(GtkWindow *parent, const TodoItem *existing_todo);

  // Global state
  namespace
  {
    TodoCallback g_todoAddedCallback;
    TodoCallback g_todoUpdatedCallback;
    TodoCallback g_todoDeletedCallback;
    GMainContext *g_gtk_main_context = nullptr;
    GMainLoop *g_main_loop = nullptr;
    std::thread *g_gtk_thread = nullptr;
    std::vector<TodoItem> g_todos;
  }

  // Helper functions
  static void notify_callback(const TodoCallback &callback, const std::string &json)
  {
    if (callback && g_gtk_main_context)
    {
      g_main_context_invoke(g_gtk_main_context, [](gpointer data) -> gboolean
                            {
            auto* cb_data = static_cast<std::pair<TodoCallback, std::string>*>(data);
            cb_data->first(cb_data->second);
            delete cb_data;
            return G_SOURCE_REMOVE; }, new std::pair<TodoCallback, std::string>(callback, json));
    }
  }

  static void update_todo_row_label(GtkListBoxRow *row, const TodoItem &todo)
  {
    auto *label = gtk_label_new((todo.text + " - " + TodoItem::formatDate(todo.date)).c_str());
    auto *old_label = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(row))->data);
    gtk_container_remove(GTK_CONTAINER(row), old_label);
    gtk_container_add(GTK_CONTAINER(row), label);
    gtk_widget_show_all(GTK_WIDGET(row));
  }

  static GtkWidget *create_todo_dialog(GtkWindow *parent, const TodoItem *existing_todo = nullptr)
  {
    auto *dialog = gtk_dialog_new_with_buttons(
        existing_todo ? "Edit Todo" : "Add Todo",
        parent,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        nullptr);

    auto *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);

    auto *entry = gtk_entry_new();
    if (existing_todo)
    {
      gtk_entry_set_text(GTK_ENTRY(entry), existing_todo->text.c_str());
    }
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    auto *calendar = gtk_calendar_new();
    if (existing_todo)
    {
      time_t unix_time = existing_todo->date / 1000;
      struct tm *timeinfo = localtime(&unix_time);
      gtk_calendar_select_month(GTK_CALENDAR(calendar), timeinfo->tm_mon, timeinfo->tm_year + 1900);
      gtk_calendar_select_day(GTK_CALENDAR(calendar), timeinfo->tm_mday);
    }
    gtk_container_add(GTK_CONTAINER(content_area), calendar);

    gtk_widget_show_all(dialog);
    return dialog;
  }

  static void edit_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
  {
    auto *builder = static_cast<GtkBuilder *>(user_data);
    auto *list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));
    auto *row = gtk_list_box_get_selected_row(list);
    if (!row)
      return;

    gint index = gtk_list_box_row_get_index(row);
    auto size = static_cast<gint>(g_todos.size());
    if (index < 0 || index >= size)
      return;

    auto *dialog = create_todo_dialog(
        GTK_WINDOW(gtk_builder_get_object(builder, "window")),
        &g_todos[index]);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
      auto *entry = GTK_ENTRY(gtk_container_get_children(
                                  GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))))
                                  ->data);
      auto *calendar = GTK_CALENDAR(gtk_container_get_children(
                                        GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))))
                                        ->next->data);

      const char *new_text = gtk_entry_get_text(entry);

      guint year, month, day;
      gtk_calendar_get_date(calendar, &year, &month, &day);
      GDateTime *datetime = g_date_time_new_local(year, month + 1, day, 0, 0, 0);
      gint64 new_date = g_date_time_to_unix(datetime) * 1000;
      g_date_time_unref(datetime);

      g_todos[index].text = new_text;
      g_todos[index].date = new_date;

      update_todo_row_label(row, g_todos[index]);
      notify_callback(g_todoUpdatedCallback, g_todos[index].toJson());
    }

    gtk_widget_destroy(dialog);
  }

  static void delete_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
  {
    auto *builder = static_cast<GtkBuilder *>(user_data);
    auto *list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));
    auto *row = gtk_list_box_get_selected_row(list);
    if (!row)
      return;

    gint index = gtk_list_box_row_get_index(row);
    auto size = static_cast<gint>(g_todos.size());
    if (index < 0 || index >= size)
      return;

    std::string json = g_todos[index].toJson();
    gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));
    g_todos.erase(g_todos.begin() + index);
    notify_callback(g_todoDeletedCallback, json);
  }

  static void on_add_clicked(GtkButton *button, gpointer user_data)
  {
    auto *builder = static_cast<GtkBuilder *>(user_data);
    auto *entry = GTK_ENTRY(gtk_builder_get_object(builder, "todo_entry"));
    auto *calendar = GTK_CALENDAR(gtk_builder_get_object(builder, "todo_calendar"));
    auto *list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));

    const char *text = gtk_entry_get_text(entry);
    if (strlen(text) > 0)
    {
      TodoItem todo;
      uuid_generate(todo.id);
      todo.text = text;

      guint year, month, day;
      gtk_calendar_get_date(calendar, &year, &month, &day);
      GDateTime *datetime = g_date_time_new_local(year, month + 1, day, 0, 0, 0);
      todo.date = g_date_time_to_unix(datetime) * 1000;
      g_date_time_unref(datetime);

      g_todos.push_back(todo);

      auto *row = gtk_list_box_row_new();
      auto *label = gtk_label_new((todo.text + " - " + TodoItem::formatDate(todo.date)).c_str());
      gtk_container_add(GTK_CONTAINER(row), label);
      gtk_container_add(GTK_CONTAINER(list), row);
      gtk_widget_show_all(row);

      gtk_entry_set_text(entry, "");

      notify_callback(g_todoAddedCallback, todo.toJson());
    }
  }

  static void on_row_activated(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data)
  {
    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Edit", "app.edit");
    g_menu_append(menu, "Delete", "app.delete");

    auto *popover = gtk_popover_new_from_model(GTK_WIDGET(row), G_MENU_MODEL(menu));
    gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_RIGHT);
    gtk_popover_popup(GTK_POPOVER(popover));

    g_object_unref(menu);
  }

  static gboolean init_gtk_app(gpointer user_data)
  {
    auto *app = static_cast<GtkApplication *>(user_data);
    g_application_run(G_APPLICATION(app), 0, nullptr);
    g_object_unref(app);
    if (g_main_loop)
    {
      g_main_loop_quit(g_main_loop);
    }
    return G_SOURCE_REMOVE;
  }

  static void activate_handler(GtkApplication *app, gpointer user_data)
  {
    auto *builder = gtk_builder_new();

    const GActionEntry app_actions[] = {
        {"edit", edit_action, nullptr, nullptr, nullptr, {0, 0, 0}},
        {"delete", delete_action, nullptr, nullptr, nullptr, {0, 0, 0}}};
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
                                "</interface>",
                                -1, nullptr);

    auto *window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    auto *button = GTK_BUTTON(gtk_builder_get_object(builder, "add_button"));
    auto *list = GTK_LIST_BOX(gtk_builder_get_object(builder, "todo_list"));

    gtk_window_set_application(window, app);

    g_signal_connect(button, "clicked", G_CALLBACK(on_add_clicked), builder);
    g_signal_connect(list, "row-activated", G_CALLBACK(on_row_activated), nullptr);

    gtk_widget_show_all(GTK_WIDGET(window));
  }

  void hello_gui()
  {
    if (g_gtk_thread != nullptr)
    {
      g_print("GTK application is already running.\n");
      return;
    }

    if (!gtk_init_check(0, nullptr))
    {
      g_print("Failed to initialize GTK.\n");
      return;
    }

    g_gtk_main_context = g_main_context_new();
    g_main_loop = g_main_loop_new(g_gtk_main_context, FALSE);

    g_gtk_thread = new std::thread([]()
                                   {
        GtkApplication* app = gtk_application_new("com.example.todo", G_APPLICATION_NON_UNIQUE);
        g_signal_connect(app, "activate", G_CALLBACK(activate_handler), nullptr);

        g_idle_add_full(G_PRIORITY_DEFAULT, init_gtk_app, app, nullptr);

        if (g_main_loop) {
            g_main_loop_run(g_main_loop);
        } });

    g_gtk_thread->detach();
  }

  void cleanup_gui()
  {
    if (g_main_loop && g_main_loop_is_running(g_main_loop))
    {
      g_main_loop_quit(g_main_loop);
    }

    if (g_main_loop)
    {
      g_main_loop_unref(g_main_loop);
      g_main_loop = nullptr;
    }

    if (g_gtk_main_context)
    {
      g_main_context_unref(g_gtk_main_context);
      g_gtk_main_context = nullptr;
    }

    g_gtk_thread = nullptr;
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

} // namespace cpp_code
