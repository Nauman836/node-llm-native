#include "model.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    bool g_backend_initialized = false;

    void silence_log(ggml_log_level /*level*/, const char* /*text*/, void* /*user_data*/)
    {
    }

    std::string resolve_model_path(const std::string& path)
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
}

Model::~Model()
{
    reset();
}

void Model::reset()
{
    if (sampler_ != nullptr)
    {
        llama_sampler_free(sampler_);
        sampler_ = nullptr;
    }

    if (context_ != nullptr)
    {
        llama_free(context_);
        context_ = nullptr;
    }

    if (model_ != nullptr)
    {
        llama_model_free(model_);
        model_ = nullptr;
    }

    vocab_ = nullptr;
    loaded_ = false;
}

bool Model::load(const std::string& path, int gpu_layers, int context_size, int threads, float temperature)
{
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

    model_ = llama_model_load_from_file(resolved_path.c_str(), model_params);
    if (!model_)
    {
        std::cerr << "Failed to load model: " << resolved_path << std::endl;
        return false;
    }

    vocab_ = llama_model_get_vocab(model_);

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = context_size;
    ctx_params.n_batch = std::min(512, context_size);

    context_ = llama_init_from_model(model_, ctx_params);
    if (!context_)
    {
        std::cerr << "Failed to create llama context" << std::endl;
        reset();
        return false;
    }

    llama_set_n_threads(context_, threads, threads);

    auto sparams = llama_sampler_chain_default_params();
    sampler_ = llama_sampler_chain_init(sparams);
    if (!sampler_)
    {
        std::cerr << "Failed to create sampler" << std::endl;
        reset();
        return false;
    }

    // Use greedy decoding for the initial build so generation stays stable and
    // avoids sampler edge cases in this lightweight wrapper.
    llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
    loaded_ = true;
    return true;
}

bool Model::tokenize(const std::string& prompt, std::vector<llama_token>& tokens) const
{
    if (!loaded_ || vocab_ == nullptr)
    {
        return false;
    }

    const int n_tokens = -llama_tokenize(vocab_, prompt.c_str(), static_cast<int>(prompt.size()), nullptr, 0, true, true);
    if (n_tokens <= 0)
    {
        return false;
    }

    tokens.resize(n_tokens);
    const int actual = llama_tokenize(vocab_, prompt.c_str(), static_cast<int>(prompt.size()), tokens.data(), static_cast<int>(tokens.size()), true, true);
    return actual > 0;
}

std::string Model::token_to_piece(llama_token token) const
{
    if (!loaded_ || vocab_ == nullptr)
    {
        return {};
    }

    char buffer[128];
    const int n = llama_token_to_piece(vocab_, token, buffer, sizeof(buffer), 0, true);
    if (n < 0)
    {
        return {};
    }

    return std::string(buffer, n);
}

std::string Model::generate(const std::string& prompt, int max_tokens)
{
    if (!loaded_ || context_ == nullptr || model_ == nullptr || sampler_ == nullptr)
    {
        return {};
    }

    std::vector<llama_token> prompt_tokens;
    if (!tokenize(prompt, prompt_tokens))
    {
        return {};
    }

    if (prompt_tokens.empty())
    {
        return {};
    }

    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), static_cast<int>(prompt_tokens.size()));

    if (llama_model_has_encoder(model_))
    {
        if (llama_encode(context_, batch) != 0)
        {
            std::cerr << "Failed to encode prompt" << std::endl;
            return {};
        }

        llama_token decoder_start_token_id = llama_model_decoder_start_token(model_);
        if (decoder_start_token_id == LLAMA_TOKEN_NULL)
        {
            decoder_start_token_id = llama_vocab_bos(vocab_);
        }

        batch = llama_batch_get_one(&decoder_start_token_id, 1);
    }

    std::string output;
    for (int i = 0; i < std::max(1, max_tokens); ++i)
    {
        if (llama_decode(context_, batch) != 0)
        {
            break;
        }

        llama_token new_token_id = llama_sampler_sample(sampler_, context_, -1);
        if (llama_vocab_is_eog(vocab_, new_token_id))
        {
            break;
        }

        output += token_to_piece(new_token_id);
        batch = llama_batch_get_one(&new_token_id, 1);
    }

    return output;
}

#if defined(LLAMA_STANDALONE)
int main(int argc, char** argv)
{
    Model model;

    std::string model_path = "MiniCPM5-1B-Claude-Opus-Fable5-V2-Thinking-Q8_0.gguf";
    std::string prompt = "Hello, how are you?";
    int max_tokens = 32;

    if (argc > 1)
    {
        model_path = argv[1];
    }
    if (argc > 2)
    {
        prompt = argv[2];
    }
    if (argc > 3)
    {
        max_tokens = std::stoi(argv[3]);
    }

    if (!model.load(model_path))
    {
        std::cerr << "Could not load model" << std::endl;
        return 1;
    }

    const std::string response = model.generate(prompt, max_tokens);

    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Response: " << response << std::endl;
    return 0;
}
#endif
