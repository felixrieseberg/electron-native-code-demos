import Foundation
import SwiftUI

@objc
public class SwiftCode: NSObject {
    private static var windowController: NSWindowController?

    @objc
    public static func helloWorld(_ input: String) -> String {
        return "Hello from Swift! You said: \(input)"
    }

    @objc
    public static func helloGui() -> Void {
        let contentView = NSHostingView(rootView: ContentView())
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

    private struct TodoItem: Identifiable {
        let id = UUID()
        var text: String
        var date: Date
    }

    private struct ContentView: View {
        @State private var todos: [TodoItem] = []
        @State private var newTodo: String = ""
        @State private var newTodoDate: Date = Date()
        @State private var editingTodo: UUID?
        @State private var editedText: String = ""
        @State private var editedDate: Date = Date()

        var body: some View {
            VStack(spacing: 16) {
                HStack(spacing: 12) {
                    TextField("New todo", text: $newTodo)
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                        .frame(maxWidth: .infinity)
                    DatePicker("Due date", selection: $newTodoDate, displayedComponents: [.date])
                        .datePickerStyle(CompactDatePickerStyle())
                        .labelsHidden()
                        .frame(width: 100)
                        .textFieldStyle(RoundedBorderTextFieldStyle())
                    Button(action: {
                        if !newTodo.isEmpty {
                            todos.append(TodoItem(text: newTodo, date: newTodoDate))
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
                                TextField("Edit todo", text: $editedText)
                                    .textFieldStyle(RoundedBorderTextFieldStyle())
                                    .frame(maxWidth: 250)
                                DatePicker("Due date", selection: $editedDate, displayedComponents: [.date])
                                    .datePickerStyle(CompactDatePickerStyle())
                                    .textFieldStyle(RoundedBorderTextFieldStyle())
                                    .labelsHidden()
                                    .frame(width: 100)
                                Button(action: {
                                    if let index = todos.firstIndex(where: { $0.id == todo.id }) {
                                        todos[index] = TodoItem(text: editedText, date: editedDate)
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
