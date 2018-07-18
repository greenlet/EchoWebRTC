#ifndef set_sdp_observer_hpp
#define set_sdp_observer_hpp

#include "stdafx.hpp"

class SetSDPObserver : public webrtc::SetSessionDescriptionObserver {
public:
    class SetSDPListener {
    public:
        virtual void OnSetSDPSuccess(bool local) = 0;
        virtual void OnSetSDPFailure(bool local, const std::string &error) = 0;
    };
    
    SetSDPObserver(bool local, SetSDPListener *listener);
    
    void OnSuccess() override;
    
    void OnFailure(const std::string &error) override;
    
    static SetSDPObserver *Create(bool local, SetSDPListener *listener = nullptr);
    
private:
    bool local_;
    SetSDPListener *listener_;
};



#endif /* set_sdp_observer_hpp */
