#include <fstream>
#include <streambuf>

#include "Logging.h"
#include "Preferences.h"

Preferences* Preferences::Instance = 0;

void Preferences::Load()
{
	LOG_INFO("Loading Preferences from file: ");

	// Load the file as json
	json_error_t error;
	json_t* root = json_load_file("preferences.json", 0, &error);
	if (!root)
	{
		LOG_ERROR("Preferences.Load: error on line " << error.line << ":" << error.text);

		// Call save once, this adds new properties to the prefs file
		Save();
		return;
	}

	// Read properties
	ReadBool(root, "isEnabled", IsEnabled);
	ReadInt(root, "waitDurationInSecs", WaitDurationInSecs);
	ReadInt(root, "pumpDurationInSecs", PumpDurationInSecs);

	// Cleanup
	json_decref(root);

	// Call save once, this add new properties to the prefs file
	Save();
}

json_t* Preferences::JsonEncode() const
{
	// Create new empty json object
	json_t* root = json_object();

	// Add to root json object
	json_object_set_new(root, "isEnabled", IsEnabled ? json_true() : json_false());
	json_object_set_new(root, "waitDurationInSecs", json_integer(WaitDurationInSecs));
	json_object_set_new(root, "pumpDurationInSecs", json_integer(PumpDurationInSecs));

	return root;
}

void Preferences::Save() const
{
	// todo: save in executable dir
	LOG_INFO("Saving Preferences");

	// Write json to file
	json_t* root = JsonEncode();
	json_dump_file(root, "preferences.json", JSON_INDENT(4));
	json_decref(root);
}

void Preferences::ReadInt(const json_t *root, const char* propertyName, int& dest)
{
	json_t* jInt = json_object_get(root, propertyName);
	if (!jInt)
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' does not exist");
		return;
	}

	if (json_is_integer(jInt))
    {
		dest = (int)json_integer_value(jInt);
		LOG_INFO(" - " << propertyName << " = " << dest);
    }
	else
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' is not an integer");
	}
}

void Preferences::ReadInt8(const json_t *root, const char* propertyName, uint8_t& dest)
{
	int i;
	ReadInt(root, propertyName, i);
	dest = (uint8_t)i;
}

void Preferences::ReadFloat(const json_t *root, const char* propertyName, float& dest)
{
	json_t* jReal = json_object_get(root, propertyName);
	if (!jReal)
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' does not exist");
		return;
	}

	if (json_is_real(jReal))
    {
		dest = (float)json_real_value(jReal);
		LOG_INFO(" - " << propertyName << " = " << dest);
    }
	else
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' is not a float");
	}
}


void Preferences::ReadBool(const json_t *root, const char* propertyName, bool& dest)
{
	json_t* jBool = json_object_get(root, propertyName);
	if (!jBool)
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' does not exist");
		return;
	}

	if (json_is_boolean(jBool))
    {
		dest = json_is_true(jBool);
		LOG_INFO(" - " << propertyName << " = " << (dest?"true":"false"));
    }
	else
	{
		LOG_ERROR("Preferences.Load: '" << propertyName << "' is not a boolean");
	}
}
