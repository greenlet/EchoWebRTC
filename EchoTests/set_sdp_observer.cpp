#include "stdafx.hpp"
#include "set_sdp_observer.hpp"

SetSDPObserver *SetSDPObserver::Create(bool local, SetSDPListener *listener) {
    return new rtc::RefCountedObject<SetSDPObserver>(local, listener);
}

SetSDPObserver::SetSDPObserver(bool local, SetSDPListener *listener)
: local_(local)
, listener_(listener) {};

void SetSDPObserver::OnSuccess() {
    if (listener_) {
        listener_->OnSetSDPSuccess(local_);
        listener_ = nullptr;
    }
};

void SetSDPObserver::OnFailure(const std::string &error) {
    if (listener_) {
        listener_->OnSetSDPFailure(local_, error);
        listener_ = nullptr;
    }
}

