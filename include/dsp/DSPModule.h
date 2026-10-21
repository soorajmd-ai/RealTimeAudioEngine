#pragma once

namespace AudioEngine
{
    class DSPModule
    {
    public:

        virtual ~DSPModule() = default;

        virtual float Process(float sample) = 0;
    };
}
/*
What does this mean?
This class says: "Every DSP effect must provide a Process() function."
It doesn't say how. Each module decides that.
*/