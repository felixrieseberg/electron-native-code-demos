#import "TodoViewController.h"

@interface TodoItem : NSObject
@property (nonatomic, strong) NSString *id;
@property (nonatomic, strong) NSString *text;
@property (nonatomic, strong) NSDate *date;
@end

@implementation TodoItem
@end

@interface TodoViewController () <NSTableViewDelegate, NSTableViewDataSource>
@property (nonatomic, strong) NSMutableArray<TodoItem *> *todos;
@property (nonatomic, strong) NSTableView *tableView;
@property (nonatomic, strong) NSTextField *todoInput;
@property (nonatomic, strong) NSDatePicker *datePicker;
@end

@implementation TodoViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Set up input controls with proper sizing
    NSView *containerView = self.view;

    // Configure input field
    self.todoInput = [[NSTextField alloc] init];
    self.todoInput.translatesAutoresizingMaskIntoConstraints = NO;
    [containerView addSubview:self.todoInput];

    // Configure date picker
    self.datePicker = [[NSDatePicker alloc] init];
    self.datePicker.datePickerStyle = NSDatePickerStyleTextField;
    self.datePicker.datePickerElements = NSDatePickerElementFlagYearMonth |
                                        NSDatePickerElementFlagYearMonthDay |
                                        NSDatePickerElementFlagHourMinute;
    self.datePicker.translatesAutoresizingMaskIntoConstraints = NO;
    [containerView addSubview:self.datePicker];

    // Configure add button
    NSButton *addButton = [[NSButton alloc] init];
    [addButton setTitle:@"Add"];
    [addButton setBezelStyle:NSBezelStyleRounded];
    [addButton setTarget:self];
    [addButton setAction:@selector(addButtonClicked:)];
    addButton.translatesAutoresizingMaskIntoConstraints = NO;
    [containerView addSubview:addButton];

    [NSLayoutConstraint activateConstraints:@[
        [self.todoInput.leadingAnchor constraintEqualToAnchor:containerView.leadingAnchor constant:20],
        [self.todoInput.topAnchor constraintEqualToAnchor:containerView.topAnchor constant:20],
        [self.todoInput.widthAnchor constraintGreaterThanOrEqualToConstant:200],

        [self.datePicker.leadingAnchor constraintEqualToAnchor:self.todoInput.trailingAnchor constant:8],
        [self.datePicker.centerYAnchor constraintEqualToAnchor:self.todoInput.centerYAnchor],
        [self.datePicker.widthAnchor constraintGreaterThanOrEqualToConstant:150],

        [addButton.leadingAnchor constraintEqualToAnchor:self.datePicker.trailingAnchor constant:8],
        [addButton.trailingAnchor constraintEqualToAnchor:containerView.trailingAnchor constant:-20],
        [addButton.centerYAnchor constraintEqualToAnchor:self.todoInput.centerYAnchor],
        [addButton.widthAnchor constraintEqualToConstant:60]
    ]];

    self.todos = [NSMutableArray array];

    // Create table view
    NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(20, 20, 460, 420)];
    self.tableView = [[NSTableView alloc] initWithFrame:NSZeroRect];

    // Create columns
    NSTableColumn *textColumn = [[NSTableColumn alloc] initWithIdentifier:@"text"];
    textColumn.title = @"Todo";
    textColumn.width = 180;
    [self.tableView addTableColumn:textColumn];

    NSTableColumn *dateColumn = [[NSTableColumn alloc] initWithIdentifier:@"date"];
    dateColumn.title = @"Due Date";
    dateColumn.width = 100;
    [self.tableView addTableColumn:dateColumn];

    // Add action columns
    NSTableColumn *editColumn = [[NSTableColumn alloc] initWithIdentifier:@"edit"];
    editColumn.title = @"Edit";
    editColumn.width = 60;
    [self.tableView addTableColumn:editColumn];

    NSTableColumn *deleteColumn = [[NSTableColumn alloc] initWithIdentifier:@"delete"];
    deleteColumn.title = @"Delete";
    deleteColumn.width = 60;
    [self.tableView addTableColumn:deleteColumn];

    self.tableView.delegate = self;
    self.tableView.dataSource = self;

    scrollView.documentView = self.tableView;
    scrollView.hasVerticalScroller = YES;

    [self.view addSubview:scrollView];
}

// Implementing the required methods from the header
- (void)addTodo:(NSString *)text withDate:(NSDate *)date {
    TodoItem *item = [[TodoItem alloc] init];
    item.id = [[NSUUID UUID] UUIDString];
    item.text = text;
    item.date = date;

    [self.todos addObject:item];
    [self.tableView reloadData];
}

- (void)updateTodo:(NSString *)todoId withText:(NSString *)text andDate:(NSDate *)date {
    for (TodoItem *item in self.todos) {
        if ([item.id isEqualToString:todoId]) {
            item.text = text;
            item.date = date;
            break;
        }
    }
    [self.tableView reloadData];
}

- (void)deleteTodo:(NSString *)todoId {
    TodoItem *itemToDelete = nil;
    for (TodoItem *item in self.todos) {
        if ([item.id isEqualToString:todoId]) {
            itemToDelete = item;
            break;
        }
    }
    if (itemToDelete) {
        [self.todos removeObject:itemToDelete];
        [self.tableView reloadData];
    }
}

// Table view data source methods
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return self.todos.count;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    TodoItem *item = self.todos[row];
    NSString *identifier = tableColumn.identifier;

    if ([identifier isEqualToString:@"text"]) {
        NSTextField *cell = [tableView makeViewWithIdentifier:@"textCell" owner:self];
        if (!cell) {
            cell = [[NSTextField alloc] init];
            cell.identifier = @"textCell";
            cell.editable = NO;
            cell.bordered = NO;
            cell.drawsBackground = NO;
        }
        cell.stringValue = item.text;
        return cell;
    }
    else if ([identifier isEqualToString:@"date"]) {
        NSTextField *cell = [tableView makeViewWithIdentifier:@"dateCell" owner:self];
        if (!cell) {
            cell = [[NSTextField alloc] init];
            cell.identifier = @"dateCell";
            cell.editable = NO;
            cell.bordered = NO;
            cell.drawsBackground = NO;
        }
        NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
        formatter.dateStyle = NSDateFormatterMediumStyle;
        cell.stringValue = [formatter stringFromDate:item.date];
        return cell;
    }
    else if ([identifier isEqualToString:@"edit"]) {
        NSButton *button = [tableView makeViewWithIdentifier:@"editButton" owner:self];
        if (!button) {
            button = [[NSButton alloc] init];
            button.identifier = @"editButton";
            button.bezelStyle = NSBezelStyleRounded;
            NSImage *pencilImage = [NSImage imageWithSystemSymbolName:@"pencil" accessibilityDescription:nil];
            [button setImage:pencilImage];
            [button setImagePosition:NSImageOnly];
            [button setTarget:self];
            [button setAction:@selector(editButtonClicked:)];
        }
        button.tag = row;
        return button;
    }
    else if ([identifier isEqualToString:@"delete"]) {
        NSButton *button = [tableView makeViewWithIdentifier:@"deleteButton" owner:self];
        if (!button) {
            button = [[NSButton alloc] init];
            button.identifier = @"deleteButton";
            button.bezelStyle = NSBezelStyleRounded;
            NSImage *trashImage = [NSImage imageWithSystemSymbolName:@"trash" accessibilityDescription:nil];
            [button setImage:trashImage];
            [button setImagePosition:NSImageOnly];
            [button setTarget:self];
            [button setAction:@selector(deleteButtonClicked:)];
        }
        button.tag = row;
        return button;
    }

    return nil;
}

// Actions
- (void)addButtonClicked:(id)sender {
    TodoItem *item = [[TodoItem alloc] init];
    item.id = [[NSUUID UUID] UUIDString];
    item.text = self.todoInput.stringValue;
    item.date = self.datePicker.dateValue;

    [self.todos addObject:item];
    [self.tableView reloadData];

    // Notify delegate
    if (self.delegate) {
        NSDictionary *dict = @{
            @"id": item.id,
            @"text": item.text,
            @"date": @([item.date timeIntervalSince1970] * 1000) // Convert to milliseconds for JavaScript
        };
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:0 error:nil];
        NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        [self.delegate todoAdded:jsonString];
    }

    self.todoInput.stringValue = @"";
}

- (void)editButtonClicked:(NSButton *)sender {
    NSInteger row = sender.tag;
    TodoItem *item = self.todos[row];

    // Create edit dialog
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Edit Todo";
    alert.alertStyle = NSAlertStyleInformational;

    // Create stack view for input fields
    NSStackView *stackView = [[NSStackView alloc] init];
    stackView.orientation = NSUserInterfaceLayoutOrientationVertical;
    stackView.spacing = 8;
    stackView.frame = NSMakeRect(0, 0, 300, 80);

    // Add text field
    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    input.stringValue = item.text;
    input.placeholderString = @"Todo text";
    [input setEditable:YES];
    [input setSelectable:YES];
    [input setBezeled:YES];
    [input setBezelStyle:NSTextFieldSquareBezel];

    // Add date picker
    NSDatePicker *datePicker = [[NSDatePicker alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    datePicker.dateValue = item.date;
    datePicker.datePickerStyle = NSDatePickerStyleTextField;
    datePicker.datePickerElements = NSDatePickerElementFlagYearMonth |
                                   NSDatePickerElementFlagYearMonthDay |
                                   NSDatePickerElementFlagHourMinute;
    [datePicker setEnabled:YES];

    // Add labels and controls to stack view
    NSTextField *textLabel = [[NSTextField alloc] init];
    [textLabel setStringValue:@"Todo Text:"];
    [textLabel setBezeled:NO];
    [textLabel setDrawsBackground:NO];
    [textLabel setEditable:NO];
    [textLabel setSelectable:NO];

    NSTextField *dateLabel = [[NSTextField alloc] init];
    [dateLabel setStringValue:@"Due Date:"];
    [dateLabel setBezeled:NO];
    [dateLabel setDrawsBackground:NO];
    [dateLabel setEditable:NO];
    [dateLabel setSelectable:NO];

    [stackView addArrangedSubview:textLabel];
    [stackView addArrangedSubview:input];
    [stackView addArrangedSubview:dateLabel];
    [stackView addArrangedSubview:datePicker];

    alert.accessoryView = stackView;

    [alert addButtonWithTitle:@"Save"];
    [alert addButtonWithTitle:@"Cancel"];

    // Make input field first responder
    [alert.window makeFirstResponder:input];

    [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse response) {
        if (response == NSAlertFirstButtonReturn) {
            // Validate input
            if (input.stringValue.length == 0) {
                NSAlert *errorAlert = [[NSAlert alloc] init];
                errorAlert.messageText = @"Invalid Input";
                errorAlert.informativeText = @"Todo text cannot be empty";
                errorAlert.alertStyle = NSAlertStyleWarning;
                [errorAlert beginSheetModalForWindow:self.view.window completionHandler:nil];
                return;
            }

            item.text = input.stringValue;
            item.date = datePicker.dateValue;
            [self.tableView reloadData];

            // Notify delegate
            if (self.delegate) {
                NSString *json = [self jsonStringFromTodo:item];
                [self.delegate todoUpdated:json];
            }
        }
    }];
}

- (void)deleteButtonClicked:(NSButton *)sender {
    NSInteger row = sender.tag;
    TodoItem *item = self.todos[row];

    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Delete Todo";
    alert.informativeText = @"Are you sure you want to delete this todo?";
    alert.alertStyle = NSAlertStyleWarning;

    [alert addButtonWithTitle:@"Delete"];
    [alert addButtonWithTitle:@"Cancel"];

    [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse response) {
        if (response == NSAlertFirstButtonReturn) {
            // Notify delegate before removing the item
            if (self.delegate) {
                [self.delegate todoDeleted:item.id];
            }

            [self.todos removeObjectAtIndex:row];
            [self.tableView reloadData];
        }
    }];
}

// Helper method to convert TodoItem to JSON string
- (NSString *)jsonStringFromTodo:(TodoItem *)todo {
    NSDictionary *dict = @{
        @"id": todo.id,
        @"text": todo.text,
        @"date": @([todo.date timeIntervalSince1970])
    };

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:0 error:&error];
    if (error) {
        NSLog(@"Error converting todo to JSON: %@", error);
        return @"{}";
    }

    return [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
}

@end
