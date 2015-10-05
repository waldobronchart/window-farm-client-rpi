#pragma once

#include <stdint.h>
#include <string>
#include <jansson.h>

using namespace std;

class Preferences
{
public:
	static Preferences* Instance;

	Preferences() : IsEnabled(false), WaitDurationInSecs(600), PumpDurationInSecs(300)
	{
		Instance = this;
	}

	void Load();
	json_t* JsonEncode() const;
	void Save() const;

public:
	bool IsEnabled;
	int WaitDurationInSecs;
	int PumpDurationInSecs;

private:
	void ReadInt(const json_t *root, const char* propertyName, int& dest);
	void ReadInt8(const json_t *root, const char* propertyName, uint8_t& dest);
	void ReadFloat(const json_t *root, const char* propertyName, float& dest);
	void ReadBool(const json_t *root, const char* propertyName, bool& dest);
};
