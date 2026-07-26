#include "llm-native/model.h"

#include "common.h"      // common_params, common_init_from_params, common_init_result
#include "sampling.h"     // common_sampler_*
#include "chat.h"         // common_chat_*
#include "llama.h"

#include <iostream>
#include <cstring>

struct Model::Impl
{
    common_init_result_ptr    init_result;      // unique_ptr<common_init_result>
    common_chat_templates_ptr chat_templates;
    bool loaded = false;
};

void Model::reset()
{
    if (!impl_) return;
    impl_->chat_templates.reset();
    impl_->init_result.reset();   // destructor frees model + context
    impl_->loaded = false;
}

bool Model::is_loaded() const
{
    return impl_ && impl_->loaded;
}

bool Model::load(const std::string &path, int gpu_layers, int context_size,
                  int threads, float temperature)
{
    if (!impl_) impl_ = new Impl();
    reset();

    common_params params;
    params.model.path          = path;  
    params.n_gpu_layers         = gpu_layers;
    params.n_ctx                = context_size;
    params.cpuparams.n_threads  = threads;
    params.sampling.temp        = temperature;

    impl_->init_result = common_init_from_params(params);
    if (!impl_->init_result || !impl_->init_result->model() || !impl_->init_result->context())
    {
        std::cerr << "Failed to load model: " << path << std::endl;
        return false;
    }

    llama_model* model = impl_->init_result->model();
    impl_->chat_templates = common_chat_templates_init(model, /*chat_template_override=*/"");

    impl_->loaded = true;
    return true;
}

std::string Model::generate(const std::string &prompt, int max_tokens)
{
    if (!impl_ || !impl_->loaded || !impl_->init_result) return {};

    llama_context*     ctx   = impl_->init_result->context();
    llama_model*        model = impl_->init_result->model();
    const llama_vocab*  vocab = llama_model_get_vocab(model);

    llama_memory_clear(llama_get_memory(ctx), false);
    impl_->init_result->reset_samplers();          // clears penalty history from any prior call
    common_sampler* sampler = impl_->init_result->sampler(/*seq_id=*/0);
    if (!sampler) return {};

    std::vector<llama_token> tokens = common_tokenize(vocab, prompt, /*add_special=*/true, /*parse_special=*/true);
    if (tokens.empty()) return {};

    llama_batch batch = llama_batch_get_one(tokens.data(), (int32_t)tokens.size());
    if (llama_decode(ctx, batch) != 0) return {};

    std::string output;
    for (int i = 0; i < max_tokens; ++i)
    {
        llama_token id = common_sampler_sample(sampler, ctx, -1);
        common_sampler_accept(sampler, id, /*accept_grammar=*/true);

        if (llama_vocab_is_eog(vocab, id)) break;

        output += common_token_to_piece(ctx, id);

        llama_token next = id;
        llama_batch next_batch = llama_batch_get_one(&next, 1);
        if (llama_decode(ctx, next_batch) != 0) break;
    }

    return output;
}

Model::ChatResult Model::chat(const std::vector<ChatMessage>& messages, int max_tokens)
{
    ChatResult result;
    if (!impl_ || !impl_->loaded || !impl_->chat_templates) return result;

    std::vector<common_chat_msg> chat_msgs;
    chat_msgs.reserve(messages.size());
    for (auto& m : messages)
    {
        common_chat_msg cm;
        cm.role    = m.role;
        cm.content = m.content;
        chat_msgs.push_back(cm);
    }

    common_chat_templates_inputs inputs;
    inputs.messages              = chat_msgs;
    inputs.add_generation_prompt = true;
    inputs.use_jinja              = true;
    inputs.enable_thinking        = true;   // let reasoning models think; harmless no-op otherwise

    common_chat_params cparams = common_chat_templates_apply(impl_->chat_templates.get(), inputs);

    std::string raw = generate(cparams.prompt, max_tokens);

    // The template itself reports whether/how it delimits a reasoning block,
    // so we split on its own reported tag instead of guessing token text.
    if (cparams.supports_thinking && !cparams.thinking_end_tag.empty())
    {
        size_t end_pos = raw.rfind(cparams.thinking_end_tag);
        if (end_pos != std::string::npos)
        {
            result.reasoning_content = raw.substr(0, end_pos);
            std::string remaining = raw.substr(end_pos + cparams.thinking_end_tag.size());
            size_t trim = remaining.find_first_not_of(" \t\n\r");
            result.content = (trim == std::string::npos) ? std::string{} : remaining.substr(trim);
        }
        else
        {
            // Model didn't close its thinking block within max_tokens —
            // treat the whole thing as reasoning, no final answer yet.
            result.reasoning_content = raw;
        }
    }
    else
    {
        result.content = raw;
    }

    return result;
}

std::vector<Model::BackendInfo> Model::get_backends()
{
    std::vector<BackendInfo> result;
    size_t devCount = ggml_backend_dev_count();
    for (size_t i = 0; i < devCount; ++i)
    {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        if (!dev) continue;
        const char* name = ggml_backend_dev_name(dev);
        const char* desc = ggml_backend_dev_description(dev);
        BackendInfo info;
        info.name        = name ? name : "unknown";
        info.description = desc ? desc : "unknown";
        info.is_cpu      = (name && std::strcmp(name, "CPU") == 0);
        result.push_back(info);
    }
    return result;
}

std::string Model::get_primary_backend()
{
    size_t devCount = ggml_backend_dev_count();
    for (size_t i = 0; i < devCount; ++i)
    {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        if (!dev) continue;
        const char* name = ggml_backend_dev_name(dev);
        if (name && std::strcmp(name, "CPU") != 0) return name;
    }
    return "CPU";
}