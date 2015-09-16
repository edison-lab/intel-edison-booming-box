#pragma once

#include <string.h>

#include <mutex>

class Application
{
public:

    Application(unsigned int num_inputs,
        unsigned int num_outputs,
        unsigned int sample_rate,
        unsigned int buffer_size)
        : sample_rate_(sample_rate)
        , buffer_size_(buffer_size)
        , num_inputs_(num_inputs)
        , num_outputs_(num_outputs)
    {}

    virtual ~Application() {}

    virtual void audioIO(const float* input, float* output, unsigned int num_frames) = 0;
    virtual void run() = 0;

    unsigned int getSampleRate() const { return sample_rate_; }
    unsigned int getBufferSize() const { return buffer_size_; }
    unsigned int getNumInputs() const { return num_inputs_; }
    unsigned int getNumOutputs() const { return num_outputs_; }

private:
    unsigned int sample_rate_;
    unsigned int buffer_size_;
    unsigned int num_inputs_;
    unsigned int num_outputs_;
};
