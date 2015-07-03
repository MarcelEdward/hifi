//  3DConnexion.h
//  hifi
//
//  Created by Marcel Verhagen on 09-06-15.
//
//

#ifndef hifi_ConnexionClient_h
#define hifi_ConnexionClient_h

#include <qobject.h>
#include <qlibrary.h>
#include "InterfaceLogging.h"
#include "Application.h"

#include "ui/UserInputMapper.h"

// connnects to the userinputmapper
class ConnexionData : public QObject{
	Q_OBJECT

public:
	static ConnexionData& getInstance();
	ConnexionData();

	enum ConnexionAxisChannel {
		POSITION_AXIS_X_POS = 1,
		POSITION_AXIS_X_NEG = 2,
		POSITION_AXIS_Y_POS = 3,
		POSITION_AXIS_Y_NEG = 4,
		POSITION_AXIS_Z_POS = 5,
		POSITION_AXIS_Z_NEG = 6,
		ROTATION_AXIS_X_POS = 7,
		ROTATION_AXIS_X_NEG = 8,
		ROTATION_AXIS_Y_POS = 9,
		ROTATION_AXIS_Y_NEG = 10,
		ROTATION_AXIS_Z_POS = 11,
		ROTATION_AXIS_Z_NEG = 12,
	};

	enum ButtonChannel {
		BUTTON_1 = 1,
		BUTTON_2 = 2,
		BUTTON_3 = 3,
	};

	typedef std::unordered_set<int> ButtonPressedMap;
	typedef std::map<int, float> AxisStateMap;

	float getButton(int channel) const;
	float getAxis(int channel) const;

	UserInputMapper::Input makeInput(ConnexionData::ConnexionAxisChannel axis);
	UserInputMapper::Input makeInput(ConnexionData::ButtonChannel button);

	void registerToUserInputMapper(UserInputMapper& mapper);
	void assignDefaultInputMapping(UserInputMapper& mapper);

	void update(float deltaTime);
	void focusOutEvent();

	int getDeviceID() { return _deviceID; }

	void closeJoystick();

	QString _name;

	glm::vec3 cc_position;
	glm::vec3 cc_rotation;
	int clientId;
	int buttonState;

	void setButton(int lastButtonState);
	void handleAxisEvent();

protected:
	int _deviceID = 0;

	ButtonPressedMap _buttonPressedMap;
	AxisStateMap _axisStateMap;
};

// the windows connexion rawinput
#ifdef _WIN32
#ifdef HAVE_CONNEXIONCLIENT
#include "I3dMouseParams.h"
#include <QAbstractNativeEventFilter>
#include <QAbstractEventDispatcher>
#endif /* HAVE_CONNEXIONCLIENT */ 
#include <Winsock2.h>
#include <windows.h>

#ifdef HAVE_CONNEXIONCLIENT
// windows rawinput parameters
class MouseParameters : public I3dMouseParam
{
public:
	MouseParameters();
	~MouseParameters();

	// I3dmouseSensor interface
	bool		IsPanZoom()  const;
	bool		IsRotate()  const;
	ESpeed		GetSpeed()  const;

	void		SetPanZoom(bool isPanZoom);
	void		SetRotate(bool isRotate);
	void		SetSpeed(ESpeed speed);

	// I3dmouseNavigation interface
	ENavigation			GetNavigationMode()  const;
	EPivot				GetPivotMode()  const;
	EPivotVisibility	GetPivotVisibility()  const;
	bool				IsLockHorizon()  const;

	void				SetLockHorizon(bool bOn);
	void				SetNavigationMode(ENavigation navigation);
	void				SetPivotMode(EPivot pivot);
	void				SetPivotVisibility(EPivotVisibility visibility);

private:

	MouseParameters(const MouseParameters&);
	const MouseParameters& operator =(const MouseParameters&);

	ENavigation			fNavigation;
	EPivot				fPivot;
	EPivotVisibility	fPivotVisibility;
	bool				fIsLockHorizon;

	bool				fIsPanZoom;
	bool				fIsRotate;
	ESpeed				fSpeed;
};
#endif /* HAVE_CONNEXIONCLIENT */

class ConnexionClient : public  QObject, public QAbstractNativeEventFilter {
	Q_OBJECT
public:
	ConnexionClient();
	~ConnexionClient();

	static void init();
	static void destroy();

	static	bool		Is3dmouseAttached();

#ifdef HAVE_CONNEXIONCLIENT

	I3dMouseParam&			MouseParams();
	const I3dMouseParam&	MouseParams() const;

	virtual	void		Move3d(HANDLE device, std::vector<float>& motionData);
	virtual void		On3dmouseKeyDown(HANDLE device, int virtualKeyCode);
	virtual void		On3dmouseKeyUp(HANDLE device, int virtualKeyCode);

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE
	{
		MSG *msg = static_cast< MSG * >(message);
		return ConnexionClient::RawInputEventFilter(message,  result);
	}

signals:
	void		Move3d(std::vector<float>& motionData);
	void		On3dmouseKeyDown(int virtualKeyCode);
	void		On3dmouseKeyUp(int virtualKeyCode);

private:
	bool		InitializeRawInput(HWND hwndTarget);

	static	bool		RawInputEventFilter(void* msg, long* result);

	void		OnRawInput(UINT nInputCode, HRAWINPUT hRawInput);
	UINT		GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
	bool		TranslateRawInputData(UINT nInputCode, PRAWINPUT pRawInput);
	void		On3dmouseInput();

	class TInputData
	{
	public:
		TInputData() : fAxes(6) {}

		bool IsZero() {
			return (0. == fAxes[0] && 0. == fAxes[1] && 0. == fAxes[2] &&
				0. == fAxes[3] && 0. == fAxes[4] && 0. == fAxes[5]);
		}

		int                   fTimeToLive; // For telling if the device was unplugged while sending data
		bool                  fIsDirty;
		std::vector<float>	 fAxes;

	};

	HWND								fWindow;

	// Data cache to handle multiple rawinput devices
	std::map< HANDLE, TInputData>       fDevice2Data;
	std::map< HANDLE, unsigned long>    fDevice2Keystate;

	// 3dmouse parameters
	MouseParameters						f3dMouseParams;     // Rotate, Pan Zoom etc.

	// use to calculate distance traveled since last event
	DWORD								fLast3dmouseInputTime;

#endif /* HAVE_CONNEXIONCLIENT */    
};

// the osx connexion api
#else

#include <glm/glm.hpp>
#include "3DconnexionClient/ConnexionClientAPI.h"

class ConnexionClient : public QObject {
	Q_OBJECT
public:
	static ConnexionClient& getInstance();
	static ConnexionData connexionData;
	static void init();
	static void destroy();
};

#endif /* __APPLE__ */

#endif /* defined(hifi_ConnexionClient_h) */