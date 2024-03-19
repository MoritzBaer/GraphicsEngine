#pragma once

#include <inttypes.h>

namespace Engine
{   
    class Destroyable {
    public:
        virtual void Destroy() const = 0;
    };

    class Initializable : public Destroyable {
    public:
        virtual void Create() = 0;
    };

    struct DeletionQueue
    {
    private:
        Destroyable const ** queue;
        uint32_t queueSize;
        void ChangeBaseMemory();
    public:
        uint32_t queueCapacity;
        void Create();
        void Destroy() const;
        void Push(Initializable * object, bool preInitialized = false);
        void Push(Destroyable const * object);
        void Flush();
    };

    inline DeletionQueue mainDeletionQueue;
    
} // namespace Engine
