#include <napi.h>

#include "llm-native/model.h"

#include <memory>
#include <string>
#include <vector>

namespace
{
    class ModelWrapper : public Napi::ObjectWrap<ModelWrapper>
    {
    public:
        static Napi::Function Init(Napi::Env env)
        {
            Napi::Function func = DefineClass(env, "Model", {
                                                                InstanceMethod("load", &ModelWrapper::Load),
                                                                InstanceMethod("generate", &ModelWrapper::Generate),
                                                                InstanceMethod("chat", &ModelWrapper::Chat),
                                                                StaticMethod("getBackends", &ModelWrapper::GetBackends),
                                                                StaticMethod("getPrimaryBackend", &ModelWrapper::GetPrimaryBackend),
                                                            });
            constructor = Napi::Persistent(func);
            constructor.SuppressDestruct();
            return func;
        }

        static Napi::Object NewInstance(Napi::Env env, const Napi::Value &arg)
        {
            return constructor.Value().New({arg});
        }

        ModelWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ModelWrapper>(info)
        {
            if (info.Length() > 0 && info[0].IsObject())
            {
                const auto options = info[0].As<Napi::Object>();
                std::string model_path = options.Get("model").ToString();
                int gpu_layers = options.Has("gpuLayers") ? options.Get("gpuLayers").ToNumber().Int32Value() : -1;
                int context_size = options.Has("contextSize") ? options.Get("contextSize").ToNumber().Int32Value() : 2048;
                int threads = options.Has("threads") ? options.Get("threads").ToNumber().Int32Value() : 4;
                float temperature = options.Has("temperature") ? static_cast<float>(options.Get("temperature").ToNumber().FloatValue()) : 0.7f;

                model_.load(model_path, gpu_layers, context_size, threads, temperature);
            }
        }

    private:
        Napi::Value Load(const Napi::CallbackInfo &info)
        {
            if (info.Length() < 1 || !info[0].IsString())
            {
                throw Napi::TypeError::New(info.Env(), "Expected model path string");
            }

            const std::string path = info[0].As<Napi::String>();
            return Napi::Boolean::New(info.Env(), model_.load(path));
        }

        Napi::Value Generate(const Napi::CallbackInfo &info)
        {
            if (info.Length() < 1 || !info[0].IsString())
            {
                throw Napi::TypeError::New(info.Env(), "Expected prompt string");
            }

            std::string prompt = info[0].As<Napi::String>();
            int max_tokens = 64;
            if (info.Length() >= 2 && info[1].IsNumber())
            {
                max_tokens = info[1].As<Napi::Number>().Int32Value();
            }
            return Napi::String::New(info.Env(), model_.generate(prompt, max_tokens));
        }

        Napi::Value Chat(const Napi::CallbackInfo &info)
        {
            Napi::Env env = info.Env();

            if (info.Length() < 1 || !info[0].IsArray())
            {
                throw Napi::TypeError::New(env, "Expected messages array as first argument");
            }

            Napi::Array messagesArray = info[0].As<Napi::Array>();
            std::vector<Model::ChatMessage> messages;

            uint32_t length = messagesArray.Length();
            for (uint32_t i = 0; i < length; i++)
            {
                Napi::Value item = messagesArray[i];
                if (!item.IsObject())
                {
                    throw Napi::TypeError::New(env, "Each message must be an object");
                }

                Napi::Object msgObj = item.As<Napi::Object>();
                Model::ChatMessage msg;

                if (!msgObj.Has("role") || !msgObj.Get("role").IsString())
                {
                    throw Napi::TypeError::New(env, "Each message must have a 'role' string");
                }
                msg.role = msgObj.Get("role").As<Napi::String>();

                if (!msgObj.Has("content") || !msgObj.Get("content").IsString())
                {
                    throw Napi::TypeError::New(env, "Each message must have a 'content' string");
                }
                msg.content = msgObj.Get("content").As<Napi::String>();

                messages.push_back(msg);
            }

            int max_tokens = 512;
            if (info.Length() >= 2 && info[1].IsNumber())
            {
                max_tokens = info[1].As<Napi::Number>().Int32Value();
            }

            std::string response = model_.chat(messages, max_tokens);
            return Napi::String::New(env, response);
        }

        static Napi::Value GetBackends(const Napi::CallbackInfo &info)
        {
            Napi::Env env = info.Env();
            auto backends = Model::get_backends();

            Napi::Array arr = Napi::Array::New(env);
            for (size_t i = 0; i < backends.size(); ++i)
            {
                Napi::Object obj = Napi::Object::New(env);
                obj.Set("name", backends[i].name);
                obj.Set("description", backends[i].description);
                obj.Set("isCpu", backends[i].is_cpu);
                arr.Set(static_cast<uint32_t>(i), obj);
            }

            return arr;
        }

        static Napi::Value GetPrimaryBackend(const Napi::CallbackInfo &info)
        {
            return Napi::String::New(info.Env(), Model::get_primary_backend());
        }

        Model model_;
        static Napi::FunctionReference constructor;
    };

    Napi::FunctionReference ModelWrapper::constructor;
}

Napi::Value BuildInfo(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("backend", Model::get_primary_backend());

#ifdef _WIN32
    obj.Set("platform", "win32");
#elif __APPLE__
    obj.Set("platform", "darwin");
#else
    obj.Set("platform", "linux");
#endif

#if defined(__x86_64__) || defined(_M_X64)
    obj.Set("arch", "x64");
#elif defined(__aarch64__) || defined(_M_ARM64)
    obj.Set("arch", "arm64");
#else
    obj.Set("arch", "unknown");
#endif

    // Also include full backends list
    auto backends = Model::get_backends();
    Napi::Array arr = Napi::Array::New(env);
    for (size_t i = 0; i < backends.size(); ++i)
    {
        Napi::Object b = Napi::Object::New(env);
        b.Set("name", backends[i].name);
        b.Set("description", backends[i].description);
        b.Set("isCpu", backends[i].is_cpu);
        arr.Set(static_cast<uint32_t>(i), b);
    }
    obj.Set("backends", arr);

    return obj;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("createModel", Napi::Function::New(env, [](const Napi::CallbackInfo &info)
                                                   { return ModelWrapper::NewInstance(info.Env(), info[0]); }));

    exports.Set("Model", ModelWrapper::Init(env));

    exports.Set("buildInfo", Napi::Function::New(env, BuildInfo));

    return exports;
}

NODE_API_MODULE(node_llm_native, Init)