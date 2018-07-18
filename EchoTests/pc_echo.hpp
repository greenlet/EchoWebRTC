#ifndef test_pc_echo_hpp
#define test_pc_echo_hpp
#include "stdafx.hpp"
#include "custom_audio_device.hpp"
#include "base_object.hpp"
#include "set_sdp_observer.hpp"

class PCEcho
: public BaseObject
, public webrtc::PeerConnectionObserver
, public webrtc::CreateSessionDescriptionObserver
, public SetSDPObserver::SetSDPListener
, public webrtc::AudioTrackSinkInterface {

private:

    bool isSrc_;
    bool isOwnFactory_;
    std::string name_;
    std::string tag_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;
    rtc::scoped_refptr<webrtc::AudioSourceInterface> audioSource_;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack_;
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;
    rtc::scoped_refptr<PCEcho> pceOther_;
    std::unique_ptr<rtc::Thread> workerThread_;
    rtc::scoped_refptr<CustomAudioDeviceModule> adm_;
    
	void setRemoteSDP(webrtc::SessionDescriptionInterface *desc);
	void addIceCandidate(webrtc::IceCandidateInterface *candidate);
    void w_initAdm();
    void w_deinitAdm();
    void initAdm();
    void deinitAdm();

protected:
    // PeerConnectionObserver
    virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
    virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
    virtual void OnRenegotiationNeeded() override;
    virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    virtual void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    virtual void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;

    // CreateSessionDescriptionObserver
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    virtual void OnFailure(const std::string& error) override;

	// SetSDPListener
	virtual void OnSetSDPSuccess(bool local) override;
	virtual void OnSetSDPFailure(bool local, const std::string &error) override;

    // AudioTrackSinkInterface
    void OnData(const void* audio_data,
                int bits_per_sample,
                int sample_rate,
                size_t number_of_channels,
                size_t number_of_frames) override;
    
    PCEcho(bool isSrc);
    ~PCEcho();

public:
    void start(rtc::scoped_refptr<PCEcho> pceOther, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, bool echoCancellation);
    void stop();
    
    static void RunTest();
};

#endif /* test_pc_echo_hpp */
