#include "pch.h"
#include "Sound/AudioManager.hpp"
#include "../../Libraries/FMOD/inc/fmod.h"
#include "../../Libraries/FMOD/inc/fmod_errors.h"

AudioManager::AudioManager(): m_system(nullptr)
{
}

AudioManager::~AudioManager()
{
	shutdown();
}

bool AudioManager::initialize() 
{
	// Create FMOD system with correct version
	FMOD_RESULT result = FMOD_System_Create(&m_system, FMOD_VERSION);
	if (result != FMOD_OK) 
	{
		std::cerr << "Failed to create FMOD system: " << FMOD_ErrorString(result) <<
			std::endl;
		return false;
	}
	// Initialize FMOD system
	result = FMOD_System_Init(m_system, 32, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) 
	{
		std::cerr << "Failed to initialize FMOD system: " << FMOD_ErrorString(result) <<
			std::endl;
		return false;
	}
	std::cout << "AudioManager initialized successfully" << std::endl;
	return true;
}

void AudioManager::shutdown() 
{
	if (m_system) 
	{
		// Stop all sounds
		stopAllSounds();
		// Unload all sounds
		unloadAllSounds();
		// Close and release FMOD system
		FMOD_System_Close(m_system);
		FMOD_System_Release(m_system);
		m_system = nullptr;
		std::cout << "AudioManager shutdown complete" << std::endl;
	}
}

void AudioManager::update() 
{
	if (m_system) 
	{
		FMOD_System_Update(m_system);
	}
}

bool AudioManager::loadSound(const std::string& name, const std::string& filePath, bool	loop) 
{
	if (!m_system) 
	{
		std::cerr << "AudioManager not initialized" << std::endl;
		return false;
	}
	
	// Check if sound is already loaded
	if (m_sounds.find(name) != m_sounds.end()) 
	{
		std::cout << "Sound '" << name << "' is already loaded" << std::endl;
		return true;
	}
	
	std::string fullPath = getFullPath(filePath);
	
	// Check if file exists
	if (!std::filesystem::exists(fullPath)) 
	{
		std::cerr << "Audio file not found: " << fullPath << std::endl;
		return false;
	}
	
	FMOD_SOUND* sound = nullptr;
	FMOD_MODE mode = FMOD_DEFAULT;
	
	if (loop) 
	{
		mode |= FMOD_LOOP_NORMAL;
	}
	
	FMOD_RESULT result = FMOD_System_CreateSound(m_system, fullPath.c_str(), mode, nullptr,	&sound);
	
	if (result != FMOD_OK) 
	{
		std::cerr << "Failed to load sound '" << name << "': " << FMOD_ErrorString(result)
			<< std::endl;
		return false;
	}
	
	m_sounds[name] = sound;
	std::cout << "Loaded sound: " << name << " from " << fullPath << std::endl;
	
	return true;
}

void AudioManager::unloadSound(const std::string& name)
{
	auto it = m_sounds.find(name);
	if (it != m_sounds.end()) 
	{
		FMOD_Sound_Release(it->second);
		m_sounds.erase(it);
		m_channels.erase(name); // Also remove any associated channel
		std::cout << "Unloaded sound: " << name << std::endl;
	}
	else 
	{
		std::cerr << "Sound '" << name << "' not found." << std::endl;
	}
}

void AudioManager::unloadAllSounds() 
{
	for (auto& pair : m_sounds) 
	{
		FMOD_Sound_Release(pair.second);
	}
	m_sounds.clear();
	std::cout << "Unloaded all sounds" << std::endl;
}

bool AudioManager::playSound(const std::string& name, float volume, float pitch) 
{
	if (!m_system) 
	{
		std::cerr << "AudioManager not initialized" << std::endl;
		return false;
	}
	
	auto it = m_sounds.find(name);
	if (it == m_sounds.end()) 
	{
		std::cerr << "Sound '" << name << "' not loaded" << std::endl;
		return false;
	}
	FMOD_CHANNEL* channel = nullptr;
	FMOD_RESULT result = FMOD_System_PlaySound(m_system, it->second, nullptr, false, &channel);
	
	if (result != FMOD_OK) 
	{
		std::cerr << "Failed to play sound '" << name << "': " << FMOD_ErrorString(result)
			<< std::endl;
		return false;
	}
	
	// Set volume and pitch
	FMOD_Channel_SetVolume(channel, volume);
	FMOD_Channel_SetPitch(channel, pitch);

	// Store channel for later control
	m_channels[name] = channel;
	std::cout << "Playing sound: " << name << std::endl;
	return true;
}

void AudioManager::stopSound(const std::string& name) 
{
	auto it = m_channels.find(name);
	if (it != m_channels.end()) 
	{
		FMOD_Channel_Stop(it->second);
		m_channels.erase(it);
		std::cout << "Stopped sound: " << name << std::endl;
	}
	else 
	{
		std::cerr << "Sound '" << name << "' is not playing." << std::endl;
	}
}

void AudioManager::stopAllSounds() 
{
	for (auto& pair : m_channels) 
	{
		FMOD_Channel_Stop(pair.second);
	}
	m_channels.clear();
	std::cout << "Stopped all sounds" << std::endl;
}

void AudioManager::pauseSound(const std::string& name, bool pause) 
{
	auto it = m_channels.find(name);
	if (it != m_channels.end()) 
	{
		FMOD_Channel_SetPaused(it->second, pause);
		std::cout << (pause ? "Paused" : "Resumed") << " sound: " << name << std::endl;
	}
	else 
	{
		std::cerr << "Sound '" << name << "' is not playing." << std::endl;
	}
}

void AudioManager::pauseAllSounds(bool pause) 
{
	for (auto& pair : m_channels) 
	{
		FMOD_Channel_SetPaused(pair.second, pause);
	}
	std::cout << (pause ? "Paused" : "Resumed") << " all sounds" << std::endl;
}


void AudioManager::setMasterVolume(float volume) {
	if (m_system) 
	{
		FMOD_CHANNELGROUP* masterGroup = nullptr;
		FMOD_RESULT result = FMOD_System_GetMasterChannelGroup(m_system, &masterGroup);
		
		if (result == FMOD_OK && masterGroup) 
		{
			FMOD_ChannelGroup_SetVolume(masterGroup, volume);
			std::cout << "Set master volume to: " << volume << std::endl;
		}
		else 
		{
			std::cerr << "Failed to get master channel group: " << FMOD_ErrorString(result) << std::endl;
		}
	}
}

void AudioManager::setSoundVolume(const std::string& name, float volume) 
{
	auto it = m_channels.find(name);
	if (it != m_channels.end()) 
	{
		FMOD_Channel_SetVolume(it->second, volume);
		std::cout << "Set volume of sound '" << name << "' to: " << volume << std::endl;
	}
	else 
	{
		std::cerr << "Sound '" << name << "' is not playing." << std::endl;
	}
}

void AudioManager::setSoundPitch(const std::string& name, float pitch) 
{
	auto it = m_channels.find(name);
	if (it != m_channels.end()) 
	{
		FMOD_Channel_SetPitch(it->second, pitch);
		std::cout << "Set pitch of sound '" << name << "' to: " << pitch << std::endl;
	}
	else 
	{
		std::cerr << "Sound '" << name << "' is not playing." << std::endl;
	}
}


bool AudioManager::isSoundLoaded(const std::string& name) const 
{
	return m_sounds.find(name) != m_sounds.end();
}

bool AudioManager::isSoundPlaying(const std::string& name) const 
{
	auto it = m_channels.find(name);
	if (it != m_channels.end()) 
	{
		FMOD_BOOL isPlaying = false;
		FMOD_Channel_IsPlaying(it->second, &isPlaying);
		return isPlaying != 0;
	}
	return false;
}

std::vector<std::string> AudioManager::getLoadedSounds() const 
{
	std::vector<std::string> soundNames;
	for (const auto& pair : m_sounds) 
	{
		soundNames.push_back(pair.first);
	}
	return soundNames;
}


// Additional methods implementation...
std::string AudioManager::getFullPath(const std::string& fileName) const 
{
	// Try to find the audio file in the game-assets directory
	std::filesystem::path currentPath = std::filesystem::current_path();

	// Try different possible paths
	std::vector<std::filesystem::path> possiblePaths = {
	currentPath / "Resources" / "Audio" / "sfx" / fileName,
	currentPath / ".." / "Resources" / "Audio" / "sfx" / fileName,
	currentPath / ".." / ".." / "Resources" / "Audio" / "sfx" / fileName,
	currentPath / ".." / ".." / ".." / "Resources" / "Audio" / "sfx" / fileName
	};
	
	for (const auto& path : possiblePaths) 
	{
		if (std::filesystem::exists(path)) 
		{
			return path.string();
		}
	}
	
	// If not found, return the original filename
	return fileName;
}


void AudioManager::checkFMODError(int result, const std::string& operation) const 
{
	FMOD_RESULT fmodResult = static_cast<FMOD_RESULT>(result);
	if (fmodResult != FMOD_OK)
	{
		std::cerr << "FMOD error during " << operation << ": " << FMOD_ErrorString(fmodResult) << std::endl;
	}
}


// Static interface implementations
bool AudioManager::staticInitalize() 
{
	return instance().initialize();
}

void AudioManager::staticShutdown() 
{
	instance().shutdown();
}

void AudioManager::staticUpdate() 
{
	instance().update();
}

bool AudioManager::staticLoadSound(const std::string& name, const std::string& file, bool loop) 
{
	return instance().loadSound(name, file, loop);
}

bool AudioManager::staticPlaySound(const std::string& name, float vol, float pitch) 
{
	return instance().playSound(name, vol, pitch);
}

void AudioManager::staticStopAllSounds() 
{
	instance().stopAllSounds();
}

void AudioManager::staticSetMasterVolume(float v) 
{
	instance().setMasterVolume(v);
}

