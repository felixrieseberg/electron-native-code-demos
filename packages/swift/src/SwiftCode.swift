import Foundation
import SwiftUI

@objc
public class SwiftCode: NSObject {
    private static var windowController: NSWindowController?
    private static var todoAddedCallback: ((String) -> Void)?
    private static var todoUpdatedCallback: ((String) -> Void)?
    private static var todoDeletedCallback: ((String) -> Void)?

    @objc
    public static func helloWorld(_ input: String) -> String {
        return "Hello from Swift! You said: \(input)"
    }

    @objc
    public static func setTodoAddedCallback(_ callback: @escaping (String) -> Void) {
        todoAddedCallback = callback
    }

    @objc
    public static func setTodoUpdatedCallback(_ callback: @escaping (String) -> Void) {
        todoUpdatedCallback = callback
    }

    @objc
    public static func setTodoDeletedCallback(_ callback: @escaping (String) -> Void) {
        todoDeletedCallback = callback
    }

    private static func encodeToJson<T: Encodable>(_ item: T) -> String? {
        let encoder = JSONEncoder()

        // Encode date as milliseconds since 1970, which is what the JS side expects
        encoder.dateEncodingStrategy = .custom { date, encoder in
            let milliseconds = Int64(date.timeIntervalSince1970 * 1000)
            var container = encoder.singleValueContainer()
            try container.encode(milliseconds)
        }

        guard let jsonData = try? encoder.encode(item),
              let jsonString = String(data: jsonData, encoding: .utf8) else {
            return nil
        }
        return jsonString
    }

    @objc
    public static func helloGui() -> Void {
        let contentView = NSHostingView(rootView: ContentView(
            onTodoAdded: { todo in
                if let jsonString = encodeToJson(todo) {
                    todoAddedCallback?(jsonString)
                }
            },
            onTodoUpdated: { todo in
                if let jsonString = encodeToJson(todo) {
                    todoUpdatedCallback?(jsonString)
                }
            },
            onTodoDeleted: { todoId in
                todoDeletedCallback?(todoId.uuidString)
            }
        ))
        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 500, height: 500),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )

        window.title = "Todo List"
        window.contentView = contentView
        window.center()

        windowController = NSWindowController(window: window)
        windowController?.showWindow(nil)

        NSApp.activate(ignoringOtherApps: true)
    }

    private struct TodoItem: Identifiable, Codable {
        let id: UUID
        var text: String
        var date: Date

        init(id: UUID = UUID(), text: String, date: Date) {
            self.id = id
            self.text = text
            self.date = date
        }
    }

    private struct ContentView: View {
        @State private var todos: [TodoItem] = []
        @State private var newTodo: String = ""
        @State private var newTodoDate: Date = Date()
        @State private var editingTodo: UUID?
        @State private var editedText: String = ""
        @State private var editedDate: Date = Date()

        let onTodoAdded: (TodoItem) -> Void
        let onTodoUpdated: (TodoItem) -> Void
        let onTodoDeleted: (UUID) -> Void

        private func todoTextField(_ text: Binding<String>, placeholder: String, maxWidth: CGFloat? = nil) -> some View {
            TextField(placeholder, text: text)
                .textFieldStyle(RoundedBorderTextFieldStyle())
                .frame(maxWidth: maxWidth ?? .infinity)
        }

        private func todoDatePicker(_ date: Binding<Date>) -> some View {
            DatePicker("Due date", selection: date, displayedComponents: [.date])
                .datePickerStyle(CompactDatePickerStyle())
                .labelsHidden()
                .frame(width: 100)
                .textFieldStyle(RoundedBorderTextFieldStyle())
        }

        var body: some View {
            VStack(spacing: 16) {
                HStack(spacing: 12) {
                    todoTextField($newTodo, placeholder: "New todo")
                    todoDatePicker($newTodoDate)
                    Button(action: {
                        if !newTodo.isEmpty {
                            let todo = TodoItem(text: newTodo, date: newTodoDate)
                            todos.append(todo)
                            onTodoAdded(todo)
                            newTodo = ""
                            newTodoDate = Date()
                        }
                    }) {
                        Text("Add")
                            .frame(width: 50)
                    }
                }
                .padding(.horizontal, 12)
                .padding(.vertical, 8)

                List {
                    ForEach(todos) { todo in
                        if editingTodo == todo.id {
                            HStack(spacing: 12) {
                                todoTextField($editedText, placeholder: "Edit todo", maxWidth: 250)
                                todoDatePicker($editedDate)
                                Button(action: {
                                    if let index = todos.firstIndex(where: { $0.id == todo.id }) {
                                        let updatedTodo = TodoItem(id: todo.id, text: editedText, date: editedDate)
                                        todos[index] = updatedTodo
                                        onTodoUpdated(updatedTodo)
                                        editingTodo = nil
                                    }
                                }) {
                                    Text("Save")
                                        .frame(width: 60)
                                }
                            }
                            .padding(.vertical, 4)
                        } else {
                            HStack(spacing: 12) {
                                Text(todo.text)
                                    .lineLimit(1)
                                    .truncationMode(.tail)
                                Spacer()
                                Text(todo.date.formatted(date: .abbreviated, time: .shortened))
                                    .foregroundColor(.gray)
                                Button(action: {
                                    editingTodo = todo.id
                                    editedText = todo.text
                                    editedDate = todo.date
                                }) {
                                    Image(systemName: "pencil")
                                }
                                .buttonStyle(BorderlessButtonStyle())
                                Button(action: {
                                    todos.removeAll(where: { $0.id == todo.id })
                                    onTodoDeleted(todo.id)
                                }) {
                                    Image(systemName: "trash")
                                        .foregroundColor(.red)
                                }
                                .buttonStyle(BorderlessButtonStyle())
                            }
                            .padding(.vertical, 4)
                        }
                    }
                }
            }
        }
    }
}
