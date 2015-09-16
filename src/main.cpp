#include "AudioDevice.h"
#include "app/SFPlayer.h"

#include <assert.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <memory>

using namespace std;

std::shared_ptr<AudioDevice> audio;

static void sig_handler(int sig)
{
    assert(sig == SIGINT || sig == SIGHUP || sig == SIGTERM);
    audio.reset();
}

static void exit_handler()
{
    audio.reset();
}

int main(int, const char**)
{
    cout << "start" << endl;

    if (signal(SIGHUP, sig_handler) == SIG_ERR)
        return 1;
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        return 2;
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
        return 3;
    atexit (exit_handler);

    audio = std::make_shared<AudioDevice>();

    try
    {
        if (audio->open("USB PnP Sound Device", "USB PnP Sound Device", 0, 2) == false)
        {
            audio.reset();
            return -1;
        }

        cout << "stream opend. launching app" << endl;

        audio->run<SoundFilePlayer>();
    }
    catch (std::exception& e)
    {
        audio.reset();
    }

    return 0;
}
