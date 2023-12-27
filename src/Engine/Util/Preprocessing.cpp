#include "Preprocessing.h"
#include <stdexcept>

using tokenstream = const char *;

enum class ParserState {
    SCANNING,
    READING_INCLUDE,
    READ_INCLUDE,
    GETTING_FILE_NAME
};

const char* includeKeyword = "include";

Engine::Util::InlinedShader Inline(uint32_t &startLine, const char * fileName) {
    Engine::Util::PreprocessedShader shader(fileName);
    return { shader, startLine, startLine += (shader.GetNumberOfLines() - 1) };
}

Engine::Util::PreprocessedShader::PreprocessedShader(char const *file) : sourceFile(file)
{
    Preprocess(file);
}

char const *Engine::Util::PreprocessedShader::GetPreprocessedCode()
{
    return preprocessedCode.c_str();
}

uint32_t Engine::Util::PreprocessedShader::GetNumberOfLines()
{
    return numberOfLines;
}

const std::string Engine::Util::PreprocessedShader::GetSourceFile()
{
    return sourceFile;
}

const Engine::Util::CompilerMessage Engine::Util::PreprocessedShader::ReconstructMessage(CompilerMessage const &message)
{
    if(message.fileName != sourceFile) { throw std::runtime_error("Message does not belong to this file (file name mismatch)!"); }
    if(message.line > numberOfLines) { throw std::runtime_error("Message does not belong to this file (line number too high)!"); }
    uint32_t currentMainLine = message.line;
    for(auto component : inclusions) {
        if(component.startAtLine > message.line) return { sourceFile, currentMainLine, message.message };
        else if(component.endAtLine < message.line) { currentMainLine -= component.endAtLine - component.startAtLine; }
        else return component.code.ReconstructMessage({ component.code.sourceFile, message.line - component.startAtLine + 1, message.message });
    }
    return { sourceFile, currentMainLine, message.message };
}

void Engine::Util::PreprocessedShader::Preprocess(const char *file)
{
    std::string unprocessedCode = FileIO::ReadFile(file);

    tokenstream ts = unprocessedCode.c_str();
    ParserState state = ParserState::SCANNING;

    std::string fileName;
    numberOfLines = 1;
    
    preprocessedCode = "";
    
    while (*ts)
    {
        switch (state)
        {
        case ParserState::READING_INCLUDE:
            for (int c = 0; c < strlen(includeKeyword); c++) {
                if(includeKeyword[c] != *ts) {    // Other macro
                    preprocessedCode += '#'; 
                    for(int i = 0; i < c; i++) { preprocessedCode += includeKeyword[i]; }
                    state = ParserState::SCANNING; 
                    break; 
                }
                ts++;
            }
            if(state == ParserState::READING_INCLUDE) { state = ParserState::READ_INCLUDE; }
            break;
        case ParserState::READ_INCLUDE:
            if (*ts++ == ' ' && *ts++ == '"') { state = ParserState::GETTING_FILE_NAME; }
            else { ts -= 2; state = ParserState::SCANNING; }
            break;
        case ParserState::GETTING_FILE_NAME:
            fileName = "";
            while (*ts != '"') {
                if(*ts == '\n') { throw std::runtime_error("Line ended while scanning for file to be included (" + GetSourceFile() + ":" + std::to_string(numberOfLines) + ")"); }
                if(!ts) { throw std::runtime_error("File ended while scanning for file to be included (" + GetSourceFile() + ":" + std::to_string(numberOfLines) + ")"); } 
                fileName += *ts++;
            }
            ts++;

            // Preprocess included file
            inclusions.push_back(Inline(numberOfLines, fileName.c_str()));
            preprocessedCode.append(inclusions.front().code.GetPreprocessedCode());
            preprocessedCode += '\n';

            state = ParserState::SCANNING;
            break;
        case ParserState::SCANNING:
            if(*ts == '#') { state = ParserState::READING_INCLUDE; }
            else {
                if(*ts == '\n') { numberOfLines++; }
                preprocessedCode += *ts;
            }
            ts++;
            break;
        default:
            throw std::logic_error("Unexpected parser state");
            break;
        }
    }
    
}