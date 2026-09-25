#pragma once
#include "common.h"
#include "Sprite2d.h"
#include "TxdStore.h"

// Constantes de diseño del joystick
#define JOYSTICK_BASE_SIZE    0.18f   // Tamaño del área base del joystick (normalizado)
#define JOYSTICK_STICK_SIZE   0.08f   // Tamaño del stick interno
#define JOYSTICK_DEAD_ZONE    0.15f   // Zona muerta del joystick
#define JOYSTICK_MAX_RADIUS   0.08f   // Radio máximo de movimiento del stick

#define BUTTON_SIZE           0.09f   // Tamaño de botones de acción
#define BUTTON_SPACING        0.10f   // Espaciado entre botones

// IDs de botones
enum TouchButtonID {
	TOUCH_NONE = -1,
	TOUCH_CROSS = 0,      // X - Saltar/Entrar vehículo
	TOUCH_CIRCLE,         // O - Atacar
	TOUCH_SQUARE,         // □ - Sprint
	TOUCH_TRIANGLE,       // △ - Entrar/Salir vehículo
	TOUCH_L1,             // L1 - Arma anterior
	TOUCH_R1,             // R1 - Arma siguiente/Disparar
	TOUCH_L2,             // L2 - Mirar atrás
	TOUCH_R2,             // R2 - Acelerar (vehículo)
	TOUCH_DPAD_UP,
	TOUCH_DPAD_DOWN,
	TOUCH_DPAD_LEFT,
	TOUCH_DPAD_RIGHT,
	TOUCH_START,
	TOUCH_SELECT,
	NUM_TOUCH_BUTTONS
};

// Estado de un botón táctil
struct TouchButton {
	float x, y;           // Posición normalizada (0-1)
	float size;           // Tamaño normalizado
	bool pressed;         // Estado actual
	bool justPressed;     // Se acaba de presionar
	bool justReleased;    // Se acaba de soltar
	int fingerID;         // ID del dedo que lo toca
	CSprite2d* sprite;    // Sprite del botón
	CRGBA normalColor;    // Color normal
	CRGBA pressedColor;   // Color cuando está presionado
	
	TouchButton() : x(0), y(0), size(0), pressed(false), 
		justPressed(false), justReleased(false), fingerID(-1), sprite(nil),
		normalColor(255, 255, 255, 180), pressedColor(255, 255, 255, 255) {}
	
	void SetPosition(float _x, float _y, float _size = BUTTON_SIZE) {
		x = _x; y = _y; size = _size;
	}
	
	CRect GetScreenRect() const;
	bool ContainsPoint(float px, float py) const;
};

// Joystick virtual
struct VirtualJoystick {
	float baseX, baseY;       // Posición base (centro)
	float stickX, stickY;     // Posición actual del stick
	float baseSize;           // Tamaño del área base
	float stickSize;          // Tamaño del stick
	bool active;              // Está siendo usado
	int fingerID;             // Dedo que lo controla
	float outputX, outputY;   // Salida normalizada (-1 a 1)
	CSprite2d* baseSprite;    // Sprite de la base
	CSprite2d* stickSprite;   // Sprite del stick
	
	VirtualJoystick() : baseX(0), baseY(0), stickX(0), stickY(0),
		baseSize(JOYSTICK_BASE_SIZE), stickSize(JOYSTICK_STICK_SIZE),
		active(false), fingerID(-1), outputX(0), outputY(0),
		baseSprite(nil), stickSprite(nil) {}
	
	void SetPosition(float x, float y) {
		baseX = x; baseY = y;
		stickX = x; stickY = y;
	}
	
	void Reset() {
		stickX = baseX;
		stickY = baseY;
		active = false;
		fingerID = -1;
		outputX = outputY = 0;
	}
	
	CRect GetBaseRect() const;
	CRect GetStickRect() const;
	bool ContainsPoint(float px, float py) const;
};

class CTouchInput {
public:
	static void Init();
	static void Shutdown();
	static void Update();
	static void Draw();
	
	// Obtener estado de botones
	static bool IsButtonPressed(TouchButtonID id);
	static bool IsButtonJustPressed(TouchButtonID id);
	static bool IsButtonJustReleased(TouchButtonID id);
	
	// Obtener ejes del joystick de movimiento (-128 a 128)
	static int16 GetLeftStickX();
	static int16 GetLeftStickY();
	
	// Obtener ejes del joystick de cámara (-128 a 128)  
	static int16 GetRightStickX();
	static int16 GetRightStickY();
	
	// Modo de control (a pie / vehículo)
	static void SetVehicleMode(bool inVehicle);
	static bool IsVehicleMode() { return m_bVehicleMode; }
	
private:
	static void LoadSprites();
	static void UnloadSprites();
	static void UpdateButtons();
	static void UpdateJoysticks();
	static void DrawButtons();
	static void DrawJoysticks();
	
	static float ScreenToNormX(float screenX);
	static float ScreenToNormY(float screenY);
	static float NormToScreenX(float normX);
	static float NormToScreenY(float normY);
	
	static bool m_bInitialized;
	static bool m_bVehicleMode;
	static int m_nTxdSlot;
	
	// Joysticks
	static VirtualJoystick m_LeftJoystick;   // Movimiento
	static VirtualJoystick m_RightJoystick;  // Cámara (zona de look)
	
	// Botones
	static TouchButton m_Buttons[NUM_TOUCH_BUTTONS];
	
	// Sprites
	static CSprite2d m_SpriteJoyBase;
	static CSprite2d m_SpriteJoyStick;
	static CSprite2d m_SpriteCross;
	static CSprite2d m_SpriteCircle;
	static CSprite2d m_SpriteSquare;
	static CSprite2d m_SpriteTriangle;
	static CSprite2d m_SpriteL1;
	static CSprite2d m_SpriteR1;
	static CSprite2d m_SpriteL2;
	static CSprite2d m_SpriteR2;
	static CSprite2d m_SpriteDpadUp;
	static CSprite2d m_SpriteDpadDown;
	static CSprite2d m_SpriteDpadLeft;
	static CSprite2d m_SpriteDpadRight;
	
	// Para el joystick derecho (cámara) usamos zona de touch
	static float m_fLookSensitivity;
	static int m_nLookFingerID;
	static float m_fLastLookX;
	static float m_fLastLookY;
	static float m_fLookDeltaX;
	static float m_fLookDeltaY;
};
