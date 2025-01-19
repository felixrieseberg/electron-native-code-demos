#import "SwiftBridge.h"
#import "swift_addon-Swift.h"
#import <Foundation/Foundation.h>

@implementation SwiftBridge

static void (^todoAddedCallback)(NSString*);
static void (^todoUpdatedCallback)(NSString*);
static void (^todoDeletedCallback)(NSString*);

+ (NSString*)helloWorld:(NSString*)input {
    return [SwiftCode helloWorld:input];
}

+ (void)helloGui {
    [SwiftCode helloGui];
}

+ (void)setTodoAddedCallback:(void(^)(NSString*))callback {
    todoAddedCallback = callback;
    [SwiftCode setTodoAddedCallback:callback];
}

+ (void)setTodoUpdatedCallback:(void(^)(NSString*))callback {
    todoUpdatedCallback = callback;
    [SwiftCode setTodoUpdatedCallback:callback];
}

+ (void)setTodoDeletedCallback:(void(^)(NSString*))callback {
    todoDeletedCallback = callback;
    [SwiftCode setTodoDeletedCallback:callback];
}

@end
