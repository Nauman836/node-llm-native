#pragma once

#include "llama.h"

#include <string>
#include <vector>

class Model
{
public:
    Model() = default;
    ~Model();

    bool load(const std::string& path, int gpu_layers = 99, int context_size = 2048, int threads = 4, float temperature = 0.7f);
    std::string generate(const std::string& prompt, int max_tokens = 64);

private:
    void reset();
    bool tokenize(const std::string& prompt, std::vector<llama_token>& tokens) const;
    std::string token_to_piece(llama_token token) const;

    llama_model* model_ = nullptr;
    llama_context* context_ = nullptr;
    llama_sampler* sampler_ = nullptr;
    const llama_vocab* vocab_ = nullptr;
    bool loaded_ = false;
};