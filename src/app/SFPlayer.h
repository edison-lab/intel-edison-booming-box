#pragma once

#include <dirent.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <thread>

#include <sndfile.h>
#include <mraa.h>

#include "SystemTime.h"
#include "Application.h"

class SoundFilePlayer
    : public Application
{
public:

    unsigned long long last_emit;
    unsigned int emit_speed;

    float global_volume;

    SoundFilePlayer(unsigned int num_inputs,
        unsigned int num_outputs,
        unsigned int sample_rate,
        unsigned int buffer_size)
        : Application(num_inputs, num_outputs, sample_rate, buffer_size)
        , mtx(new std::mutex)
        , last_emit(0)
        , playhead(std::numeric_limits<float>::infinity())
        , current_snd_index(0)
    {
        load_next();

        emit_speed = 100;

        global_volume = 1;
    }

    ~SoundFilePlayer()
    {}

    bool load(const std::string& fp)
    {
        std::lock_guard<std::mutex> lock(*mtx);

        playhead = std::numeric_limits<float>::infinity();

    	SF_INFO sinfo;
    	sinfo.format = 0;

    	SNDFILE* sf = sf_open(fp.c_str(), SFM_READ, &sinfo);

    	if (sf == NULL)
    	{
    		std::cerr << "file not found: " << fp << std::endl;
    		return false;
    	}

        if (sinfo.channels != 2)
        {
            std::cerr << "stereo file required: " << fp << std::endl;
            return false;
        }

    	int channels = sinfo.channels;
    	int samplerate = sinfo.samplerate;

    	const size_t read_frames = 2048;
    	const size_t read_samples = read_frames * channels;

    	float buf[read_samples * 2];
        samples.clear();

    	while (true)
    	{
    		sf_count_t n = sf_read_float(sf, buf, read_frames);
    		samples.insert(samples.end(), buf, buf + n);
    		if (n < read_frames) break;
    	}

    	sf_close(sf);

        sample_size = samples.size() / 2;

        return true;
    }

    void play()
    {
        unsigned long long T = GetSystemTimeMilis();
        if (T - last_emit < emit_speed) return;

        playhead = 0;
        last_emit = T;
    }

    void audioIO(const float* input, float* output, unsigned int num_frames) override
    {
        std::lock_guard<std::mutex> lock(*mtx);

        if (playhead >= sample_size) return;

        {
            int remain = sample_size - playhead;
            int write_frames = std::min<int>(remain, num_frames);

            float* dst = output;
            const float* src = &samples[playhead * 2];

            memcpy(dst, src, sizeof(float) * write_frames * 2);

            playhead += write_frames;
        }

        {
            float* dst = output;
            for (int i = 0; i < num_frames; i++)
            {
                dst[0] *= global_volume;
                dst[1] *= global_volume;
                dst += 2;
            }
        }
    }

    std::vector<std::string> reload_dir()
    {
        std::vector<std::string> snd_files;

        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir("src/snd")) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                std::string fn = ent->d_name;
                std::string ext = fn.substr(fn.find_last_of("."));

                if (ext == ".aif"
                    || ext == ".aiff")
                {
                    std::string fp = "src/snd/" + fn;
                    snd_files.push_back(fp);
                }
            }
            closedir(dir);
        } else {
            std::cerr << "snd dir not found" << std::endl;
        }

        return snd_files;
    }

    void load_next()
    {
        std::vector<std::string> files = reload_dir();
        if (files.size() == 0) return;

        load(files[current_snd_index]);

        current_snd_index++;
        current_snd_index %= files.size();
    }

    void run() override
    {
        using namespace std;

        running = true;

        bool pressed = false;

        std::thread serial_thread([&]{
            mraa_uart_context uart = mraa_uart_init(0);
            assert(uart != NULL);

            mraa_uart_set_baudrate(uart, 115200);

            while (running)
            {
                if (mraa_uart_data_available(uart, 10))
                {
                    char buffer[256];
                    int n = mraa_uart_read(uart, buffer, 256);

                    if (n == 2)
                    {
                        char ctrl = buffer[0];

                        if (ctrl == 'a')
                        {
                            play();
                        }
                        else if (ctrl == 'c')
                        {
                            if (pressed == false)
                            {
                                emit_speed -= 10;
                                if (emit_speed < 10) emit_speed = 10;
                            }
                            else
                            {
                                global_volume -= 0.05;
                                if (global_volume < 0.05) global_volume = 0.05;
                            }
                        }
                        else if (ctrl == 'b')
                        {
                            if (pressed == false)
                            {
                                emit_speed += 10;
                                if (emit_speed >= 300) emit_speed = 300;
                            }
                            else
                            {
                                global_volume += 0.05;
                                if (global_volume > 1) global_volume = 1;
                            }
                        }
                        else if (ctrl == 'd')
                        {
                            load_next();
                            pressed = true;
                        }
                        else if (ctrl == 'e')
                        {
                            pressed = false;
                        }
                    }
                }
            }
        });

        while (1)
        {
            string s;
            cin >> s;

            if (s == "1")
            {
                play();
            }

            if (s == ".")
            {
                load_next();
            }

            if (s == "q")
            {
                running = false;
                cout << "quit" << endl;
                break;
            }
        }

        serial_thread.join();
    }

protected:

    std::mutex* mtx;

    float playhead;
    std::vector<float> samples;
    size_t sample_size;

    int current_snd_index;
    bool running;
};
