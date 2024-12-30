#include "Shader.h"
#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "Util/FileIO.h"
#include <algorithm>

namespace Engine::Graphics {

class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
  struct UserData {
    const char *name;
    const char *content;

    UserData(const char *name, const char *content)
        : name(strcpy(static_cast<char *>(malloc(strlen(name) * sizeof(char))), name)),
          content(strcpy(static_cast<char *>(malloc(strlen(content) * sizeof(char))), content)) {}
    ~UserData() {
      // free((void *)name);
      // free((void *)content);
    }
  };

public:
  shaderc_include_result *GetInclude(const char *requested_source, shaderc_include_type type,
                                     const char *requesting_source, size_t include_depth) override {
    auto shaderCode = Util::FileIO::ReadFile("res/shaders/" + std::string(requested_source));
    shaderCode.push_back(0);

    UserData *userData = new UserData(requested_source, shaderCode.data());
    return new shaderc_include_result{.source_name = userData->name,
                                      .source_name_length = strlen(userData->name),
                                      .content = userData->content,
                                      .content_length = strlen(userData->content),
                                      .user_data = userData};
  }

  void ReleaseInclude(shaderc_include_result *data) override {
    delete static_cast<UserData *>(data->user_data);
    delete data;
  }
};

void ShaderCompiler::AssertPreprocessingWorked(shaderc_compilation_status status, const char *shaderName,
                                               const char *message) {
  ENGINE_ASSERT(status == shaderc_compilation_status::shaderc_compilation_status_success, "-- Preprocessing {} --\n{}",
                shaderName, message);
}

void ShaderCompiler::AssertCompilationWorked(shaderc_compilation_status status, const char *shaderName,
                                             const char *message) {
  ENGINE_ASSERT(status == shaderc_compilation_status::shaderc_compilation_status_success, "-- Compiling {} --\n{}",
                shaderName, message);
}

ShaderCompiler::ShaderCompiler(InstanceManager &instanceManager) : instanceManager(instanceManager) {
  options.SetIncluder(std::make_unique<ShaderIncluder>());
}

} // namespace Engine::Graphics