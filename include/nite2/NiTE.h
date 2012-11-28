/*******************************************************************************
*                                                                              *
*   PrimeSense NiTE 2.0                                                        *
*   Copyright (C) 2012 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#ifndef _NITE_H_
#define _NITE_H_

#include "NiteCAPI.h"
#include <OpenNI.h>

// Summary of use cases, modules, facades

namespace nite {
#include "NiteEnums.h"

// General
_NITE_DECLARE_VERSION(Version);

class Point3f : public NitePoint3f
{
public:
	Point3f()
	{
		x = y = z = 0.0f;
	}
	Point3f(float x, float y, float z)
	{
		this->set(x, y, z);
	}
	Point3f(const Point3f& other)
	{
		*this = other;
	}

	void set(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Point3f& operator=(const Point3f& other)
	{
		set(other.x, other.y, other.z);

		return *this;
	}
	bool operator==(const Point3f& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=(const Point3f& other) const
	{
		return !operator==(other);
	}
};

class Plane : public NitePlane
{
public:
	Plane()
	{
		this->point = Point3f();
		this->normal = Point3f();
	}
	Plane(const Point3f& point, const Point3f& normal)
	{
		this->point = point;
		this->normal = normal;
	}
};
class Quaternion : protected NiteQuaternion
{
public:
	Quaternion()
	{
		x = y = z = w = 0;
	}
	Quaternion(float w, float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

class BoundingBox : public NiteBoundingBox
{
public:
	BoundingBox();
	BoundingBox(const Point3f& min, const Point3f& max)
	{
		this->min = min;
		this->max = max;
	}
};

template <class T>
class Array
{
public:
	Array() : m_size(0), m_data(NULL) {}
	void setData(int size, T* data) {m_data = data; m_size = size;}
	const T& operator[](int index) const {return m_data[index];}
	int getSize() const {return m_size;}
	bool isEmpty() const {return m_size == 0;}
private:
	Array(const Array&);
	Array& operator=(const Array&);

	int m_size;
	T* m_data;
};

// UserTracker
typedef short int UserId;

class PoseData : protected NitePoseData
{
public:
	PoseType getType() const {return (PoseType)type;}

	bool isInPose() const {return (state & NITE_POSE_STATE_IN_POSE) != 0;}
	bool isEnterPose() const {return (state & NITE_POSE_STATE_ENTER) != 0;}
	bool isExitPose() const {return (state & NITE_POSE_STATE_EXIT) != 0;}
};

class UserMap : private NiteUserMap
{
public:
	const UserId* getPixels() const {return pixels;}
	int getWidth() const {return width;}
	int getHeight() const {return height;}
	int getStride() const {return stride;}

	friend class UserTrackerFrameRef;
};

class SkeletonJoint : private NiteSkeletonJoint
{
public:
	JointType getType() const {return (JointType)jointType;}
	const Point3f& getPosition() const {return (Point3f&)position;}
	float getPositionConfidence() const {return positionConfidence;}
	const Quaternion& getOrientation() const {return (Quaternion&)orientation;}
	float getOrientationConfidence() const {return orientationConfidence;}
};
class Skeleton : private NiteSkeleton
{
public:
	const SkeletonJoint& getJoint(JointType type) const {return (SkeletonJoint&)joints[type];}
	SkeletonState getState() const {return (SkeletonState)state;}
};
class UserData : private NiteUserData
{
public:
	UserId getId() const {return id;}
	const BoundingBox& getBoundingBox() const {return (const BoundingBox&)boundingBox;}
	const Point3f& getCenterOfMass() const {return (const Point3f&)centerOfMass;}

	bool isNew() const {return (state & NITE_USER_STATE_NEW) != 0;}
	bool isVisible() const {return (state & NITE_USER_STATE_VISIBLE) != 0;}
	bool isLost() const {return (state & NITE_USER_STATE_LOST) != 0;}

	const Skeleton& getSkeleton() const {return (const Skeleton&)skeleton;}

	const PoseData& getPose(PoseType type) const {return (const PoseData&)poses[type];}
};

/** Snapshot of the User Tracker algorithm. It holds all the users identified at this time, including their position, skeleton and such, as well as the floor plane */
class UserTrackerFrameRef
{
public:
	UserTrackerFrameRef() : m_pFrame(NULL), m_userTrackerHandle(NULL)
	{}
	~UserTrackerFrameRef()
	{
		release();
	}

	UserTrackerFrameRef(const UserTrackerFrameRef& other) : m_pFrame(NULL)
	{
		*this = other;
	}
	UserTrackerFrameRef& operator=(const UserTrackerFrameRef& other)
	{
		setReference(other.m_userTrackerHandle, other.m_pFrame);
		niteUserTrackerFrameAddRef(m_userTrackerHandle, m_pFrame);

		return *this;
	}

	void release()
	{
		if (m_pFrame != NULL)
		{
			niteUserTrackerFrameRelease(m_userTrackerHandle, m_pFrame);
		}
		m_pFrame = NULL;
		m_userTrackerHandle = NULL;
	}

	const UserData* getUserById(UserId id) const
	{
		for (int i = 0; i < m_users.getSize(); ++i)
		{
			if (m_users[i].getId() == id)
			{
				return &m_users[i];
			}
		}
		return NULL;
	}

	const Array<UserData>& getUsers() const {return m_users;}

	float getFloorConfidence() const {return m_pFrame->floorConfidence;}
	const Plane& getFloor() const {return (const Plane&)m_pFrame->floor;}

	openni::VideoFrameRef getDepthFrame() {return m_depthFrame;}
	const UserMap& getUserMap() const {return static_cast<const UserMap&>(m_pFrame->userMap);}
	uint64_t getTimestamp() const {return m_pFrame->timestamp;}
	int getFrameIndex() const {return m_pFrame->frameIndex;}
private:
	friend class User;
	friend class UserTracker;

	Array<UserData> m_users;

	void setReference(NiteUserTrackerHandle userTrackerHandle, NiteUserTrackerFrame* pFrame)
	{
		release();
		m_userTrackerHandle = userTrackerHandle;
		m_pFrame = pFrame;
		m_depthFrame._setFrame(pFrame->pDepthFrame);
		m_users.setData(m_pFrame->userCount, (UserData*)m_pFrame->pUser);
		
	}

	NiteUserTrackerFrame* m_pFrame;
	NiteUserTrackerHandle m_userTrackerHandle;
	openni::VideoFrameRef m_depthFrame;
};

/**
This is the main object of the User Tracker algorithm.
Through it all the users are accessible.
*/
class UserTracker
{
public:
	class Listener
	{
	public:
		Listener() : m_pUserTracker(NULL)
		{
			m_userTrackerCallbacks.readyForNextFrame = newFrameCallback;
		}

		// SAme name as in OPenNI. Not abstract - make sure OPenNI is the same
		virtual void onNewFrame(UserTracker&) {}

	private:
		NiteUserTrackerCallbacks m_userTrackerCallbacks;

		NiteUserTrackerCallbacks& getCallbacks() {return m_userTrackerCallbacks;}

		static void ONI_CALLBACK_TYPE newFrameCallback(void* pCookie)
		{
			Listener* pListener = (Listener*)pCookie;
			pListener->onNewFrame(*pListener->m_pUserTracker);
		}


		friend class UserTracker;
		void setUserTracker(UserTracker* pUserTracker)
		{
			m_pUserTracker = pUserTracker;
		}

		UserTracker* m_pUserTracker;
	};

	UserTracker() : m_userTrackerHandle(NULL)
	{}


	~UserTracker()
	{
		destroy();
	}

	Status create(openni::Device* pDevice = NULL)
	{
		if (pDevice == NULL)
		{
			return (Status)niteInitializeUserTracker(&m_userTrackerHandle);
		}
		return (Status)niteInitializeUserTrackerByDevice(pDevice, &m_userTrackerHandle);
	}

	void destroy()
	{
		if (isValid())
		{
			niteShutdownUserTracker(m_userTrackerHandle);
			m_userTrackerHandle = NULL;
		}
	}

	/** Get the next snapshot of the algorithm */
	Status readFrame(UserTrackerFrameRef* pFrame)
	{
		NiteUserTrackerFrame *pNiteFrame = NULL;
		Status rc = (Status)niteReadUserTrackerFrame(m_userTrackerHandle, &pNiteFrame);
		pFrame->setReference(m_userTrackerHandle, pNiteFrame);

		return rc;
	}

	bool isValid() const
	{
		return m_userTrackerHandle != NULL;
	}

	/** Control the smoothing factor of the skeleton joints */
	Status setSkeletonSmoothingFactor(float factor)
	{
		return (Status)niteSetSkeletonSmoothing(m_userTrackerHandle, factor);
	}
	float getSkeletonSmoothingFactor() const
	{
		float factor;
		Status rc = (Status)niteGetSkeletonSmoothing(m_userTrackerHandle, &factor);
		if (rc != STATUS_OK)
		{
			factor = 0;
		}
		return factor;
	}

	/** Request a skeleton for a specific user */
	Status startSkeletonTracking(UserId id)
	{
		return (Status)niteStartSkeletonTracking(m_userTrackerHandle, id);
	}
	/** Inform the algorithm that a skeleton is no longer required for a specific user */
	void stopSkeletonTracking(UserId id)
	{
		niteStopSkeletonTracking(m_userTrackerHandle, id);
	}

	/** Start detecting a specific gesture */
	Status startPoseDetection(UserId user, PoseType type)
	{
		return (Status)niteStartPoseDetection(m_userTrackerHandle, (NiteUserId)user, (NitePoseType)type);
	}
	/** Stop detecting a specific gesture */
	void stopPoseDetection(UserId user, PoseType type)
	{
		niteStopPoseDetection(m_userTrackerHandle, (NiteUserId)user, (NitePoseType)type);
	}
	void stopPoseDetection(UserId user)
	{
		niteStopAllPoseDetection(m_userTrackerHandle, (NiteUserId)user);
	}


	void addListener(Listener* pListener)
	{
		niteRegisterUserTrackerCallbacks(m_userTrackerHandle, &pListener->getCallbacks(), pListener);
		pListener->setUserTracker(this);
	}
	void removeListener(Listener* pListener)
	{
		niteUnregisterUserTrackerCallbacks(m_userTrackerHandle, &pListener->getCallbacks());
		pListener->setUserTracker(NULL);
	}

	/**
	Skeleton joint position is provided in a different set of coordinates than the depth coordinates.
	While the depth coordinates are projective, the joint is provided in real world coordinates, i.e. number of millimeters from the sensor.
	This function enables conversion from the joint coordinates to the depth coordinates. This is useful, for instance, to match the joint on the depth.
	*/
	Status convertJointCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY)
	{
		return (Status)niteConvertJointCoordinatesToDepth(m_userTrackerHandle, x, y, z, pOutX, pOutY);
	}
	/**
	Skeleton joint position is provided in a different set of coordinates than the depth coordinates.
	While the depth coordinates are projective, the joint is provided in real world coordinates, i.e. number of millimeters from the sensor.
	This function enables conversion from the depth coordinates to the joint coordinates. This is useful, for instance, to allow measurements.
	*/
	Status convertDepthCoordinatesToJoint(int x, int y, int z, float* pOutX, float* pOutY)
	{
		return (Status)niteConvertDepthCoordinatesToJoint(m_userTrackerHandle, x, y, z, pOutX, pOutY);
	}

private:
	NiteUserTrackerHandle m_userTrackerHandle;
};


// HandTracker
typedef short int HandId;

class GestureData : protected NiteGestureData
{
public:
	GestureType getType() const {return (GestureType)type;}
	const Point3f& getCurrentPosition() const {return (Point3f&)currentPosition;}

	bool isComplete() const {return (state & NITE_GESTURE_STATE_COMPLETED) != 0;}
	bool isInProgress() const {return (state & NITE_GESTURE_STATE_IN_PROGRESS) != 0;}
};

class HandData : protected NiteHandData
{
public:
	HandId getId() const {return id;}
	const Point3f& getPosition() const {return (Point3f&)position;}

	bool isNew() const {return (state & NITE_HAND_STATE_NEW) != 0;}
	bool isLost() const {return state == NITE_HAND_STATE_LOST;}
	bool isTracking() const {return (state & NITE_HAND_STATE_TRACKED) != 0;}
	bool isTouchingFov() const {return (state & NITE_HAND_STATE_TOUCHING_FOV) != 0;}
};

/** Snapshot of the Hand Tracker algorithm. It holds all the hands identified at this time, as well as the detected gestures */
class HandTrackerFrameRef
{
public:
	HandTrackerFrameRef() : m_pFrame(NULL), m_handTracker(NULL)
	{}
	~HandTrackerFrameRef()
	{
		release();
	}

	HandTrackerFrameRef(const HandTrackerFrameRef& other) : m_pFrame(NULL)
	{
		*this = other;
	}
	HandTrackerFrameRef& operator=(const HandTrackerFrameRef& other)
	{
		setReference(other.m_handTracker, other.m_pFrame);
		niteHandTrackerFrameAddRef(m_handTracker, m_pFrame);

		return *this;
	}

	void release()
	{
		if (m_pFrame != NULL)
		{
			niteHandTrackerFrameRelease(m_handTracker, m_pFrame);
		}
		m_pFrame = NULL;
		m_handTracker = NULL;
	}

	const Array<HandData>& getHands() const {return m_hands;}
	const Array<GestureData>& getGestures() const {return m_gestures;}

	openni::VideoFrameRef getDepthFrame() const
	{
		return m_depthFrame;
	}

	uint64_t getTimestamp() const {return m_pFrame->timestamp;}
	int getFrameIndex() const {return m_pFrame->frameIndex;}
private:
	friend class HandTracker;

	void setReference(NiteHandTrackerHandle handTracker, NiteHandTrackerFrame* pFrame)
	{
		release();
		m_handTracker = handTracker;
		m_pFrame = pFrame;
		m_depthFrame._setFrame(pFrame->pDepthFrame);

		m_hands.setData(m_pFrame->handCount, (HandData*)m_pFrame->pHands);
		m_gestures.setData(m_pFrame->gestureCount, (GestureData*)m_pFrame->pGestures);
	}

	NiteHandTrackerFrame* m_pFrame;
	NiteHandTrackerHandle m_handTracker;
	openni::VideoFrameRef m_depthFrame;

	Array<HandData> m_hands;
	Array<GestureData> m_gestures;
};

/**
This is the main object of the Hand Tracker algorithm.
Through it all the hands and gestures are accessible.
*/
class HandTracker
{
public:
	class Listener
	{
	public:
		Listener() : m_pHandTracker(NULL)
		{
			m_handTrackerCallbacks.readyForNextFrame = newFrameCallback;
		}
		virtual void onNewFrame(HandTracker&) {}
	private:
		friend class HandTracker;
		NiteHandTrackerCallbacks m_handTrackerCallbacks;
		
		NiteHandTrackerCallbacks& getCallbacks() {return m_handTrackerCallbacks;}

		static void ONI_CALLBACK_TYPE newFrameCallback(void* pCookie)
		{
			Listener* pListener = (Listener*)pCookie;
			pListener->onNewFrame(*pListener->m_pHandTracker);
		}

		void setHandTracker(HandTracker* pHandTracker)
		{
			m_pHandTracker = pHandTracker;
		}
		HandTracker* m_pHandTracker;
	};

	HandTracker() : m_handTrackerHandle(NULL)
	{}
	~HandTracker()
	{
		destroy();
	}

	Status create(openni::Device* pDevice = NULL)
	{
		if (pDevice == NULL)
		{
			return (Status)niteInitializeHandTracker(&m_handTrackerHandle);
			// Pick a device
		}
		return (Status)niteInitializeHandTrackerByDevice(pDevice, &m_handTrackerHandle);
	}

	void destroy()
	{
		if (isValid())
		{
			niteShutdownHandTracker(m_handTrackerHandle);
			m_handTrackerHandle = NULL;
		}
	}

	/** Get the next snapshot of the algorithm */
	Status readFrame(HandTrackerFrameRef* pFrame)
	{
		NiteHandTrackerFrame *pNiteFrame = NULL;
		Status rc = (Status)niteReadHandTrackerFrame(m_handTrackerHandle, &pNiteFrame);
		pFrame->setReference(m_handTrackerHandle, pNiteFrame);

		return rc;
	}

	bool isValid() const
	{
		return m_handTrackerHandle != NULL;
	}

	/** Control the smoothing factor of the skeleton joints */
	Status setSmoothingFactor(float factor)
	{
		return (Status)niteSetHandSmoothingFactor(m_handTrackerHandle, factor);
	}
	float getSmoothingFactor() const
	{
		float factor;
		Status rc = (Status)niteGetHandSmoothingFactor(m_handTrackerHandle, &factor);
		if (rc != STATUS_OK)
		{
			factor = 0;
		}
		return factor;
	}

	/**
	Request a hand in a specific position, assuming there really is a hand there.
	For instance, the position received from a gesture can be used.
	*/
	Status startHandTracking(const Point3f& position, HandId* pNewHandId)
	{
		return (Status)niteStartHandTracking(m_handTrackerHandle, (const NitePoint3f*)&position, pNewHandId);
	}
	/** Inform the algorithm that a specific hand is no longer required */
	void stopHandTracking(HandId id)
	{
		niteStopHandTracking(m_handTrackerHandle, id);
	}

	void addListener(Listener* pListener)
	{
		niteRegisterHandTrackerCallbacks(m_handTrackerHandle, &pListener->getCallbacks(), pListener);
		pListener->setHandTracker(this);
	}
	void removeListener(Listener* pListener)
	{
		niteUnregisterHandTrackerCallbacks(m_handTrackerHandle, &pListener->getCallbacks());
		pListener->setHandTracker(NULL);
	}

	/** Start detecting a specific gesture */
	Status startGestureDetection(GestureType type)
	{
		return (Status)niteStartGestureDetection(m_handTrackerHandle, (NiteGestureType)type);
	}
	/** Stop detecting a specific gesture */
	void stopGestureDetection(GestureType type)
	{
		niteStopGestureDetection(m_handTrackerHandle, (NiteGestureType)type);
	}

	/**
	Hand position is provided in a different set of coordinates than the depth coordinates.
	While the depth coordinates are projective, the hand and gestures are provided in real world coordinates, i.e. number of millimeters from the sensor.
	This function enables conversion from the hand coordinates to the depth coordinates. This is useful, for instance, to match the hand to the depth.
	*/
	Status convertHandCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY)
	{
		return (Status)niteConvertHandCoordinatesToDepth(m_handTrackerHandle, x, y, z, pOutX, pOutY);
	}
	/**
	Hand position is provided in a different set of coordinates than the depth coordinates.
	While the depth coordinates are projective, the hand and gestures are provided in real world coordinates, i.e. number of millimeters from the sensor.
	This function enables conversion from the depth coordinates to the hand coordinates. This is useful, for instance, to allow measurements.
	*/
	Status convertDepthCoordinatesToHand(int x, int y, int z, float* pOutX, float* pOutY)
	{
		return (Status)niteConvertDepthCoordinatesToHand(m_handTrackerHandle, x, y, z, pOutX, pOutY);
	}

private:
	NiteHandTrackerHandle m_handTrackerHandle;
};

/**
The NiTE class is a static entry point to the library.
Through it you can initialize the library, as well as create User Trackers and Hand Trackers.
*/
class NiTE
{
public:
	static Status initialize()
	{
		return (Status)niteInitialize();
	}
	static void shutdown()
	{
		niteShutdown();
	}

	static Version getVersion()
	{
		NiteVersion version = niteGetVersion();
		union
		{
			NiteVersion* pC;
			Version* pCpp;
		} a;
		a.pC = &version;
		return *a.pCpp;
	}
private:
	NiTE();
};

} // namespace nite

#endif // _NITE_H_
