#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <exception>
#include <csignal>
#include <wiringPi.h>

#define BLYNK_PRINT stdout
#include <BlynkApiWiringPi.h>
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>

#include "stacktrace.h"
#include "Logging.h"
#include "Config.h"
#include "Preferences.h"
#include "PumpController.h"

using namespace std;

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

Preferences* prefs;
PumpController* pumpController = 0;

void cleanup()
{
	// Makes sure the pump is disabled
	if (pumpController != 0)
	{
		delete pumpController;
		pumpController = 0;
	}

	// Saves current settings
	if (prefs != 0)
	{
		prefs->Save();
		delete prefs;
		prefs = 0;
	}

	LOG_INFO("Shutdown...\n");
	log4cplus::Logger::shutdown();
}

void signalHandler(int signum)
{
	fprintf(stderr, "Signal '%i' received\n", signum);
	print_stacktrace(stderr, 300);
	cleanup();
    exit(signum);
}

void onPumpStateChanged(PumpControllerState prevState, PumpControllerState currState)
{
	Blynk.virtualWrite(BLYNK_PIN_STATE_DISABLED, (currState == PUMP_STATE_DISABLED) ? 1 : 0);
	Blynk.virtualWrite(BLYNK_PIN_STATE_WAITING, (currState == PUMP_STATE_WAITING) ? 1 : 0);
	Blynk.virtualWrite(BLYNK_PIN_STATE_PUMPING, (currState == PUMP_STATE_PUMPING) ? 1 : 0);
	Blynk.virtualWrite(BLYNK_PIN_STATE_COMPLETION, pumpController->GetStateCompletion() * 100);
}

void pushPrefsToServer()
{
	onPumpStateChanged(PUMP_STATE_DISABLED, pumpController->GetState());
	Blynk.virtualWrite(BLYNK_PIN_POWER, prefs->IsEnabled ? 1 : 0);
	Blynk.virtualWrite(BLYNK_PIN_WAIT_TIME_MINS, prefs->WaitDurationInSecs / 60);
	Blynk.virtualWrite(BLYNK_PIN_PUMP_TIME_MINS, prefs->PumpDurationInSecs / 60);
}

int main(int argc, char* argv[])
{
	signal(SIGABRT, signalHandler);
	signal(SIGFPE, signalHandler);
	signal(SIGILL, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGTERM, signalHandler);

	try
	{
		// Logging setup
		LogUtil::Setup();

		// Blynk setup
		const char *auth, *serv, *port;
		parse_options(argc, argv, auth, serv, port);
		Blynk.begin(auth, serv, port);

		// Load preferences (saved settings from last run)
		prefs = new Preferences();
		prefs->Load();

		// Setup controller
		pumpController = new PumpController(PUMP_PIN);
		pumpController->SetOnStateChangedCallback(onPumpStateChanged);
		pumpController->SetWaitDuration(prefs->WaitDurationInSecs);
		pumpController->SetPumpDuration(prefs->PumpDurationInSecs);
		pumpController->SetEnabled(prefs->IsEnabled);

		// Main program loop
		unsigned int prevTimeMs = millis();
		bool previouslyConnected = false;
		float stateCompletionTimer = 0;
		while (true)
		{
			Blynk.run();

			// Handle connection established
			if (!previouslyConnected && Blynk.connected())
			{
				LOG_INFO("Blynk connection established: pushing current prefs and state");
				pushPrefsToServer();
			}
			previouslyConnected = Blynk.connected();

			// Calculate delta time
			unsigned int currentTimeMs = millis();
			float deltaTime = float(currentTimeMs - prevTimeMs) / 1000.0f;
			prevTimeMs = currentTimeMs;

			// Update pump state machine
			pumpController->Update(deltaTime);

			// Update the app with current completion state
			stateCompletionTimer += deltaTime;
			if (stateCompletionTimer > 1)
			{
				float completion = pumpController->GetStateCompletion();
				Blynk.virtualWrite(BLYNK_PIN_STATE_COMPLETION, completion * 100);
				stateCompletionTimer = 0;
			}

			// sleep 1ms
			delay(1);
		}
	}
	catch (std::exception& e)
	{
		cleanup();
		LOG_FATALF("Exception: %s\n", e.what());
		print_stacktrace();
		return -1;
	}
	catch (...)
	{
		cleanup();
		print_stacktrace();
		return -1;
	}

	return 0;
}

// Blynk events
BLYNK_WRITE(BLYNK_PIN_PUMP_TIME_MINS)
{
	pumpController->SetPumpDuration(param[0].asInt() * 60);
}
BLYNK_WRITE(BLYNK_PIN_WAIT_TIME_MINS)
{
	pumpController->SetWaitDuration(param[0].asInt() * 60);
}
BLYNK_WRITE(BLYNK_PIN_POWER)
{
	bool powerButtonOn = (param[0].asInt() == 1);
	pumpController->SetEnabled(powerButtonOn);
}
BLYNK_WRITE(BLYNK_PIN_RESET_WAIT)
{
	bool pressed = (param[0].asInt() == 1);
	if (pressed)
	{
		pumpController->Reset(PUMP_STATE_WAITING);
	}
}
BLYNK_WRITE(BLYNK_PIN_RESET_PUMP)
{
	bool pressed = (param[0].asInt() == 1);
	if (pressed)
	{
		pumpController->Reset(PUMP_STATE_PUMPING);
	}
}
BLYNK_WRITE(BLYNK_PIN_SYNC)
{
	bool pressed = (param[0].asInt() == 1);
	if (pressed)
	{
		pushPrefsToServer();
	}
}


BLYNK_READ(BLYNK_PIN_POWER)
{
	LOG_DEBUG("BLYNK_PIN_POWER value requested");
	PumpControllerState currState = pumpController->GetState();
	Blynk.virtualWrite(BLYNK_PIN_POWER, (currState != PUMP_STATE_DISABLED) ? 1 : 0);
}
BLYNK_READ(BLYNK_PIN_STATE_DISABLED)
{
	LOG_DEBUG("BLYNK_PIN_STATE_DISABLED value requested");
	PumpControllerState currState = pumpController->GetState();
	Blynk.virtualWrite(BLYNK_PIN_STATE_DISABLED, (currState == PUMP_STATE_DISABLED) ? 1 : 0);
}
BLYNK_READ(BLYNK_PIN_STATE_WAITING)
{
	LOG_DEBUG("BLYNK_PIN_STATE_WAITING value requested");
	PumpControllerState currState = pumpController->GetState();
	Blynk.virtualWrite(BLYNK_PIN_STATE_WAITING, (currState == PUMP_STATE_WAITING) ? 1 : 0);
}
BLYNK_READ(BLYNK_PIN_STATE_PUMPING)
{
	LOG_DEBUG("BLYNK_PIN_STATE_PUMPING value requested");
	PumpControllerState currState = pumpController->GetState();
	Blynk.virtualWrite(BLYNK_PIN_STATE_PUMPING, (currState == PUMP_STATE_PUMPING) ? 1 : 0);
}

BLYNK_READ(BLYNK_PIN_PUMP_TIME_MINS)
{
	LOG_DEBUG("BLYNK_PIN_PUMP_TIME_MINS value requested");
	Blynk.virtualWrite(BLYNK_PIN_PUMP_TIME_MINS, Preferences::Instance->PumpDurationInSecs / 60);
}
BLYNK_READ(BLYNK_PIN_WAIT_TIME_MINS)
{
	LOG_DEBUG("BLYNK_PIN_WAIT_TIME_MINS value requested");
	Blynk.virtualWrite(BLYNK_PIN_WAIT_TIME_MINS, Preferences::Instance->WaitDurationInSecs / 60);
}

BLYNK_READ(BLYNK_PIN_GRAPH_PUMP_STATE)
{
	LOG_DEBUG("BLYNK_PIN_GRAPH_PUMP_STATE value requested");
	Blynk.virtualWrite(BLYNK_PIN_GRAPH_PUMP_STATE, (int)pumpController->GetState());
}

