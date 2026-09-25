#pragma once

// Wrapper de compatibilidad - redirige al nuevo sistema CTouchInput
#include "TouchInput.h"

// Enum de compatibilidad con el código existente en Pad.cpp
enum BtnType {
	DEFAULT,
	STICK,
	JOY,
	CAR,      // Entrar/Salir vehículo -> TOUCH_TRIANGLE
	JUMP,     // Saltar -> TOUCH_CROSS
	ATTACK,   // Atacar -> TOUCH_CIRCLE
	SPRINT,   // Correr -> TOUCH_SQUARE
	LOOK
};

// Mapeo de BtnType a TouchButtonID
inline TouchButtonID MapBtnTypeToTouchID(BtnType type) {
	switch (type) {
		case CAR:    return TOUCH_TRIANGLE;
		case JUMP:   return TOUCH_CROSS;
		case ATTACK: return TOUCH_CIRCLE;
		case SPRINT: return TOUCH_SQUARE;
		default:     return TOUCH_NONE;
	}
}

// Clase de compatibilidad con el código existente
class CTouch {
public:
	// Variables de compatibilidad con Pad.cpp (accesibles directamente)
	int16 moveAxisX;
	int16 moveAxisY;
	int16 lookAxisX;
	int16 lookAxisY;
	
	CTouch() : moveAxisX(0), moveAxisY(0), lookAxisX(0), lookAxisY(0) {}
	
	void Init() { 
		CTouchInput::Init(); 
		moveAxisX = moveAxisY = 0;
		lookAxisX = lookAxisY = 0;
	}
	
	void Update() {
		CTouchInput::Update();
		
		moveAxisX = CTouchInput::GetLeftStickX();
		moveAxisY = CTouchInput::GetLeftStickY();
		lookAxisX = CTouchInput::GetRightStickX();
		lookAxisY = CTouchInput::GetRightStickY();
	}

	void Draw() { 
		CTouchInput::Draw();
	}
	
	bool getButtonJustDown(BtnType type) {
		TouchButtonID id = MapBtnTypeToTouchID(type);
		if (id == TOUCH_NONE) return false;
		return CTouchInput::IsButtonJustPressed(id);
	}
	
	bool getButton(BtnType type) {
		TouchButtonID id = MapBtnTypeToTouchID(type);
		if (id == TOUCH_NONE) return false;
		return CTouchInput::IsButtonPressed(id);
	}
};

extern CTouch gTouch;
