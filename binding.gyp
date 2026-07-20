{
  "targets": [
    {
      "target_name": "node_llm_native",

      "sources": [
        "addon/native.cpp",
        "model/model.cpp"
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
        "-lllama"
      ],

      "cflags_cc": [
        "-std=c++17",
        "-fexceptions"
      ],

      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],

      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ]
    }
  ]
}