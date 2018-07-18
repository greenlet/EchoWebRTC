#include "stdafx.hpp"
#include "pc_echo.hpp"

using namespace std;
using namespace webrtc;
using namespace rtc;

#define BOOL_TO_STR(x) (x ? "true" : "false")
#define INTBOOL_TO_STR(x) (x < 0 ? "undefined" : BOOL_TO_STR(x))

string sdpOptions2Str(const PeerConnectionInterface::RTCOfferAnswerOptions &options) {
	char buf[200];
	sprintf(buf, "\toffer_to_receive_audio: %s\n"
		"\toffer_to_receive_video: %s\n"
		"\tvoice_activity_detection: %s\n"
		"\tice_restart: %s\n"
		"\tuse_rtp_mux: %s",
		INTBOOL_TO_STR(options.offer_to_receive_audio),
		INTBOOL_TO_STR(options.offer_to_receive_video),
		BOOL_TO_STR(options.voice_activity_detection),
		BOOL_TO_STR(options.ice_restart),
		BOOL_TO_STR(options.use_rtp_mux));
	return buf;
}

const char *sigState2Str(PeerConnectionInterface::SignalingState state) {
    switch (state) {
        case PeerConnectionInterface::SignalingState::kStable: return "kStable";
        case PeerConnectionInterface::SignalingState::kHaveLocalOffer: return "kHaveLocalOffer";
        case PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer: return "kHaveLocalPrAnswer";
        case PeerConnectionInterface::SignalingState::kHaveRemoteOffer: return "kHaveRemoteOffer";
        case PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer: return "kHaveRemotePrAnswer";
        case PeerConnectionInterface::SignalingState::kClosed: return "kClosed";
        default: return "Unknown";
    }
}

const char *iceConnState2Str(PeerConnectionInterface::IceConnectionState state) {
    switch (state) {
        case PeerConnectionInterface::IceConnectionState::kIceConnectionNew: return "kIceConnectionNew";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionChecking: return "kIceConnectionChecking";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionConnected: return "kIceConnectionConnected";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted: return "kIceConnectionCompleted";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionFailed: return "kIceConnectionFailed";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected: return "kIceConnectionDisconnected";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionClosed: return "kIceConnectionClosed";
        case PeerConnectionInterface::IceConnectionState::kIceConnectionMax: return "kIceConnectionMax";
        default: return "Unkonwn";
    }
}

const char *iceGathState2Str(PeerConnectionInterface::IceGatheringState state) {
    switch (state) {
        case PeerConnectionInterface::IceGatheringState::kIceGatheringNew: return "kIceGatheringNew";
        case PeerConnectionInterface::IceGatheringState::kIceGatheringGathering: return "kIceGatheringGathering";
        case PeerConnectionInterface::IceGatheringState::kIceGatheringComplete: return "kIceGatheringComplete";
        default: return "Unkonwn";
    }
}



PCEcho::PCEcho(bool isSrc)
: BaseObject(isSrc ? "PCE-src" : "PCE-dst")
, isSrc_(isSrc)
, name_ (isSrc_ ? "src" : "dst") {
}

PCEcho::~PCEcho() {
    
}

void PCEcho::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
    log("PC_OnSignalingChange: %s", sigState2Str(new_state));
}

void PCEcho::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
    log("PC_OnAddStream: %s", stream->id().c_str());
    for (auto track : stream->GetAudioTracks()) {

        log("Audio track: %s. enabled: %s. state: %d", track->id().c_str(), BOOL_TO_STR(track->enabled()), track->state());
//        track->GetSource()->SetVolume(10);
//        if (isSrc_) {
//            log("Add audio track sink");
//            track->AddSink(this);
//        }
    }
    if (!isSrc_ && isOwnFactory_) {
//        log("Redirecting remote stream back");
//        pc_->AddStream(stream);

        if (stream->GetAudioTracks().size()) {
            auto track = stream->GetAudioTracks()[0];
//            auto source = track->GetSource();
//            source->SetVolume(10);
//            audioTrack_ = factory_->CreateAudioTrack(name_ + "_audiotrack", source);
//            stream_ = factory_->CreateLocalMediaStream(name_ + "_stream");
//            stream_->AddTrack(audioTrack_);
//            log("Redirect remote stream back");
//            pc_->AddStream(stream_);
            log("AddSink for remote audio track");
            track->AddSink(this);
        }
    }
}

void PCEcho::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
    log("PC_OnRemoveStream");
}

void PCEcho::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
    log("PC_OnDataChannel");
}

void PCEcho::OnRenegotiationNeeded() {
    log("PC_OnRenegotiationNeeded");
	if (isSrc_) {
		PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = true;
		options.offer_to_receive_video = false;
		log("Create offer\n%s", sdpOptions2Str(options).c_str());
		pc_->CreateOffer(this, options);
	}
}

void PCEcho::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    log("PC_OnIceConnectionChange: %s", iceConnState2Str(new_state));
}

void PCEcho::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    log("PC_OnIceGatheringChange: %s", iceGathState2Str(new_state));
}

void PCEcho::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    log("PC_OnIceCandidate: %s", candidate->candidate().ToString().c_str());
	pceOther_->addIceCandidate(const_cast<IceCandidateInterface *>(candidate));
}

void PCEcho::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
    log("PC_OnRemoveTrack");
}

void PCEcho::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    log("PC_OnAddTrack");
}

void PCEcho::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
    string descStr;
    desc->ToString(&descStr);
    log("SDP_Create_OnSuccess:\n%s", descStr.c_str());
	pc_->SetLocalDescription(SetSDPObserver::Create(true, const_cast<PCEcho *>(this)), desc);
}

void PCEcho::OnFailure(const std::string &error) {
    log("SDP_Create_OnFailure: %s", error.c_str());
}

void PCEcho::OnSetSDPSuccess(bool local) {
	log("SDP_Set_OnSuccess (%s)", local ? "local" : "remote");
	if (local) {
		pceOther_->setRemoteSDP(const_cast<SessionDescriptionInterface *>(pc_->local_description()));
	}
	else if (!isSrc_) {
		PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = true;
		options.offer_to_receive_video = false;
		log("Create answer\n%s", sdpOptions2Str(options).c_str());
		pc_->CreateAnswer(this, options);
	}
}

void PCEcho::OnSetSDPFailure(bool local, const string &error) {
	log("SDP_Set_OnFailure (%s): %s", local ? "local" : "remote", error.c_str());
}

void PCEcho::OnData(const void* audio_data,
            int bits_per_sample,
            int sample_rate,
            size_t number_of_channels,
            size_t number_of_frames) {
//    log("ASINK_OnData. channels: %d. frames: %d", number_of_channels, number_of_frames);
    adm_->TransmitAudio10Ms_i((const int16_t*)audio_data, (int32_t)sample_rate, (int32_t)number_of_channels);
}

void PCEcho::setRemoteSDP(SessionDescriptionInterface *desc) {
	log("setRemoteSDP");
	pc_->SetRemoteDescription(SetSDPObserver::Create(false, const_cast<PCEcho *>(this)), desc);
}

void PCEcho::addIceCandidate(IceCandidateInterface *candidate) {
	pc_->AddIceCandidate(candidate);
}

void PCEcho::w_initAdm() {
    log("w_initAdm");
    if (isSrc_) {
        adm_ = CustomAudioDeviceModule::Create(false, false);
    } else {
        adm_ = CustomAudioDeviceModule::Create(true, true);
//        adm_ = CustomAudioDeviceModule::Create(false, false);
    }
}

void PCEcho::w_deinitAdm() {
    log("w_deinitAdm");
    adm_ = nullptr;
}

void PCEcho::initAdm() {
    workerThread_ = rtc::Thread::Create();
    workerThread_->Start();
    MethodCall0<PCEcho, void> call(this, &PCEcho::w_initAdm);
    call.Marshal(RTC_FROM_HERE, workerThread_.get());
}

void PCEcho::deinitAdm() {
    if (workerThread_) {
        MethodCall0<PCEcho, void> call(this, &PCEcho::w_deinitAdm);
        call.Marshal(RTC_FROM_HERE, workerThread_.get());
    }
}

void PCEcho::start(scoped_refptr<PCEcho> pceOther, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, bool echoCancellation) {
    pceOther_ = pceOther;
	
    if (factory) {
        factory_ = factory;
        isOwnFactory_ = false;
        log("Using single factory");
    } else {
        log("Creating new factory");
//        factory_ = CreatePeerConnectionFactory(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        
        initAdm();
        
		scoped_refptr<AudioEncoderFactory> audioEncoderFactory = CreateBuiltinAudioEncoderFactory();
		scoped_refptr<AudioDecoderFactory> audioDecoderFactory = CreateBuiltinAudioDecoderFactory();
		AudioProcessingBuilder audioBuilder;
		if (echoCancellation) {
			EchoCanceller3Factory *echoFactory = new EchoCanceller3Factory();
			audioBuilder.SetEchoControlFactory(unique_ptr<EchoControlFactory>(echoFactory));
		}
		scoped_refptr<AudioProcessing> audioProcessing = audioBuilder.Create();
        if (echoCancellation) {
            AudioProcessing::Config audioConfig;
            //audioConfig.residual_echo_detector.enabled = true;
            //audioProcessing->ApplyConfig(audioConfig);
            audioProcessing->echo_cancellation()->Enable(true);
			//audioProcessing->echo_cancellation()->set_suppression_level(EchoCancellation::SuppressionLevel::kHighSuppression);
			//audioProcessing->echo_cancellation()->enable_drift_compensation(true);
			Config config;
			//config.Set(new DelayAgnostic(true));
			//config.Set(new ExtendedFilter(true));
			//config.Set(new RefinedAdaptiveFilter(true));
			audioProcessing->SetExtraOptions(config);
        }
        
        factory_ = CreatePeerConnectionFactory(nullptr, workerThread_.get(), nullptr, (AudioDeviceModule *)adm_, audioEncoderFactory, audioDecoderFactory, nullptr, nullptr, nullptr, audioProcessing);
        isOwnFactory_ = true;
    }

    PeerConnectionInterface::RTCConfiguration config;
    PeerConnectionInterface::IceServer iceServer;
    iceServer.uri = "turn:turn.us-west-oregon.mursion.com:443";
    iceServer.username = "mursion";
    iceServer.password = "mursion";
    config.servers.push_back(iceServer);
    config.type = PeerConnectionInterface::IceTransportsType::kRelay;
    FakeConstraints constraints;
    pc_ = factory_->CreatePeerConnection(config, &constraints, nullptr, nullptr, this);

//    if (isSrc_) {
		cricket::AudioOptions audioOptions;
        audioOptions.echo_cancellation.emplace(echoCancellation);
		audioSource_ = factory_->CreateAudioSource(audioOptions);
		audioTrack_ = factory_->CreateAudioTrack(name_ + "_audiotrack", audioSource_);
		stream_ = factory_->CreateLocalMediaStream(name_ + "_stream");
		stream_->AddTrack(audioTrack_);
		log("Add local stream. Echo cancellation: %s", audioOptions.echo_cancellation ? "on" : "off");
		log("%s", audioOptions.ToString().c_str());
		pc_->AddStream(stream_);
//    }
}

void PCEcho::stop() {
	pc_->Close();
    deinitAdm();
}

void run(bool singleFactory, bool echoCancellation) {
    const char *capFactory = singleFactory ? "Run single factory" : "Run two factories";
    const char *capEcho = echoCancellation ? "Echo cancellation is On" : "Echo cancellation is Off";

    printf("\n------- %s start. %s -------\n\n", capFactory, capEcho);
    
    scoped_refptr<PCEcho> pcEcho1 = new RefCountedObject<PCEcho>(true);
    scoped_refptr<PCEcho> pcEcho2 = new RefCountedObject<PCEcho>(false);
    
    scoped_refptr<PeerConnectionFactoryInterface> factory;
    if (singleFactory) {
        factory = CreatePeerConnectionFactory(CreateBuiltinAudioEncoderFactory(), CreateBuiltinAudioDecoderFactory());
    }
    
    pcEcho1->start(pcEcho2, factory, echoCancellation);
    pcEcho2->start(pcEcho1, factory, echoCancellation);
    
    Thread::Current()->ProcessMessages(40*1000);
    //Thread::Current()->ProcessMessages(5*1000);
    
    pcEcho1->stop();
    pcEcho2->stop();
    
    printf("\n------- %s stop. %s -------\n\n", capFactory, capEcho);

}

void PCEcho::RunTest() {
    // One factory
//    run(true, false);
//    run(true, true);

    // Two factories
    //run(false, false);
    run(false, true);
}

