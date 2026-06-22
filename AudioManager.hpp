#ifndef AUDIOMANAGER_HPP
#define AUDIOMANAGER_HPP

#include <string>

class AudioManager
{
private:
    bool bgmPlaying;

public:
    AudioManager();

    void playBackgroundMusic(const std::string& filePath);
    void stopBackgroundMusic();

    void playSoundEffect(const std::string& filePath);
    void stopSoundEffect();
};

#endif
