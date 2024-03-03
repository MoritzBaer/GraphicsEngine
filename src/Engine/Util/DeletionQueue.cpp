#include "DeletionQueue.h"

#include <cstdlib>
#include "../Debug/Logging.h"
#include "Macros.h"

const uint32_t INITIAL_QUEUE_CAPACITIES = 16;

void Engine::DeletionQueue::ChangeBaseMemory()
{
    Destroyable** newQueue = static_cast<Destroyable **>(malloc(queueCapacity * sizeof(Destroyable *)));
    memcpy(newQueue, queue, queueSize * sizeof(void *));
    free(queue);
    queue = newQueue;
}

void Engine::DeletionQueue::Create()
{
    queueCapacity = INITIAL_QUEUE_CAPACITIES;
    queueSize = 0;
    queue = static_cast<Destroyable **>(malloc(queueCapacity * sizeof(Destroyable *)));
}

void Engine::DeletionQueue::Destroy()
{
    if(queueSize > 0) { ENGINE_WARNING("Deleting non-empty deletion queue") }
    free(queue);
}

void Engine::DeletionQueue::Push(Initializable *object, bool preInitialized)
{
    if(!preInitialized) { object->Create(); }
    Push(static_cast<Destroyable * >(object));
}

void Engine::DeletionQueue::Push(Destroyable *object)
{
    queue[queueSize++] = object;
    if(queueSize == queueCapacity) { 
        queueCapacity *= 2;
        ChangeBaseMemory();
    }
}

void Engine::DeletionQueue::Flush()
{
    bool queueTooLarge = false;
    if(queueSize < queueCapacity / 4) { queueTooLarge = true; }

    while(queueSize) {
        queue[--queueSize]->Destroy();
    }

    if (queueTooLarge) { 
        queueCapacity /= 2;
        ChangeBaseMemory(); 
    }
}
