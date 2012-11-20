#ifndef __FINGER_TRACKING_EVENT_H__
#define __FINGER_TRACKING_EVENT_H__



class FingerTrackingEvent
{
public:
    enum Type { FINGER_ADDED_EVENT, FINGER_UPDATED_EVENT, FINGER_REMOVED_EVENT, NUM_EVENT_TYPES };
    DEFINE_ENUM_TO_STRING_CLASS_METHOD(Type, "FingerAdded", "FingerUpdated", "FingerRemoved", "Invalid");

    Type type;
    int id;
    Vector3D<Float> pos; // fingertip

    FingerTrackingEvent(Type type, int id, const Vector3D<Float> &pos=Vector3D<Float>()) : type(type), id(id), pos(pos) {}
};

class FingerTrackingEventListener
{
public:
    virtual void ProcessEvent(const FingerTrackingEvent &event) = 0;
};

class FingerTrackingEventGenerator
{
public:
    void AddListener(FingerTrackingEventListener *listener) { m_listeners.Push(listener); }
    void RemoveListener(FingerTrackingEventListener *listener) { m_listeners.RemoveUnorderedByValue(listener); }
    void SendEvent(const FingerTrackingEvent &event) { for(int i=0; i<m_listeners.NumElements(); i++) m_listeners[i]->ProcessEvent(event); }

private:
    Array<FingerTrackingEventListener*> m_listeners;
};

#endif /* __FINGER_TRACKING_EVENT_H__ */
