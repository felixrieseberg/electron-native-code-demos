#import "ObjCBridge.h"
#import "TodoViewController.h"
#import <Cocoa/Cocoa.h>

@interface ObjCBridge () <TodoDelegate>
@end

@implementation ObjCBridge

static ObjCBridge* sharedInstance = nil;
static void (^todoAddedCallback)(NSString*);
static void (^todoUpdatedCallback)(NSString*);
static void (^todoDeletedCallback)(NSString*);
static NSWindowController* windowController;

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[ObjCBridge alloc] init];
    });
    return sharedInstance;
}

+ (NSString*)helloWorld:(NSString*)input {
    return [NSString stringWithFormat:@"Hello from Objective-C! You said: %@", input];
}

+ (void)helloGui {
    TodoViewController* controller = [[TodoViewController alloc] init];
    controller.delegate = [ObjCBridge sharedInstance];

    NSWindow* window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, 500, 500)
        styleMask:NSWindowStyleMaskTitled |
                 NSWindowStyleMaskClosable |
                 NSWindowStyleMaskMiniaturizable |
                 NSWindowStyleMaskResizable
        backing:NSBackingStoreBuffered
        defer:NO];

    window.contentViewController = controller;
    window.title = @"Todo List";
    [window center];

    windowController = [[NSWindowController alloc] initWithWindow:window];
    [windowController showWindow:nil];

    [NSApp activateIgnoringOtherApps:YES];
}

+ (void)setTodoAddedCallback:(void(^)(NSString*))callback {
    todoAddedCallback = callback;
}

+ (void)setTodoUpdatedCallback:(void(^)(NSString*))callback {
    todoUpdatedCallback = callback;
}

+ (void)setTodoDeletedCallback:(void(^)(NSString*))callback {
    todoDeletedCallback = callback;
}

#pragma mark - TodoDelegate

- (void)todoAdded:(NSString *)todoJson {
    if (todoAddedCallback) {
        todoAddedCallback(todoJson);
    }
}

- (void)todoUpdated:(NSString *)todoJson {
    if (todoUpdatedCallback) {
        todoUpdatedCallback(todoJson);
    }
}

- (void)todoDeleted:(NSString *)todoId {
    if (todoDeletedCallback) {
        todoDeletedCallback(todoId);
    }
}

@end
