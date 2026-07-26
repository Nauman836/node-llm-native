#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

struct llama_model;
struct llama_context;
struct common_sampler;

class Model
{
public:
    Model() = default;
    ~Model();

    bool load(const std::string& path, int gpu_layers = 99, int context_size = 2048,
              int threads = 4, float temperature = 0.7f);

    bool is_loaded() const;

    std::string generate(const std::string& prompt, int max_tokens = 64);

    struct ChatMessage {
        std::string role;
        std::string content;
    };

    // reasoning_content is populated when the model's template supports
    // splitting out a <think>-style block (via common_chat_parse); empty
    // if the template/model doesn't produce one.
    struct ChatResult {
        std::string content;
        std::string reasoning_content;
    };

    ChatResult chat(const std::vector<ChatMessage>& messages, int max_tokens = 512);

    struct BackendInfo {
        std::string name;
        std::string description;
        bool is_cpu = false;
    };

    static std::vector<BackendInfo> get_backends();
    static std::string get_primary_backend();

private:
    void reset();

    struct Impl;
    Impl* impl_ = nullptr;
};