#include "Audio.hpp"

#pragma region Sfx

AudioPlayer::AudioPlayer() {
    for (uint8_t i = 0u; i < AudioEffects::Count; ++i) {
        m_Sounds[i] = new sf::Sound(m_Buffers[i]);
    }
}

AudioPlayer::~AudioPlayer() {
    for (sf::Sound*& sound : m_Sounds) {
        delete sound;
    }
}

void AudioPlayer::LoadFromFile(uint8_t name, const std::string& path) {
    sf::SoundBuffer& buffer = m_Buffers[name];
    sf::Sound& sound = *m_Sounds[name];

    if (!buffer.loadFromFile(path)) {
    #ifdef DEBUG
        LOG_ERROR("Error Loading Sound, File Not Found: " + path);
    #endif // DEBUG
    }

    sound.stop();
    sound.setBuffer(buffer);
}

void AudioPlayer::Play(uint8_t name) {
    m_Sounds[name]->play();
}

void AudioPlayer::PauseAll() {
    for (sf::Sound*& sound : m_Sounds) {
        if (sound->getStatus() == sf::Sound::Status::Playing) {
            sound->pause();
        }
    }
}

void AudioPlayer::ResumeAll(uint8_t step_ahead) {
    sf::Time step = sf::milliseconds((1000 * step_ahead) / 60);

    for (sf::Sound*& sound : m_Sounds) {
        if (sound->getStatus() == sf::Sound::Status::Paused) {
            if (step_ahead) {
                sf::Time newPos = sound->getPlayingOffset() + step;

                if (newPos < sound->getBuffer().getDuration()) {
                    sound->setPlayingOffset(newPos);
                    sound->play();
                } else {
                    sound->stop();
                }
            } else {
                sound->play();
            }
        }
    }
}

void AudioPlayer::StopAll() {
    for (sf::Sound*& sound : m_Sounds) {
        sound->stop();
    }
}

void AudioPlayer::SetMuted(bool muted) {
    if (muted != IsMuted()) {
        const float volume = 100.f * !muted;
        
        for (sf::Sound*& sound : m_Sounds) {
            sound->setVolume(volume);
        }
    }
}

// global audio manager instance defination
AudioPlayer audioPlayer;

#pragma region Music

MusicPlayer::MusicPlayer() {
    for (uint8_t i = 0u; i < Music::Count; ++i) {
        m_MusicBuffers[i] = new sf::Music();
    }
}

MusicPlayer::~MusicPlayer() {
    for (sf::Music*& music : m_MusicBuffers) {
        delete music;
    }
}

void MusicPlayer::LoadFromFile(uint8_t name, const std::string& path) {
    if (!m_MusicBuffers[name]->openFromFile(path)) {
        LOG_ERROR("Failed To Load Music: " << path);
    }
}

void MusicPlayer::Play(uint8_t name, bool loop) {
    m_MusicBuffers[m_CurrentlyPlayingMusic]->stop();
    m_CurrentlyPlayingMusic = name;

    sf::Music*& music = m_MusicBuffers[name];

    if (loop) {
        sf::Time offset = sf::milliseconds(2000 / 60);

        music->setLoopPoints(sf::Music::TimeSpan(offset, music->getDuration() - offset));
    }

    music->setLooping(loop);
    music->play();
}

void MusicPlayer::Pause() {
    m_MusicBuffers[m_CurrentlyPlayingMusic]->pause();
}

void MusicPlayer::Resume(uint8_t step_ahead) {
    sf::Music*& playingMusic = m_MusicBuffers[m_CurrentlyPlayingMusic];

    if (playingMusic->getStatus() == sf::Music::Status::Paused) {
        if (step_ahead) {
            playingMusic->setPlayingOffset(playingMusic->getPlayingOffset() + sf::milliseconds((1000 * step_ahead) / 60));
        }

        playingMusic->play();
    }
}

void MusicPlayer::Stop() {
    m_MusicBuffers[m_CurrentlyPlayingMusic]->stop();
}

void MusicPlayer::SetMuted(bool muted) {
    if (muted != IsMuted()) {
        const float volume = 100.f * !muted;
    
        for (sf::Music*& music : m_MusicBuffers) {
            music->setVolume(volume);
        }
    }
}

// global music player instance defination
MusicPlayer musicPlayer;