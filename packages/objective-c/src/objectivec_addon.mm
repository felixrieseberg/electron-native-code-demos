#import <Foundation/Foundation.h>
#import "ObjCBridge.h"
#include <napi.h>

class ObjectiveCAddon : public Napi::ObjectWrap<ObjectiveCAddon> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "ObjectiveCAddon", {
            InstanceMethod("helloWorld", &ObjectiveCAddon::HelloWorld),
            InstanceMethod("helloGui", &ObjectiveCAddon::HelloGui),
            InstanceMethod("on", &ObjectiveCAddon::On)
        });

        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("ObjectiveCAddon", func);
        return exports;
    }

    struct CallbackData {
        std::string eventType;
        std::string payload;
        ObjectiveCAddon* addon;
    };

    ObjectiveCAddon(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<ObjectiveCAddon>(info)
        , env_(info.Env())
        , emitter(Napi::Persistent(Napi::Object::New(info.Env())))
        , callbacks(Napi::Persistent(Napi::Object::New(info.Env())))
        , tsfn_(nullptr) {

        napi_status status = napi_create_threadsafe_function(
            env_,
            nullptr,
            nullptr,
            Napi::String::New(env_, "ObjectiveCCallback"),
            0,
            1,
            nullptr,
            nullptr,
            this,
            [](napi_env env, napi_value js_callback, void* context, void* data) {
                auto* callbackData = static_cast<CallbackData*>(data);
                if (!callbackData) return;

                Napi::Env napi_env(env);
                Napi::HandleScope scope(napi_env);

                auto addon = static_cast<ObjectiveCAddon*>(context);
                if (!addon) {
                    delete callbackData;
                    return;
                }

                try {
                    auto callback = addon->callbacks.Value().Get(callbackData->eventType).As<Napi::Function>();
                    if (callback.IsFunction()) {
                        callback.Call(addon->emitter.Value(), {Napi::String::New(napi_env, callbackData->payload)});
                    }
                } catch (...) {}

                delete callbackData;
            },
            &tsfn_
        );

        if (status != napi_ok) {
            Napi::Error::New(env_, "Failed to create threadsafe function").ThrowAsJavaScriptException();
            return;
        }

        auto makeCallback = [this](const char* eventType) {
            return ^(NSString* payload) {
                if (tsfn_ != nullptr) {
                    auto* data = new CallbackData{
                        eventType,
                        std::string([payload UTF8String]),
                        this
                    };
                    napi_call_threadsafe_function(tsfn_, data, napi_tsfn_blocking);
                }
            };
        };

        [ObjCBridge setTodoAddedCallback:makeCallback("todoAdded")];
        [ObjCBridge setTodoUpdatedCallback:makeCallback("todoUpdated")];
        [ObjCBridge setTodoDeletedCallback:makeCallback("todoDeleted")];
    }

    ~ObjectiveCAddon() {
        if (tsfn_ != nullptr) {
            napi_release_threadsafe_function(tsfn_, napi_tsfn_release);
            tsfn_ = nullptr;
        }
    }

private:
    Napi::Env env_;
    Napi::ObjectReference emitter;
    Napi::ObjectReference callbacks;
    napi_threadsafe_function tsfn_;

    Napi::Value HelloWorld(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected string argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        std::string input = info[0].As<Napi::String>();
        NSString* nsInput = [NSString stringWithUTF8String:input.c_str()];
        NSString* result = [ObjCBridge helloWorld:nsInput];

        return Napi::String::New(env, [result UTF8String]);
    }

    void HelloGui(const Napi::CallbackInfo& info) {
        [ObjCBridge helloGui];
    }

    Napi::Value On(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
            Napi::TypeError::New(env, "Expected (string, function) arguments").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        callbacks.Value().Set(info[0].As<Napi::String>(), info[1].As<Napi::Function>());
        return env.Undefined();
    }
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return ObjectiveCAddon::Init(env, exports);
}

NODE_API_MODULE(objectivec_addon, Init)
