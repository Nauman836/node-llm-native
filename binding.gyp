{
  "targets": [
    {
      "target_name": "node_llm_native",
      "sources": [
        "src/native.cpp",
        "src/model.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "vendor/llama.cpp/include",
        "vendor/llama.cpp/ggml/include",
        "vendor/llama.cpp/common"
      ],
      "libraries": [
        "-L<(module_root_dir)/vendor/llama.cpp/build/bin",
        "-Wl,-rpath,'$$ORIGIN/../../vendor/llama.cpp/build/bin'",
        "-lllama",
        "-lggml",
        "-lggml-base"
      ],
      "cflags_cc": ["-std=c++17", "-fexceptions"],
      "defines": [],
      "dependencies": []
    }
  ]
}
