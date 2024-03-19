#pragma once

namespace Engine::Graphics
{
    class UniformAggregate
    {
    private:
        void * data;
        size_t size;
    public:
        UniformAggregate() : data(nullptr), size(0) { }
        ~UniformAggregate() { delete data; }
        template<typename T>
        inline UniformAggregate & PushData(T * data) { 
            char * newData = reinterpret_cast<char *>(malloc(size + sizeof(T)));
            memcpy(newData, this->data, size);
            memcpy(newData + size, data, sizeof(T));
            this->data = newData;
            size += sizeof(T);

            return *this;
        }

        inline void * Data() { return data; }
        inline size_t Size() { return size; }
    };
    
} // namespace Engine::Graphics
