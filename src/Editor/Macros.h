#pragma once

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define EDITOR_MESSAGE(format, ...)                                                                                    \
  { Engine::Debug::Logging::PrintMessage("Editor", format " ({}({},0))", __VA_ARGS__, __FILE_NAME__, __LINE__); }
#define EDITOR_WARNING(format, ...)                                                                                    \
  { Engine::Debug::Logging::PrintWarning("Editor", format " ({}({},0))", __VA_ARGS__, __FILE_NAME__, __LINE__); }
#define EDITOR_SUCCESS(format, ...)                                                                                    \
  { Engine::Debug::Logging::PrintSuccess("Editor", format " ({}({},0))", __VA_ARGS__, __FILE_NAME__, __LINE__); }
#define EDITOR_ERROR(format, ...)                                                                                      \
  {                                                                                                                    \
    Engine::Debug::Logging::PrintError("Editor", format " ({}({},0))", __VA_ARGS__, __FILE_NAME__, __LINE__);          \
    __debugbreak();                                                                                                    \
  }