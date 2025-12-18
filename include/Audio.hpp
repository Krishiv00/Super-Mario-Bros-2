#ifndef AUDIO_MANAGER_HPP
#define AUDIO_MANAGER_HPP

#include <vector>
#include <unordered_map>

#include "Globals.hpp"
#include "SFML/Audio.hpp"

class AudioPlayer {
public:
    enum AudioEffects : uint8_t {
        PlayerJump1,
        PlayerJump2,
        Stomp,
        Swim = Stomp,
        Kick,
        StarmanKill = Kick,
        BlockHit,
        PowerupSpawn,
        PowerupAcquire,
        CoinAcquire,
        ExtraLife,
        BrickSmash,
        Pipe,
        Damage = Pipe,
        Pause,
        UnPause = Pause,
        Count
    };

private:
    sf::Sound* m_Sounds[AudioEffects::Count];
    sf::SoundBuffer m_Buffers[AudioEffects::Count];

public:
    AudioPlayer();
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer& am) = delete;

    void LoadFromFile(uint8_t name, const std::string& path);

    void Play(uint8_t name);

    void PauseAll();
    void ResumeAll(uint8_t step_ahead = 0);
    void StopAll();

    void SetMuted(bool muted);

    inline bool IsMuted() const noexcept {
        return m_Sounds[0u]->getVolume() == 0.f;
    }
};

class MusicPlayer {
public:
    enum Music : uint8_t {
        Clear_1,
        Clear_2,
        Death,
        Flag,
        Star,
        MainTheme,
        Count
    };

private:
    sf::Music* m_MusicBuffers[Music::Count];
    uint8_t m_CurrentlyPlayingMusic = 0u;

public:
    MusicPlayer();
    ~MusicPlayer();

    void LoadFromFile(uint8_t name, const std::string& path);

    void Play(uint8_t name, bool loop = false);
    void PlayLast(bool loop = false);
    void Pause();
    void Resume(uint8_t step_ahead = 0);
    void Stop();

    void SetMuted(bool muted);

    inline bool IsMuted() const noexcept {
        return m_MusicBuffers[0u]->getVolume() == 0.f;
    }
};

extern AudioPlayer audioPlayer;
extern MusicPlayer musicPlayer;

#endif // !AUDIO_MANAGER_HPP