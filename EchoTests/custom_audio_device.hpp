#ifndef custom_audio_device_hpp
#define custom_audio_device_hpp

#include "stdafx.hpp"
#include "base_object.hpp"

class CustomAudioDeviceModule
: public BaseObject
, public webrtc::AudioDeviceModuleImpl {
public:
    static rtc::scoped_refptr<CustomAudioDeviceModule> Create(bool customRecording, bool customPlayout);
    
//    int32_t ActiveAudioLayer(AudioLayer* audio_layer) const override;
//    ErrorCode LastError() const override;
    int32_t RegisterAudioCallback(webrtc::AudioTransport* audio_callback) override;
    
//    int32_t Init() override;
//    int32_t Terminate() override;
//    bool Initialized() const override;
//
//    int16_t PlayoutDevices() override;
//    int16_t RecordingDevices() override;
//    int32_t PlayoutDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;
//    int32_t RecordingDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;
//
//    int32_t SetPlayoutDevice(uint16_t index) override;
//    int32_t SetPlayoutDevice(WindowsDeviceType device) override;
//    int32_t SetRecordingDevice(uint16_t index) override;
//    int32_t SetRecordingDevice(WindowsDeviceType device) override;
    
//    int32_t PlayoutIsAvailable(bool* available) override;
//    int32_t InitPlayout() override;
//    bool PlayoutIsInitialized() const override;
//    int32_t RecordingIsAvailable(bool* available) override;
//    int32_t InitRecording() override;
//    bool RecordingIsInitialized() const override;
    
    int32_t StartPlayout() override;
    int32_t StopPlayout() override;
    bool Playing() const override;
    int32_t StartRecording() override;
    int32_t StopRecording() override;
    bool Recording() const override;
    
//    int32_t SetAGC(bool enable) override;
//    bool AGC() const override;
//
    int32_t InitSpeaker() override;
//    bool SpeakerIsInitialized() const override;
//    int32_t InitMicrophone() override;
//    bool MicrophoneIsInitialized() const override;
//
//    int32_t SpeakerVolumeIsAvailable(bool* available) override;
//    int32_t SetSpeakerVolume(uint32_t volume) override;
//    int32_t SpeakerVolume(uint32_t* volume) const override;
//    int32_t MaxSpeakerVolume(uint32_t* max_volume) const override;
//    int32_t MinSpeakerVolume(uint32_t* min_volume) const override;
//
//    int32_t MicrophoneVolumeIsAvailable(bool* available) override;
//    int32_t SetMicrophoneVolume(uint32_t volume) override;
//    int32_t MicrophoneVolume(uint32_t* volume) const override;
//    int32_t MaxMicrophoneVolume(uint32_t* max_volume) const override;
//    int32_t MinMicrophoneVolume(uint32_t* min_volume) const override;
//
//    int32_t SpeakerMuteIsAvailable(bool* available) override;
//    int32_t SetSpeakerMute(bool enable) override;
//    int32_t SpeakerMute(bool* enabled) const override;
//
//    int32_t MicrophoneMuteIsAvailable(bool* available) override;
//    int32_t SetMicrophoneMute(bool enable) override;
//    int32_t MicrophoneMute(bool* enabled) const override;
//
//    int32_t StereoPlayoutIsAvailable(bool* available) const override;
//    int32_t SetStereoPlayout(bool enable) override;
//    int32_t StereoPlayout(bool* enabled) const override;
//    int32_t StereoRecordingIsAvailable(bool* available) const override;
//    int32_t SetStereoRecording(bool enable) override;
//    int32_t StereoRecording(bool* enabled) const override;
//    int32_t SetRecordingChannel(const ChannelType channel) override;
//    int32_t RecordingChannel(ChannelType* channel) const override;
//
//    int32_t PlayoutDelay(uint16_t* delay_ms) const override;
//    int32_t RecordingDelay(uint16_t* delay_ms) const override;
//
//    int32_t SetRecordingSampleRate(const uint32_t samples_per_sec) override;
//    int32_t RecordingSampleRate(uint32_t* samples_per_sec) const override;
//    int32_t SetPlayoutSampleRate(const uint32_t samples_per_sec) override;
//    int32_t PlayoutSampleRate(uint32_t* samples_per_sec) const override;
//
//    int32_t SetLoudspeakerStatus(bool enable) override;
//    int32_t GetLoudspeakerStatus(bool* enabled) const override;
//
//    bool BuiltInAECIsAvailable() const override { return false; }
//    int32_t EnableBuiltInAEC(bool enable) override { return -1; }
//    bool BuiltInAGCIsAvailable() const override { return false; }
//    int32_t EnableBuiltInAGC(bool enable) override { return -1; }
//    bool BuiltInNSIsAvailable() const override { return false; }
//    int32_t EnableBuiltInNS(bool enable) override { return -1; }

    void TransmitAudio10Ms(const float* data, int32_t sampleRate, int32_t channels);
    void TransmitAudio10Ms_i(const int16_t* data, int32_t sampleRate, int32_t channels);
    int32_t ReceiveMonoFrame10Ms(float* data, int32_t sampleRate);

protected:
    CustomAudioDeviceModule(bool customRecording, bool customPlayout, const std::string &tag = "CAD");
    
private:
    int32_t PushFrame(const void* audioData, int bitsPerSample, int sampleRate, size_t numberOfChannels);
    
    webrtc::AudioTransport* audioTransport_;
    bool isRecording_;
    bool isPlaying_;
    uint32_t currentMicLevel_;
    std::vector<int16_t> recBuffer_;
    std::vector<int16_t> audioBuffer_;
    
    bool customRecording_;
    bool customPlayout_;

    static bool RunCapture(void*);
    static bool RunRender(void*);
    bool CaptureWorkerThread();
    bool RenderWorkerThread();

    std::unique_ptr<rtc::PlatformThread> captureWorkerThread_;
    std::unique_ptr<rtc::PlatformThread> renderWorkerThread_;
};

#endif /* custom_audio_device_hpp */
