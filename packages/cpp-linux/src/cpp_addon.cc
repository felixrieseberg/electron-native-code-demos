#include <napi.h>
#include <string>
#include "cpp_code.h"

class CppAddon : public Napi::ObjectWrap<CppAddon> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "CppLinuxAddon", {
            InstanceMethod("helloWorld", &CppAddon::HelloWorld),
            InstanceMethod("helloGui", &CppAddon::HelloGui),
            InstanceMethod("on", &CppAddon::On)
        });

        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("CppLinuxAddon", func);
        return exports;
    }

    struct CallbackData {
        std::string eventType;
        std::string payload;
        CppAddon* addon;
    };

    CppAddon(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<CppAddon>(info)
        , env_(info.Env())
        , emitter(Napi::Persistent(Napi::Object::New(info.Env())))
        , callbacks(Napi::Persistent(Napi::Object::New(info.Env())))
        , tsfn_(nullptr) {

        napi_status status = napi_create_threadsafe_function(
            env_,
            nullptr,
            nullptr,
            Napi::String::New(env_, "CppCallback"),
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

                auto addon = static_cast<CppAddon*>(context);
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

        // Set up the callbacks here
        auto makeCallback = [this](const std::string& eventType) {
            return [this, eventType](const std::string& payload) {
                if (tsfn_ != nullptr) {
                    auto* data = new CallbackData{
                        eventType,
                        payload,
                        this
                    };
                    napi_call_threadsafe_function(tsfn_, data, napi_tsfn_blocking);
                }
            };
        };

        cpp_code::setTodoAddedCallback(makeCallback("todoAdded"));
        cpp_code::setTodoUpdatedCallback(makeCallback("todoUpdated"));
        cpp_code::setTodoDeletedCallback(makeCallback("todoDeleted"));
    }

    ~CppAddon() {
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
        std::string result = cpp_code::hello_world(input);

        return Napi::String::New(env, result);
    }

    void HelloGui(const Napi::CallbackInfo& info) {
        cpp_code::hello_gui();
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
    return CppAddon::Init(env, exports);
}

NODE_API_MODULE(cpp_addon, Init)
