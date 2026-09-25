#include "common.h"
#include "TouchInput.h"
#include "Sprite2d.h"
#include "TxdStore.h"
#include "Timer.h"
#include "Camera.h"
#include "Pad.h"
#include <cmath>

// TouchInfo está definido en sdl3.cpp
#ifndef LIBRW_SDL3
struct TouchInfo {
	float x, y;
	float dx, dy;
	bool pressed;
};
#endif
extern TouchInfo touchInfo[10];

#ifdef __ANDROID__
#include <android/log.h>
#endif

// Forward declaration de helper
static void DrawCircle(float cx, float cy, float radius, CRGBA color, int segments);

// Variables estáticas
bool CTouchInput::m_bInitialized = false;
bool CTouchInput::m_bVehicleMode = false;
int CTouchInput::m_nTxdSlot = -1;

VirtualJoystick CTouchInput::m_LeftJoystick;
VirtualJoystick CTouchInput::m_RightJoystick;
TouchButton CTouchInput::m_Buttons[NUM_TOUCH_BUTTONS];

CSprite2d CTouchInput::m_SpriteJoyBase;
CSprite2d CTouchInput::m_SpriteJoyStick;
CSprite2d CTouchInput::m_SpriteCross;
CSprite2d CTouchInput::m_SpriteCircle;
CSprite2d CTouchInput::m_SpriteSquare;
CSprite2d CTouchInput::m_SpriteTriangle;
CSprite2d CTouchInput::m_SpriteL1;
CSprite2d CTouchInput::m_SpriteR1;
CSprite2d CTouchInput::m_SpriteL2;
CSprite2d CTouchInput::m_SpriteR2;
CSprite2d CTouchInput::m_SpriteDpadUp;
CSprite2d CTouchInput::m_SpriteDpadDown;
CSprite2d CTouchInput::m_SpriteDpadLeft;
CSprite2d CTouchInput::m_SpriteDpadRight;

float CTouchInput::m_fLookSensitivity = 12.0f;
int CTouchInput::m_nLookFingerID = -1;
float CTouchInput::m_fLastLookX = 0.0f;
float CTouchInput::m_fLastLookY = 0.0f;
float CTouchInput::m_fLookDeltaX = 0.0f;
float CTouchInput::m_fLookDeltaY = 0.0f;

// Implementación de TouchButton
CRect TouchButton::GetScreenRect() const {
	float halfSize = size * 0.5f;
	float screenX = x * SCREEN_WIDTH;
	float screenY = y * SCREEN_HEIGHT;
	float sizePixels = size * SCREEN_HEIGHT; // Usar altura para mantener aspecto
	
	return CRect(
		screenX - sizePixels * 0.5f,
		screenY - sizePixels * 0.5f,
		screenX + sizePixels * 0.5f,
		screenY + sizePixels * 0.5f
	);
}

bool TouchButton::ContainsPoint(float px, float py) const {
	// Botones no configurados nunca contienen puntos
	if (size <= 0.0f)
		return false;
	
	float screenX = x * SCREEN_WIDTH;
	float screenY = y * SCREEN_HEIGHT;
	float sizePixels = size * SCREEN_HEIGHT * 0.7f; // Área táctil generosa
	
	float dx = px - screenX;
	float dy = py - screenY;
	float dist = sqrtf(dx * dx + dy * dy);
	
	return dist <= sizePixels;
}

// Implementación de VirtualJoystick
CRect VirtualJoystick::GetBaseRect() const {
	float screenX = baseX * SCREEN_WIDTH;
	float screenY = baseY * SCREEN_HEIGHT;
	float sizePixels = baseSize * SCREEN_HEIGHT;
	
	return CRect(
		screenX - sizePixels * 0.5f,
		screenY - sizePixels * 0.5f,
		screenX + sizePixels * 0.5f,
		screenY + sizePixels * 0.5f
	);
}

CRect VirtualJoystick::GetStickRect() const {
	float screenX = stickX * SCREEN_WIDTH;
	float screenY = stickY * SCREEN_HEIGHT;
	float sizePixels = stickSize * SCREEN_HEIGHT;
	
	return CRect(
		screenX - sizePixels * 0.5f,
		screenY - sizePixels * 0.5f,
		screenX + sizePixels * 0.5f,
		screenY + sizePixels * 0.5f
	);
}

bool VirtualJoystick::ContainsPoint(float px, float py) const {
	float screenX = baseX * SCREEN_WIDTH;
	float screenY = baseY * SCREEN_HEIGHT;
	float radius = baseSize * SCREEN_HEIGHT * 0.5f;
	
	float dx = px - screenX;
	float dy = py - screenY;
	float dist = sqrtf(dx * dx + dy * dy);
	
	return dist <= radius;
}

// Implementación de CTouchInput
void CTouchInput::Init() {
	if (m_bInitialized)
		return;
	
	LoadSprites();
	
	// Configurar joystick izquierdo (movimiento)
	m_LeftJoystick.SetPosition(0.12f, 0.70f);
	m_LeftJoystick.baseSprite = &m_SpriteJoyBase;
	m_LeftJoystick.stickSprite = &m_SpriteJoyStick;
	
	// Configurar botones de acción (lado derecho, layout diamante PlayStation)
	// Zona segura: evitar X < 0.08 y X > 0.92, Y < 0.08 y Y > 0.88
	const float btnCenterX = 0.78f;  // Más adentro del borde derecho
	const float btnCenterY = 0.72f;  // Centro vertical de los botones
	const float btnOffset = 0.09f;   // Distancia del centro al botón
	
	// Cruz (X) - Abajo - SALTAR
	m_Buttons[TOUCH_CROSS].SetPosition(btnCenterX, btnCenterY + btnOffset, BUTTON_SIZE);
	m_Buttons[TOUCH_CROSS].sprite = &m_SpriteCross;
	m_Buttons[TOUCH_CROSS].normalColor = CRGBA(100, 180, 255, 200);
	m_Buttons[TOUCH_CROSS].pressedColor = CRGBA(100, 180, 255, 255);
	
	// Círculo (O) - Derecha - ATACAR
	m_Buttons[TOUCH_CIRCLE].SetPosition(btnCenterX + btnOffset, btnCenterY, BUTTON_SIZE);
	m_Buttons[TOUCH_CIRCLE].sprite = &m_SpriteCircle;
	m_Buttons[TOUCH_CIRCLE].normalColor = CRGBA(255, 100, 100, 200);
	m_Buttons[TOUCH_CIRCLE].pressedColor = CRGBA(255, 100, 100, 255);
	
	// Cuadrado (□) - Izquierda - SPRINT
	m_Buttons[TOUCH_SQUARE].SetPosition(btnCenterX - btnOffset, btnCenterY, BUTTON_SIZE);
	m_Buttons[TOUCH_SQUARE].sprite = &m_SpriteSquare;
	m_Buttons[TOUCH_SQUARE].normalColor = CRGBA(255, 100, 255, 200);
	m_Buttons[TOUCH_SQUARE].pressedColor = CRGBA(255, 100, 255, 255);
	
	// Triángulo (△) - Arriba - ENTRAR/SALIR VEHÍCULO
	m_Buttons[TOUCH_TRIANGLE].SetPosition(btnCenterX, btnCenterY - btnOffset, BUTTON_SIZE);
	m_Buttons[TOUCH_TRIANGLE].sprite = &m_SpriteTriangle;
	m_Buttons[TOUCH_TRIANGLE].normalColor = CRGBA(100, 255, 100, 200);
	m_Buttons[TOUCH_TRIANGLE].pressedColor = CRGBA(100, 255, 100, 255);
	
	// L1/R1 (hombros) - También fuera de zona segura
	m_Buttons[TOUCH_L1].SetPosition(0.12f, 0.18f, 0.055f);
	m_Buttons[TOUCH_L1].sprite = &m_SpriteL1;
	m_Buttons[TOUCH_L1].normalColor = CRGBA(200, 200, 200, 150);
	
	m_Buttons[TOUCH_R1].SetPosition(0.88f, 0.18f, 0.055f);
	m_Buttons[TOUCH_R1].sprite = &m_SpriteR1;
	m_Buttons[TOUCH_R1].normalColor = CRGBA(200, 200, 200, 150);
	
	m_bInitialized = true;
}

void CTouchInput::Shutdown() {
	if (!m_bInitialized)
		return;
	
	UnloadSprites();
	m_bInitialized = false;
}

void CTouchInput::LoadSprites() {
	// Cargar TXD de botones PS3
	m_nTxdSlot = CTxdStore::FindTxdSlot("ps3btns");
	if (m_nTxdSlot == -1) {
		m_nTxdSlot = CTxdStore::AddTxdSlot("ps3btns");
	}
	
	CTxdStore::LoadTxd(m_nTxdSlot, "MODELS/PS3BTNS.TXD");
	CTxdStore::AddRef(m_nTxdSlot);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(m_nTxdSlot);
	
	// Cargar sprites
	m_SpriteCross.SetTexture("cross");
	m_SpriteCircle.SetTexture("circle");
	m_SpriteSquare.SetTexture("square");
	m_SpriteTriangle.SetTexture("triangle");
	m_SpriteL1.SetTexture("l1");
	m_SpriteR1.SetTexture("r1");
	m_SpriteL2.SetTexture("l2");
	m_SpriteR2.SetTexture("r2");
	m_SpriteDpadUp.SetTexture("dud");
	m_SpriteDpadDown.SetTexture("dud");
	m_SpriteDpadLeft.SetTexture("dlr");
	m_SpriteDpadRight.SetTexture("dlr");
	
	// Para el joystick, usar texturas de thumbstick
	m_SpriteJoyBase.SetTexture("thumbl");
	m_SpriteJoyStick.SetTexture("thumblx");
	
	CTxdStore::PopCurrentTxd();
}

void CTouchInput::UnloadSprites() {
	m_SpriteCross.Delete();
	m_SpriteCircle.Delete();
	m_SpriteSquare.Delete();
	m_SpriteTriangle.Delete();
	m_SpriteL1.Delete();
	m_SpriteR1.Delete();
	m_SpriteL2.Delete();
	m_SpriteR2.Delete();
	m_SpriteDpadUp.Delete();
	m_SpriteDpadDown.Delete();
	m_SpriteDpadLeft.Delete();
	m_SpriteDpadRight.Delete();
	m_SpriteJoyBase.Delete();
	m_SpriteJoyStick.Delete();
	
	if (m_nTxdSlot != -1) {
		CTxdStore::RemoveTxdSlot(m_nTxdSlot);
		m_nTxdSlot = -1;
	}
}

void CTouchInput::Update() {
	if (!m_bInitialized)
		return;
	
	// IMPORTANTE: Botones primero para que tengan prioridad
	UpdateButtons();
	UpdateJoysticks();
}

void CTouchInput::UpdateJoysticks() {
	// Actualizar joystick izquierdo (movimiento)
	VirtualJoystick& leftJoy = m_LeftJoystick;
	
	bool wasActive = leftJoy.active;
	leftJoy.active = false;
	
	// Buscar un dedo en el área del joystick izquierdo (mitad izquierda de pantalla)
	for (int i = 0; i < 10; i++) {
		if (!touchInfo[i].pressed)
			continue;
		
		float normX = touchInfo[i].x / SCREEN_WIDTH;
		float normY = touchInfo[i].y / SCREEN_HEIGHT;
		bool sameFinger = leftJoy.fingerID == i;
		bool inStartArea = normX <= 0.45f && normY >= 0.45f;
		
		// Mantener el dedo capturado aunque salga del círculo visible.
		if (sameFinger || leftJoy.ContainsPoint(touchInfo[i].x, touchInfo[i].y) || inStartArea) {
			leftJoy.active = true;
			leftJoy.fingerID = i;
			
			// Calcular nueva posición del stick
			float baseScreenX = leftJoy.baseX * SCREEN_WIDTH;
			float baseScreenY = leftJoy.baseY * SCREEN_HEIGHT;
			float maxRadius = JOYSTICK_MAX_RADIUS * SCREEN_HEIGHT;
			
			float dx = touchInfo[i].x - baseScreenX;
			float dy = touchInfo[i].y - baseScreenY;
			float dist = sqrtf(dx * dx + dy * dy);
			
			// Limitar al radio máximo
			if (dist > maxRadius) {
				dx = dx * maxRadius / dist;
				dy = dy * maxRadius / dist;
				dist = maxRadius;
			}
			
			leftJoy.stickX = leftJoy.baseX + dx / SCREEN_WIDTH;
			leftJoy.stickY = leftJoy.baseY + dy / SCREEN_HEIGHT;
			
			// Calcular salida normalizada (-1 a 1)
			if (dist > JOYSTICK_DEAD_ZONE * maxRadius) {
				leftJoy.outputX = dx / maxRadius;
				leftJoy.outputY = dy / maxRadius;
			} else {
				leftJoy.outputX = 0.0f;
				leftJoy.outputY = 0.0f;
			}
			
			break;
		}
	}
	
	// Si no está activo, resetear
	if (!leftJoy.active) {
		leftJoy.Reset();
	}
	
	// Actualizar zona de cámara (mitad derecha de pantalla, arriba de los botones)
	bool lookActive = false;
	
	for (int i = 0; i < 10; i++) {
		if (!touchInfo[i].pressed)
			continue;
		
		float normX = touchInfo[i].x / SCREEN_WIDTH;
		float normY = touchInfo[i].y / SCREEN_HEIGHT;
		
		// Solo procesar toques en la mitad derecha superior (área de look)
		if (normX < 0.4f || normY > 0.45f)
			continue;
		
		// Verificar que no está tocando un botón
		bool touchingButton = false;
		for (int b = 0; b < NUM_TOUCH_BUTTONS; b++) {
			if (m_Buttons[b].ContainsPoint(touchInfo[i].x, touchInfo[i].y)) {
				touchingButton = true;
				break;
			}
		}
		
		if (touchingButton)
			continue;
		
		if (m_nLookFingerID == -1 || m_nLookFingerID == i) {
			if (m_nLookFingerID == i) {
				// Calcular delta
				m_fLookDeltaX = touchInfo[i].dx * m_fLookSensitivity;
				m_fLookDeltaY = touchInfo[i].dy * m_fLookSensitivity;
			}
			
			m_nLookFingerID = i;
			m_fLastLookX = touchInfo[i].x;
			m_fLastLookY = touchInfo[i].y;
			lookActive = true;
			break;
		}
	}
	
	if (!lookActive) {
		m_nLookFingerID = -1;
		m_fLookDeltaX = 0.0f;
		m_fLookDeltaY = 0.0f;
	}
}

void CTouchInput::UpdateButtons() {
	// Actualizar estado de cada botón - SIN restricciones de dedos
	for (int b = 0; b < NUM_TOUCH_BUTTONS; b++) {
		TouchButton& btn = m_Buttons[b];
		
		// Saltar botones no configurados
		if (btn.size <= 0.0f) {
			btn.pressed = false;
			btn.justPressed = false;
			btn.justReleased = false;
			continue;
		}
		
		bool wasPressed = btn.pressed;
		btn.pressed = false;
		btn.fingerID = -1;
		
		// Buscar si algún dedo está tocando este botón
		for (int i = 0; i < 10; i++) {
			if (!touchInfo[i].pressed)
				continue;
			
			if (btn.ContainsPoint(touchInfo[i].x, touchInfo[i].y)) {
				btn.pressed = true;
				btn.fingerID = i;
				break;
			}
		}
		
		btn.justPressed = btn.pressed && !wasPressed;
		btn.justReleased = !btn.pressed && wasPressed;
	}
}

void CTouchInput::Draw() {
	if (!m_bInitialized)
		return;
	
	DrawJoysticks();
	DrawButtons();
}

void CTouchInput::DrawJoysticks() {
	// Dibujar base del joystick izquierdo
	VirtualJoystick& leftJoy = m_LeftJoystick;
	
	CRect baseRect = leftJoy.GetBaseRect();
	CRect stickRect = leftJoy.GetStickRect();
	
	CRGBA baseColor = leftJoy.active ? CRGBA(100, 100, 100, 120) : CRGBA(80, 80, 80, 80);
	CRGBA stickColor = leftJoy.active ? CRGBA(255, 255, 255, 200) : CRGBA(200, 200, 200, 150);
	
	// Dibujar base como círculo
	if (leftJoy.baseSprite && leftJoy.baseSprite->m_pTexture) {
		leftJoy.baseSprite->Draw(baseRect, baseColor);
	} else {
		// Fallback: dibujar círculo con polígonos
		DrawCircle(leftJoy.baseX * SCREEN_WIDTH, leftJoy.baseY * SCREEN_HEIGHT, 
			leftJoy.baseSize * SCREEN_HEIGHT * 0.5f, baseColor, 24);
	}
	
	// Dibujar stick
	if (leftJoy.stickSprite && leftJoy.stickSprite->m_pTexture) {
		leftJoy.stickSprite->Draw(stickRect, stickColor);
	} else {
		DrawCircle(leftJoy.stickX * SCREEN_WIDTH, leftJoy.stickY * SCREEN_HEIGHT,
			leftJoy.stickSize * SCREEN_HEIGHT * 0.5f, stickColor, 20);
	}
}

void CTouchInput::DrawButtons() {
	for (int b = 0; b < NUM_TOUCH_BUTTONS; b++) {
		TouchButton& btn = m_Buttons[b];
		
		if (btn.size <= 0.0f)
			continue;
		
		CRect rect = btn.GetScreenRect();
		CRGBA color = btn.pressed ? btn.pressedColor : btn.normalColor;
		
		if (btn.sprite && btn.sprite->m_pTexture) {
			btn.sprite->Draw(rect, color);
		} else {
			// Fallback: dibujar círculo
			DrawCircle(btn.x * SCREEN_WIDTH, btn.y * SCREEN_HEIGHT,
				btn.size * SCREEN_HEIGHT * 0.5f, color, 20);
		}
	}
}

// Helpers de dibujo
static void DrawCircle(float cx, float cy, float radius, CRGBA color, int segments) {
	for (int i = 0; i < segments; i++) {
		float angle1 = (float)i / segments * 2.0f * 3.14159f;
		float angle2 = (float)(i + 1) / segments * 2.0f * 3.14159f;
		
		float x1 = cx;
		float y1 = cy;
		float x2 = cx + cosf(angle1) * radius;
		float y2 = cy + sinf(angle1) * radius;
		float x3 = cx + cosf(angle2) * radius;
		float y3 = cy + sinf(angle2) * radius;
		
		CSprite2d::Draw2DPolygon(x1, y1, x2, y2, x3, y3, x3, y3, color);
	}
}

// Getters
bool CTouchInput::IsButtonPressed(TouchButtonID id) {
	if (id < 0 || id >= NUM_TOUCH_BUTTONS)
		return false;
	return m_Buttons[id].pressed;
}

bool CTouchInput::IsButtonJustPressed(TouchButtonID id) {
	if (id < 0 || id >= NUM_TOUCH_BUTTONS)
		return false;
	return m_Buttons[id].justPressed;
}

bool CTouchInput::IsButtonJustReleased(TouchButtonID id) {
	if (id < 0 || id >= NUM_TOUCH_BUTTONS)
		return false;
	return m_Buttons[id].justReleased;
}

int16 CTouchInput::GetLeftStickX() {
	return (int16)(m_LeftJoystick.outputX * 128.0f);
}

int16 CTouchInput::GetLeftStickY() {
	return (int16)(m_LeftJoystick.outputY * 128.0f);
}

int16 CTouchInput::GetRightStickX() {
	return (int16)m_fLookDeltaX;
}

int16 CTouchInput::GetRightStickY() {
	return (int16)(-m_fLookDeltaY);
}

void CTouchInput::SetVehicleMode(bool inVehicle) {
	m_bVehicleMode = inVehicle;
	// Aquí se podría cambiar el layout de botones para vehículo
}

float CTouchInput::ScreenToNormX(float screenX) {
	return screenX / SCREEN_WIDTH;
}

float CTouchInput::ScreenToNormY(float screenY) {
	return screenY / SCREEN_HEIGHT;
}

float CTouchInput::NormToScreenX(float normX) {
	return normX * SCREEN_WIDTH;
}

float CTouchInput::NormToScreenY(float normY) {
	return normY * SCREEN_HEIGHT;
}
