#include <wiringPi.h>
#include <string>

#include "Logging.h"
#include "PumpController.h"
#include "Preferences.h"

using namespace std;

static string PUMP_STATE_NAMES[] = {"PUMP_STATE_INIT", "PUMP_STATE_DISABLED", "PUMP_STATE_WAITING", "PUMP_STATE_PUMPING"};

#define PUMP_ON LOW
#define PUMP_OFF HIGH

PumpController::PumpController(int relayPin) : 
	m_relayPin(relayPin), m_state(PUMP_STATE_INIT), 
	m_waitDuration(3600), m_pumpDuration(3600), m_currentTimer(0),
	m_stateChangeCallback(0)
{
	LOG_INFO("PumpController: setting up with relayPin=" << relayPin);

	pinMode(relayPin, OUTPUT);
	ChangeState(PUMP_STATE_DISABLED);
}

PumpController::~PumpController()
{
	// Make sure the pump is off when the app shuts down
	digitalWrite(m_relayPin, PUMP_OFF);
}

void PumpController::SetWaitDuration(float waitDuration)
{
	waitDuration = (waitDuration < 5) ? 5 : waitDuration;
	if (waitDuration == m_waitDuration)
		return;

	LOG_DEBUGF("...PumpController: set wait duration to %fsecs", waitDuration);
	m_waitDuration = waitDuration;
	Preferences::Instance->WaitDurationInSecs = waitDuration;
}

void PumpController::SetPumpDuration(float pumpDuration)
{
	pumpDuration = (pumpDuration < 5) ? 5 : pumpDuration;
	if (pumpDuration == m_pumpDuration)
		return;

	LOG_DEBUGF("...PumpController: set pump duration to %fsec", pumpDuration);
	m_pumpDuration = pumpDuration;
	Preferences::Instance->PumpDurationInSecs = pumpDuration;
}

void PumpController::SetOnStateChangedCallback(void (*stateChangeCallback)(PumpControllerState, PumpControllerState))
{
	m_stateChangeCallback = stateChangeCallback;
}

void PumpController::SetEnabled(bool enabled)
{
	bool wasEnabled = (m_state != PUMP_STATE_DISABLED);
	if (enabled == wasEnabled)
		return;

	if (enabled)
	{
		LOG_INFO("PumpController Power ON");
		ChangeState(PUMP_STATE_WAITING);
	}
	else
	{
		LOG_INFO("PumpController Power OFF");
		ChangeState(PUMP_STATE_DISABLED);
	}
	Preferences::Instance->IsEnabled = enabled;
}

void PumpController::Reset(PumpControllerState startState)
{
	LOG_INFO("PumpController Reset");
	ChangeState(PUMP_STATE_DISABLED);
	ChangeState(startState);
}

PumpControllerState PumpController::GetState() const
{
	return m_state;
}

float PumpController::GetStateCompletion()
{
	switch (m_state)
	{
		case PUMP_STATE_WAITING:
			return m_currentTimer / m_waitDuration;
		case PUMP_STATE_PUMPING:
			return m_currentTimer / m_pumpDuration;
	}

	return m_currentTimer;
}

void PumpController::ChangeState(PumpControllerState newState)
{
	if (newState == m_state)
		return;

	LOG_DEBUGF("...PumpController: ChangeState from %s to %s", 
		PUMP_STATE_NAMES[m_state].c_str(), PUMP_STATE_NAMES[newState].c_str(), m_currentTimer);

	switch (newState)
	{
		case PUMP_STATE_WAITING:
			LOG_INFOF("...waiting for %fsecs", m_waitDuration);
			m_currentTimer = 0;
			break;

		case PUMP_STATE_PUMPING:
			LOG_INFOF("...pumping for %fsecs", m_pumpDuration);
			m_currentTimer = 0;
			break;
	}

	// Callback
	if (m_stateChangeCallback != 0)
	{
		m_stateChangeCallback(m_state, newState);
	}

	m_state = newState;
}

void PumpController::Update(float deltaTime)
{
	switch (m_state)
	{
		case PUMP_STATE_DISABLED:
			digitalWrite(m_relayPin, PUMP_OFF);
			break;

		case PUMP_STATE_WAITING:
			digitalWrite(m_relayPin, PUMP_OFF);

			m_currentTimer += deltaTime;
			if (m_currentTimer >= m_waitDuration)
			{
				ChangeState(PUMP_STATE_PUMPING);
			}
			break;

		case PUMP_STATE_PUMPING:
			digitalWrite(m_relayPin, PUMP_ON);

			m_currentTimer += deltaTime;
			if (m_currentTimer >= m_pumpDuration)
			{
				ChangeState(PUMP_STATE_WAITING);
			}
			break;
	}
}
