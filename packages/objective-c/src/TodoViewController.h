#import <Cocoa/Cocoa.h>

@protocol TodoDelegate
- (void)todoAdded:(NSString *)todoJson;
- (void)todoUpdated:(NSString *)todoJson;
- (void)todoDeleted:(NSString *)todoId;
@end

@interface TodoViewController : NSViewController

@property (nonatomic, weak) id<TodoDelegate> delegate;

- (void)addTodo:(NSString *)text withDate:(NSDate *)date;
- (void)updateTodo:(NSString *)todoId withText:(NSString *)text andDate:(NSDate *)date;
- (void)deleteTodo:(NSString *)todoId;

@end
