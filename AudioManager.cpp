#include "AudioManager.hpp"

#include <windows.h>
#include <mmsystem.h>
#include <iostream>

AudioManager::AudioManager()
{
    bgmPlaying = false;
}

void AudioManager::playBackgroundMusic(const std::string& filePath)
{
    if (bgmPlaying) return;

    char fullPath[MAX_PATH];

    DWORD result = GetFullPathName(
        filePath.c_str(),
        MAX_PATH,
        fullPath,
        NULL
    );

    if (result == 0)
    {
        std::cout << "Failed to convert BGM path: " << filePath << std::endl;
        return;
    }

    DWORD fileAttribute = GetFileAttributes(fullPath);
    if (fileAttribute == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "BGM file not found: " << fullPath << std::endl;
        return;
    }

    mciSendString("close bgm", NULL, 0, NULL);

    std::string openCommand = "open \"";
    openCommand += fullPath;
    openCommand += "\" type mpegvideo alias bgm";

    MCIERROR openResult = mciSendString(openCommand.c_str(), NULL, 0, NULL);

    if (openResult != 0)
    {
        char errorText[256];
        mciGetErrorString(openResult, errorText, sizeof(errorText));

        std::cout << "Failed to open background music: " << fullPath << std::endl;
        std::cout << "MCI Error: " << errorText << std::endl;
        return;
    }

    // Lower volume so it softly plays in the background
    mciSendString("setaudio bgm volume to 150", NULL, 0, NULL);

    MCIERROR playResult = mciSendString("play bgm repeat", NULL, 0, NULL);

    if (playResult != 0)
    {
        char errorText[256];
        mciGetErrorString(playResult, errorText, sizeof(errorText));

        std::cout << "Failed to play background music: " << fullPath << std::endl;
        std::cout << "MCI Error: " << errorText << std::endl;
        return;
    }

    std::cout << "BGM playing: " << fullPath << std::endl;
    bgmPlaying = true;
}

void AudioManager::stopBackgroundMusic()
{
    mciSendString("stop bgm", NULL, 0, NULL);
    mciSendString("close bgm", NULL, 0, NULL);
    bgmPlaying = false;
}

void AudioManager::playSoundEffect(const std::string& filePath)
{
    char fullPath[MAX_PATH];

    DWORD result = GetFullPathName(
        filePath.c_str(),
        MAX_PATH,
        fullPath,
        NULL
    );

    if (result == 0)
    {
        std::cout << "Failed to convert SFX path: " << filePath << std::endl;
        return;
    }

    mciSendString("close sfx", NULL, 0, NULL);

    std::string openCommand = "open \"";
    openCommand += fullPath;
    openCommand += "\" type mpegvideo alias sfx";

    MCIERROR openResult = mciSendString(openCommand.c_str(), NULL, 0, NULL);
    if (openResult != 0)
    {
        char errorText[256];
        mciGetErrorString(openResult, errorText, sizeof(errorText));
        std::cout << "Failed to open SFX: " << fullPath << " | MCI Error: " << errorText << std::endl;
        return;
    }

    // Set SFX volume to a lower value (e.g. 300 out of 1000, which is 30% volume)
    mciSendString("setaudio sfx volume to 300", NULL, 0, NULL);

    MCIERROR playResult = mciSendString("play sfx from 0", NULL, 0, NULL);
    if (playResult != 0)
    {
        char errorText[256];
        mciGetErrorString(playResult, errorText, sizeof(errorText));
        std::cout << "Failed to play SFX: " << fullPath << " | MCI Error: " << errorText << std::endl;
    }
}

void AudioManager::stopSoundEffect()
{
    mciSendString("close sfx", NULL, 0, NULL);
}
