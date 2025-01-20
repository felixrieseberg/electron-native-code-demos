#ifndef ObjCBridge_h
#define ObjCBridge_h

#import <Foundation/Foundation.h>

@interface ObjCBridge : NSObject

+ (NSString*)helloWorld:(NSString*)input;
+ (void)helloGui;

+ (void)setTodoAddedCallback:(void(^)(NSString* todoJson))callback;
+ (void)setTodoUpdatedCallback:(void(^)(NSString* todoJson))callback;
+ (void)setTodoDeletedCallback:(void(^)(NSString* todoId))callback;

@end

#endif
