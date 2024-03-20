#pragma once

#include <vector>

namespace Engine::Graphics
{
    namespace _Publication {

        template<typename T>
        struct Range {
            T min = 0; 
            T max = 0;
            float step = 1;
        };
    }

    struct Publication
    {

        enum class Type {
            INTEGER1,
            INTEGER2,
            INTEGER3,
            INTEGER4,
            FLOAT1,
            FLOAT2,
            FLOAT3,
            FLOAT4,
            TEXT,
            COLOUR_PICKER,
            TEXTURE_SELECT,
            SHADER_SELECT,
            MESH_SELECT,
            PREFAB_SELECT,
            MATERIAL_SELECT,
            ENUM,
            COMPOSITE
        };

        enum class Style {
            DRAG,
            SLIDER,
            STEPPER,
            RADIO,
            COMBO,
            LIST
        };

        enum Flags {
            RANGE = 1
        };


        const char * label;
        Type type;
        Style style;
        int flags;
        _Publication::Range<float> floatRange;
        _Publication::Range<int> intRange;
        void* referencedPointer;
    };

    class Publishable {
    public: 
        virtual std::vector<Publication> GetPublications() { return { }; }
    };
    
} // namespace Engine::Graphics
