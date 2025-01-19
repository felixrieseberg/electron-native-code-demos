#import <Foundation/Foundation.h>
#include <napi.h>
#import "SwiftBridge.h"

class SwiftAddon : public Napi::Addon<SwiftAddon> {
public:
    SwiftAddon(Napi::Env env, Napi::Object exports) {
        DefineAddon(exports, {
            InstanceMethod("helloWorld", &SwiftAddon::HelloWorld),
            InstanceMethod("helloGui", &SwiftAddon::HelloGui)
        });
    }

private:
    Napi::Value HelloWorld(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Wrong number of arguments")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        if (!info[0].IsString()) {
            Napi::TypeError::New(env, "Wrong argument type")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        std::string input = info[0].As<Napi::String>();
        NSString* nsInput = [NSString stringWithUTF8String:input.c_str()];
        NSString* result = [SwiftBridge helloWorld:nsInput];

        return Napi::String::New(env, [result UTF8String]);
    }

    void HelloGui(const Napi::CallbackInfo& info) {
        [SwiftBridge helloGui];
    }
};

NODE_API_ADDON(SwiftAddon)
