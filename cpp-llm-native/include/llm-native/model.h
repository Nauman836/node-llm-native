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

private:
    void reset();
    bool tokenize(const std::string& prompt, std::vector<int>& tokens) const;
    std::string token_to_piece(int token) const;

    struct Impl;           // Forward declaration of implementation struct
    Impl* impl_ = nullptr; // Pimpl idiom to hide llama types
};