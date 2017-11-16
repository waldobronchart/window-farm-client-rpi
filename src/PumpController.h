#pragma once

enum PumpControllerState
{
	PUMP_STATE_INIT,
	PUMP_STATE_DISABLED,
	PUMP_STATE_WAITING,
	PUMP_STATE_PUMPING
};

class PumpController
{
public:
	PumpController(int relayPin);
	~PumpController();

	void SetEnabled(bool enabled);
	void SetWaitDuration(float duration);
	void SetPumpDuration(float pumpDuration);
	void SetOnStateChangedCallback(void (*)(PumpControllerState, PumpControllerState));
	void Reset(PumpControllerState startState);
	PumpControllerState GetState() const;
	float GetStateCompletion();
	void Update(float deltaTime);

private:
	void ChangeState(PumpControllerState state);

	int m_relayPin;
	PumpControllerState m_state;
	float m_waitDuration;
	float m_pumpDuration;
	float m_currentTimer;
	void (*m_stateChangeCallback)(PumpControllerState prevState, PumpControllerState currState);
};
