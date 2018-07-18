#include "stdafx.hpp"
#include "custom_audio_device.hpp"

using namespace std;
using namespace rtc;
using namespace webrtc;

//static const int kSamplesPerSecond = 48000;
static const int kClockDriftMs = 0;
static const uint32_t kMaxVolume = 255;

scoped_refptr<CustomAudioDeviceModule> CustomAudioDeviceModule::Create(bool customRecording, bool customPlayout) {
    string tag = "CAD";
    if (customRecording) {
        tag += "-rec";
    }
    if (customPlayout) {
        tag += "-play";
    }

    // Create the generic reference counted (platform independent) implementation.
    rtc::scoped_refptr<CustomAudioDeviceModule> audioDevice(new rtc::RefCountedObject<CustomAudioDeviceModule>(customRecording, customPlayout, tag));
    
    // Ensure that the current platform is supported.
    if (audioDevice->CheckPlatform() == -1) {
        return nullptr;
    }
    
    // Create the platform-dependent implementation.
    if (audioDevice->CreatePlatformSpecificObjects() == -1) {
        return nullptr;
    }
    
    // Ensure that the generic audio buffer can communicate with the platform
    // specific parts.
    if (audioDevice->AttachAudioBuffer() == -1) {
        return nullptr;
    }
    
    return audioDevice;
}

CustomAudioDeviceModule::CustomAudioDeviceModule(bool customRecording, bool customPlayout, const string &tag)
: BaseObject(tag)
, AudioDeviceModuleImpl(AudioLayer::kPlatformDefaultAudio)
, isRecording_(false)
, isPlaying_(false)
, currentMicLevel_(kMaxVolume)
, customRecording_(customRecording)
, customPlayout_(customPlayout) {
//    if (customRecording_) {
//        captureWorkerThread_.reset(new PlatformThread(RunCapture, this, "CustomCaptureWorkerThread"));
//    }

    if (customPlayout_) {
        renderWorkerThread_.reset(new PlatformThread(RunRender, this, "CustomRenderWorkerThread"));
    }
}

int32_t CustomAudioDeviceModule::InitSpeaker() {
    int32_t res = AudioDeviceModuleImpl::InitSpeaker();
    int32_t resVol = MicrophoneVolume(&currentMicLevel_);
    log("Mic level: %d. Res: %d", currentMicLevel_, resVol);
    return res;
}

int32_t CustomAudioDeviceModule::RegisterAudioCallback(AudioTransport* audioCallback) {
    log("RegisterAudioCallback: %d", audioCallback);
    if (customRecording_ || customPlayout_) {
        audioTransport_ = audioCallback;
    }
    return AudioDeviceModuleImpl::RegisterAudioCallback(audioCallback);
};

int32_t CustomAudioDeviceModule::StartPlayout() {
//    if (!isSrc_) return 0;
    log("StartPlayout");
    if (customPlayout_) {
        renderWorkerThread_->Start();
        renderWorkerThread_->SetPriority(ThreadPriority::kRealtimePriority);
        isPlaying_ = true;
        return 0;
    }
    int32_t res = AudioDeviceModuleImpl::StartPlayout();
//    log("StartPlayout: %d", res);
    return res;
}

int32_t CustomAudioDeviceModule::StopPlayout() {
//    if (!isSrc_) return 0;
    log("StopPlayout");
    if (customPlayout_) {
        renderWorkerThread_->Stop();
        isPlaying_ = false;
        return 0;
    }
    int32_t res = AudioDeviceModuleImpl::StopPlayout();
//    log("StopPlayout: %d", res);
    return res;
}

bool CustomAudioDeviceModule::Playing() const {
    return isPlaying_;
}

int32_t CustomAudioDeviceModule::StartRecording() {
//    if (!isSrc_) return 0;
    log("StartRecording");
    if (customRecording_) {
//        captureWorkerThread_->Start();
//        captureWorkerThread_->SetPriority(ThreadPriority::kRealtimePriority);
        isRecording_ = true;
        return 0;
    }
    int32_t res = AudioDeviceModuleImpl::StartRecording();
//    log("StartRecording: %d", res);
    return res;
}

int32_t CustomAudioDeviceModule::StopRecording() {
//    if (!isSrc_) return 0;
    log("StopRecording");
    if (customRecording_) {
//        captureWorkerThread_->Stop();
        isRecording_ = false;
        return 0;
    }
    int32_t res = AudioDeviceModuleImpl::StopRecording();
//    log("StopRecording: %d", res);
    return res;
}

bool CustomAudioDeviceModule::Recording() const {
    return isRecording_;
}

//int32_t CustomAudioDeviceModule::SetMicrophoneVolume(uint32_t volume) {
//    log("SetMicrophoneVolume: %d", volume);
//    return AudioDeviceModuleImpl::SetMicrophoneVolume(volume);
//}
//
//int32_t CustomAudioDeviceModule::MicrophoneVolume(uint32_t* volume) const
//{
//    if (customRecording_) {
//        *volume = currentMicLevel_;
//        return 0;
//    }
//    return AudioDeviceModuleImpl::MicrophoneVolume(volume);
//}
//
//int32_t CustomAudioDeviceModule::MaxMicrophoneVolume(uint32_t* max_volume) const
//{
//    if (customRecording_) {
//        *max_volume = kMaxVolume;
//        return 0;
//    }
//    return AudioDeviceModuleImpl::MaxMicrophoneVolume(max_volume);
//}

int32_t CustomAudioDeviceModule::PushFrame(const void* audioData,
                                            int bytesPerFrame,
                                            int sampleRate,
                                            size_t numberOfChannels)
{
    if (!audioTransport_) return 0;
    
    return audioTransport_->RecordedDataIsAvailable(
                                                     audioData,
                                                     sampleRate / 100,
                                                     bytesPerFrame,
                                                     numberOfChannels,
                                                     sampleRate,
                                                     0,
                                                     kClockDriftMs, currentMicLevel_,
                                                     false,
                                                     currentMicLevel_);
}

int32_t CustomAudioDeviceModule::ReceiveMonoFrame10Ms(float* data, int32_t sampleRate)
{
    if (!customPlayout_)
        throw std::logic_error("Method not supported");
    
    if (!data) return -1;
    
    try
    {
        auto samples = sampleRate / 100;
        recBuffer_.resize(samples);
        size_t nSamplesOut;
        int64_t elapsedTime;
        int64_t ntpTime;
        
        auto result = audioTransport_->NeedMorePlayData(samples, sizeof(int16_t), 1, sampleRate, recBuffer_.data(), nSamplesOut, &elapsedTime, &ntpTime);
        if (result == -1)
            return -1;
        else if (elapsedTime == -1) //sound hasn't been received yet
            return 0;
        
        for (size_t i = 0; i < samples; ++i)
            data[i] = static_cast<float>(recBuffer_[i]) / static_cast<float>(0x7fff);
        
        return static_cast<int32_t>(elapsedTime);
    }
    catch (std::exception&)
    {
        return -1;
    }
}

void CustomAudioDeviceModule::TransmitAudio10Ms(const float* data, int32_t sampleRate, int32_t channels)
{
    if (!customRecording_)
        throw std::logic_error("Method not supported");
        
    if (data == nullptr)
        throw std::invalid_argument("data cannot be null");
    
    if (channels != 1 && channels != 2)
        throw std::invalid_argument("invalid channels count");
    
    if (!(sampleRate > 0 && sampleRate % 100 == 0))
        throw std::invalid_argument("invalid sample rate");
    
    auto frameSize = sampleRate / 100 * channels;
    
    audioBuffer_.resize(frameSize);
    
    for (size_t i = 0; i < frameSize; ++i)
        audioBuffer_[i] = static_cast<int16_t>(data[i] * 0x7fff);
    
    size_t pos = 0;
    while (pos + frameSize - 1 < frameSize)
    {
        PushFrame(audioBuffer_.data() + pos, sizeof(int16_t) * channels, sampleRate, channels);
        pos += frameSize;
    }
}

void CustomAudioDeviceModule::TransmitAudio10Ms_i(const int16_t* data, int32_t sampleRate, int32_t channels)
{
    if (!customRecording_)
        throw std::logic_error("Method not supported");
    
    if (data == nullptr)
        throw std::invalid_argument("data cannot be null");
    
    if (channels != 1 && channels != 2)
        throw std::invalid_argument("invalid channels count");
    
    if (!(sampleRate > 0 && sampleRate % 100 == 0))
        throw std::invalid_argument("invalid sample rate");
    
    auto frameSize = sampleRate / 100 * channels;
    
    audioBuffer_.resize(frameSize);
    
    for (size_t i = 0; i < frameSize; ++i)
        audioBuffer_[i] = data[i];
    
    size_t pos = 0;
    while (pos + frameSize - 1 < frameSize)
    {
        PushFrame(audioBuffer_.data() + pos, sizeof(int16_t) * channels, sampleRate, channels);
        pos += frameSize;
    }
}

bool CustomAudioDeviceModule::RunRender(void* ptrThis) {
    return static_cast<CustomAudioDeviceModule*>(ptrThis)->RenderWorkerThread();
}

bool CustomAudioDeviceModule::RenderWorkerThread() {
    static float data[480];
    ReceiveMonoFrame10Ms(data, 48000);
    Thread::Current()->SleepMs(10);
    return true;
}

bool CustomAudioDeviceModule::RunCapture(void* ptrThis) {
    return static_cast<CustomAudioDeviceModule*>(ptrThis)->CaptureWorkerThread();
}

bool CustomAudioDeviceModule::CaptureWorkerThread() {
    Thread::Current()->SleepMs(10);
    return true;
}

