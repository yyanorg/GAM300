#pragma once

#include "pch.h"

struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;
struct FMOD_CHANNELGROUP;

class AudioManager
{
public:
	
	// Static access to the singleton instance
	static bool staticInitalize();     // calls instance().initialize()
    static void staticShutdown();       // calls instance().shutdown()
    static void staticUpdate();         // calls instance().update()

    // (Optional static helpers if you want to use the same style elsewhere)
    static bool staticLoadSound(const std::string& name, const std::string& file, bool loop=false);
    static bool staticPlaySound(const std::string& name, float vol=1.f, float pitch=1.f);
    static void staticStopAllSounds();
    static void staticSetMasterVolume(float v);

	AudioManager();
	~AudioManager();

	// Initialize and cleanup
	bool initialize();
	void shutdown();
	void update(); // Call this every frame

	// Sound loading and management
	bool loadSound(const std::string& name, const std::string& filePath, bool loop = false);
	void unloadSound(const std::string& name);
	void unloadAllSounds();

	// Sound playback
	bool playSound(const std::string& name, float volume = 1.0f, float pitch = 1.0f);
	//bool playSound3D(const std::string& name, float x, float y, float z, float volume = 1.0f);
	void stopSound(const std::string& name);
	void stopAllSounds();
	void pauseSound(const std::string& name, bool pause = true);
	void pauseAllSounds(bool pause = true);

	// Volume and pitch control
	void setMasterVolume(float volume);
	void setSoundVolume(const std::string& name, float volume);
	void setSoundPitch(const std::string& name, float pitch);

	// 3D audio settings
	//void setListenerPosition(float x, float y, float z);
	//void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
	//	float upX, float upY, float upZ);

	// Utility functions
	bool isSoundLoaded(const std::string& name) const;
	bool isSoundPlaying(const std::string& name) const;
	std::vector<std::string> getLoadedSounds() const;

private:
	static AudioManager& instance() {
		static AudioManager instance;
		return instance;
	}

	FMOD_SYSTEM* m_system;
	std::unordered_map<std::string, FMOD_SOUND*> m_sounds;
	std::unordered_map<std::string, FMOD_CHANNEL*> m_channels;
	
	// Helper functions
	std::string getFullPath(const std::string& fileName) const;
	void checkFMODError(int result, const std::string& operation) const;
};