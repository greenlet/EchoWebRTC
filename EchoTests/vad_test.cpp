#include "stdafx.hpp"
#include "vad_test.hpp"

using namespace std;
using namespace webrtc;
using namespace rtc;

void VADTest::RunTest() {
    auto factory = CreatePeerConnectionFactory(nullptr, nullptr);
    cricket::AudioOptions options;
    auto source = factory->CreateAudioSource(options);
    auto track = factory->CreateAudioTrack("audio_track", source);
    auto stream = factory->CreateLocalMediaStream("audio_stream");
    stream->AddTrack(track);
    
}
