#include "llm-native/model.h"

#include "llama.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    bool g_backend_initialized = false;

    void silence_log(ggml_log_level /*level*/, const char * /*text*/, void * /*user_data*/)
    {
    }

    std::string resolve_model_path(const std::string &path)
    {
        std::filesystem::path candidate(path);
        if (candidate.is_absolute() || std::filesystem::exists(candidate))
        {
            return candidate.string();
        }

        const std::filesystem::path from_cwd = std::filesystem::current_path() / candidate;
        if (std::filesystem::exists(from_cwd))
        {
            return from_cwd.string();
        }

        const std::filesystem::path from_source = std::filesystem::path(__FILE__).parent_path().parent_path() / candidate;
        if (std::filesystem::exists(from_source))
        {
            return from_source.string();
        }

        return path;
    }

    std::string build_generic_chat_prompt(const std::vector<Model::ChatMessage>& messages)
    {
        std::ostringstream prompt;

        for (const auto& msg : messages)
        {
            if (msg.role == "system")
            {
                prompt << msg.content << "\n\n";
            }
            else if (msg.role == "user")
            {
                prompt << "User: " << msg.content << "\n";
            }
            else if (msg.role == "assistant")
            {
                prompt << "Assistant: " << msg.content << "\n";
            }
        }

        prompt << "Assistant:";
        return prompt.str();
    }

    std::vector<llama_chat_message> convert_messages(const std::vector<Model::ChatMessage>& messages)
    {
        std::vector<llama_chat_message> result;
        result.reserve(messages.size());

        for (const auto& msg : messages)
        {
            llama_chat_message lm;
            lm.role = msg.role.c_str();
            lm.content = msg.content.c_str();
            result.push_back(lm);
        }

        return result;
    }

    std::string get_model_chat_template(const llama_model* model)
    {
        const char* key = "tokenizer.chat_template";
        int32_t size_needed = llama_model_meta_val_str(model, key, nullptr, 0);
        if (size_needed <= 0)
        {
            return {};
        }

        std::vector<char> buffer(size_needed + 1, '\0');
        int32_t result = llama_model_meta_val_str(model, key, buffer.data(), static_cast<int32_t>(buffer.size()));
        if (result <= 0)
        {
            return {};
        }

        return std::string(buffer.data());
    }

    bool has_native_chat_template(const llama_model* model)
    {
        return !get_model_chat_template(model).empty();
    }

    std::string apply_native_template(const llama_model* model, const std::vector<Model::ChatMessage>& messages, bool add_assistant_prefix)
    {
        std::string tmpl = get_model_chat_template(model);
        if (tmpl.empty())
        {
            return {};
        }

        auto llama_messages = convert_messages(messages);

        int32_t required_size = llama_chat_apply_template(
            tmpl.c_str(),
            llama_messages.data(),
            llama_messages.size(),
            add_assistant_prefix,
            nullptr,
            0
        );

        if (required_size < 0)
        {
            return {};
        }

        std::vector<char> buffer(required_size + 1, '\0');
        int32_t result = llama_chat_apply_template(
            tmpl.c_str(),
            llama_messages.data(),
            llama_messages.size(),
            add_assistant_prefix,
            buffer.data(),
            static_cast<int32_t>(buffer.size())
        );

        if (result < 0)
        {
            return {};
        }

        return std::string(buffer.data());
    }
}

struct Model::Impl
{
    llama_model* model = nullptr;
    llama_context* context = nullptr;
    llama_sampler* sampler = nullptr;
    const llama_vocab* vocab = nullptr;
    bool loaded = false;
};

Model::~Model()
{
    reset();
    delete impl_;
    impl_ = nullptr;
}

void Model::reset()
{
    if (!impl_)
        return;

    if (impl_->sampler != nullptr)
    {
        llama_sampler_free(impl_->sampler);
        impl_->sampler = nullptr;
    }

    if (impl_->context != nullptr)
    {
        llama_free(impl_->context);
        impl_->context = nullptr;
    }

    if (impl_->model != nullptr)
    {
        llama_model_free(impl_->model);
        impl_->model = nullptr;
    }

    impl_->vocab = nullptr;
    impl_->loaded = false;
}

bool Model::load(const std::string &path, int gpu_layers, int context_size, int threads, float /*temperature*/)
{
    if (!impl_)
        impl_ = new Impl();

    reset();

    const std::string resolved_path = resolve_model_path(path);

    llama_log_set(silence_log, nullptr);

    if (!g_backend_initialized)
    {
        llama_backend_init();
        g_backend_initialized = true;
    }

    ggml_backend_load_all();

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = gpu_layers;

    impl_->model = llama_model_load_from_file(resolved_path.c_str(), model_params);
    if (!impl_->model)
    {
        std::cerr << "Failed to load model: " << resolved_path << std::endl;
        return false;
    }

    impl_->vocab = llama_model_get_vocab(impl_->model);

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = context_size;
    ctx_params.n_batch = std::min(512, context_size);

    impl_->context = llama_init_from_model(impl_->model, ctx_params);
    if (!impl_->context)
    {
        std::cerr << "Failed to create llama context" << std::endl;
        reset();
        return false;
    }

    llama_set_n_threads(impl_->context, threads, threads);

    auto sparams = llama_sampler_chain_default_params();
    impl_->sampler = llama_sampler_chain_init(sparams);
    if (!impl_->sampler)
    {
        std::cerr << "Failed to create sampler" << std::endl;
        reset();
        return false;
    }

    llama_sampler_chain_add(impl_->sampler, llama_sampler_init_greedy());
    impl_->loaded = true;
    return true;
}

bool Model::tokenize(const std::string &prompt, std::vector<int> &tokens) const
{
    if (!impl_ || !impl_->loaded || impl_->vocab == nullptr)
    {
        return false;
    }

    const int n_tokens = -llama_tokenize(impl_->vocab, prompt.c_str(), static_cast<int>(prompt.size()), nullptr, 0, true, true);
    if (n_tokens <= 0)
    {
        return false;
    }

    tokens.resize(n_tokens);
    const int actual = llama_tokenize(impl_->vocab, prompt.c_str(), static_cast<int>(prompt.size()),
                                      reinterpret_cast<llama_token*>(tokens.data()), static_cast<int>(tokens.size()), true, true);
    return actual > 0;
}

std::string Model::token_to_piece(int token) const
{
    if (!impl_ || !impl_->loaded || impl_->vocab == nullptr)
    {
        return {};
    }

    char buffer[128];
    const int n = llama_token_to_piece(impl_->vocab, static_cast<llama_token>(token), buffer, sizeof(buffer), 0, true);
    if (n < 0)
    {
        return {};
    }

    return std::string(buffer, n);
}

std::string Model::generate(const std::string &prompt, int max_tokens)
{
    if (!impl_ || !impl_->loaded || impl_->context == nullptr || impl_->model == nullptr || impl_->sampler == nullptr)
    {
        return {};
    }

    std::vector<int> prompt_tokens;
    if (!tokenize(prompt, prompt_tokens))
    {
        return {};
    }

    if (prompt_tokens.empty())
    {
        return {};
    }

    llama_batch batch = llama_batch_get_one(reinterpret_cast<llama_token*>(prompt_tokens.data()), static_cast<int>(prompt_tokens.size()));

    if (llama_model_has_encoder(impl_->model))
    {
        if (llama_encode(impl_->context, batch) != 0)
        {
            std::cerr << "Failed to encode prompt" << std::endl;
            return {};
        }

        llama_token decoder_start_token_id = llama_model_decoder_start_token(impl_->model);
        if (decoder_start_token_id == LLAMA_TOKEN_NULL)
        {
            decoder_start_token_id = llama_vocab_bos(impl_->vocab);
        }

        batch = llama_batch_get_one(&decoder_start_token_id, 1);
    }

    std::string output;
    for (int i = 0; i < std::max(1, max_tokens); ++i)
    {
        if (llama_decode(impl_->context, batch) != 0)
        {
            break;
        }

        llama_token new_token_id = llama_sampler_sample(impl_->sampler, impl_->context, -1);
        if (llama_vocab_is_eog(impl_->vocab, new_token_id))
        {
            break;
        }

        output += token_to_piece(static_cast<int>(new_token_id));
        batch = llama_batch_get_one(&new_token_id, 1);
    }

    return output;
}

std::string Model::chat(const std::vector<ChatMessage>& messages, int max_tokens)
{
    if (!impl_ || !impl_->loaded || impl_->model == nullptr)
    {
        return {};
    }

    std::string chat_prompt;
    bool using_native_template = false;

    if (has_native_chat_template(impl_->model))
    {
        chat_prompt = apply_native_template(impl_->model, messages, true);
        if (!chat_prompt.empty())
        {
            using_native_template = true;
        }
    }

    if (chat_prompt.empty())
    {
        chat_prompt = build_generic_chat_prompt(messages);
    }

    std::string response = generate(chat_prompt, max_tokens);

    size_t pos = 0;
    while (pos < response.size() && std::isspace(static_cast<unsigned char>(response[pos])))
    {
        ++pos;
    }
    if (pos > 0)
    {
        response = response.substr(pos);
    }

    if (!using_native_template)
    {
        const std::string stop_markers[] = {"User:", "Assistant:", "Human:", "AI:"};
        for (const auto& marker : stop_markers)
        {
            size_t stop_pos = response.find(marker);
            if (stop_pos != std::string::npos)
            {
                response = response.substr(0, stop_pos);
                break;
            }
        }
    }
    else
    {
        const std::string assistant_prefixes[] = {"Assistant", "assistant", "<|assistant|>", "<|im_start|>assistant"};
        for (const auto& prefix : assistant_prefixes)
        {
            size_t stop_pos = response.find(prefix);
            if (stop_pos != std::string::npos)
            {
                response = response.substr(0, stop_pos);
                break;
            }
        }
    }

    while (!response.empty() && std::isspace(static_cast<unsigned char>(response.back())))
    {
        response.pop_back();
    }

    return response;
}

// Static: get all available backends
std::vector<Model::BackendInfo> Model::get_backends()
{
    std::vector<BackendInfo> result;

    size_t devCount = ggml_backend_dev_count();
    for (size_t i = 0; i < devCount; ++i)
    {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        if (!dev)
            continue;

        const char* name = ggml_backend_dev_name(dev);
        const char* desc = ggml_backend_dev_description(dev);

        BackendInfo info;
        info.name = name ? name : "unknown";
        info.description = desc ? desc : "unknown";
        info.is_cpu = (name && std::strcmp(name, "CPU") == 0);
        result.push_back(info);
    }

    return result;
}

// Static: get primary (preferred) backend name
std::string Model::get_primary_backend()
{
    size_t devCount = ggml_backend_dev_count();
    for (size_t i = 0; i < devCount; ++i)
    {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        if (!dev)
            continue;

        const char* name = ggml_backend_dev_name(dev);
        if (!name)
            continue;

        if (std::strcmp(name, "CPU") != 0)
        {
            return name;
        }
    }

    return "CPU";
}