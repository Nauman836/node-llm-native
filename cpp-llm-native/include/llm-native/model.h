#pragma once

#include <string>
#include <vector>

class Model
{
public:
    Model() = default;
    ~Model();

    bool load(const std::string& path, int gpu_layers = 99, int context_size = 2048, int threads = 4, float temperature = 0.7f);

    std::string generate(const std::string& prompt, int max_tokens = 64);

    struct ChatMessage {
        std::string role;
        std::string content;
    };

    std::string chat(const std::vector<ChatMessage>& messages, int max_tokens = 512);

    // Backend info — no llama types exposed
    struct BackendInfo {
        std::string name;
        std::string description;
        bool is_cpu = false;
    };

    static std::vector<BackendInfo> get_backends();
    static std::string get_primary_backend();

private:
    void reset();
    bool tokenize(const std::string& prompt, std::vector<int>& tokens) const;
    std::string token_to_piece(int token) const;

    struct Impl;
    Impl* impl_ = nullptr;
};