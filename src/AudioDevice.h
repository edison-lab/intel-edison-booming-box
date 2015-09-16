#pragma once

#include <assert.h>

#include <string>
#include <iostream>
#include <memory>

#include <RtAudio.h>

#include "Application.h"

#include <math.h>

struct AudioDevice
{
    AudioDevice()
    {}

    ~AudioDevice()
    {
        if (dac)
        {
            if (dac->isStreamOpen())
            {
                dac->stopStream();
                dac->closeStream();
            }

            dac.reset();
        }
    }

    bool open(const std::string& input_device_name,
        const std::string& output_device_name,
        unsigned int num_inputs,
        unsigned int num_outputs,
        unsigned int sample_rate = 44100,
        unsigned int buffer_size = 256,
        unsigned int num_buffres = 1)
    {
        int input_device_id = -1;
        int output_device_id = -1;

        {
            RtAudio temp_dac;
            for (int i = 0; i < temp_dac.getDeviceCount(); i++)
            {
                try {
                    RtAudio::DeviceInfo info = temp_dac.getDeviceInfo(i);
                    std::cout << info.name << " in:" << info.inputChannels << " out:" << info.outputChannels << " duplex: " << info.duplexChannels << std::endl;

                    if (info.name.find(input_device_name) != std::string::npos)
                    {
                        input_device_id = i;
                    }

                    if (info.name.find(output_device_name) != std::string::npos)
                    {
                        output_device_id = i;
                    }
                } catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }

        if (input_device_id == -1
            || output_device_id == -1)
            return false;

        num_inputs_ = num_inputs;
        num_outputs_ = num_outputs;
        sample_rate_ = sample_rate;
        buffer_size_ = buffer_size;

        input_params.deviceId = input_device_id;
        input_params.nChannels = num_inputs;

        output_params.deviceId = output_device_id;
        output_params.nChannels = num_outputs;

        RtAudio::StreamOptions options;
        options.numberOfBuffers = num_buffres;

        RtAudio::StreamParameters* input_params_p = NULL;
        RtAudio::StreamParameters* output_params_p = NULL;

        if (num_inputs) input_params_p = &input_params;
        if (num_outputs) output_params_p = &output_params;

        dac = std::make_shared<RtAudio>();
        dac->openStream(output_params_p,
            input_params_p,
            RTAUDIO_FLOAT32,
            sample_rate_,
            &buffer_size_,
            &AudioDevice::audio_callback,
            this,
            &options,
            on_error);

        return true;
    }

    template <typename T>
    void run()
    {
        assert(dac);
        app_ = std::make_shared<T>(
            num_inputs_,
            num_outputs_,
            sample_rate_,
            buffer_size_);
        dac->startStream();
        app_->run();
    }

    static int audio_callback(
        void* outputBuffer,
        void* inputBuffer,
        unsigned int nFrames,
        double streamTime,
        RtAudioStreamStatus status,
        void* userData)
    {
        if (status == RTAUDIO_INPUT_OVERFLOW)
            std::cout << "RTAUDIO_INPUT_OVERFLOW" << std::endl;
        else if (status == RTAUDIO_OUTPUT_UNDERFLOW)
            std::cout << "RTAUDIO_OUTPUT_UNDERFLOW" << std::endl;

        AudioDevice* self = (AudioDevice*)userData;

        // fill zero
        memset(outputBuffer, 0, sizeof(float) * nFrames * self->getNumOutputs());

        if (self->app_)
        {
            self->app_->audioIO(
                (float*)inputBuffer,
                (float*)outputBuffer,
                nFrames);
        }

        return 0;
    }

    unsigned int getSampleRate() const { return sample_rate_; }
    unsigned int getBufferSize() const { return buffer_size_; }
    unsigned int getNumInputs() const { return num_inputs_; }
    unsigned int getNumOutputs() const { return num_outputs_; }

private:

    std::shared_ptr<RtAudio> dac;

    RtAudio::StreamParameters input_params;
    RtAudio::StreamParameters output_params;

    unsigned int sample_rate_;
    unsigned int buffer_size_;
    unsigned int num_inputs_;
    unsigned int num_outputs_;

    std::shared_ptr<Application> app_;

    static void on_error(RtAudioError::Type type, const std::string &errorText)
    {
        std::cerr << errorText << std::endl;
    }
};
