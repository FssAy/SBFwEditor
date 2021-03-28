#pragma once

#define NET_FW_RULE_INTERFACE_ALL L"All"
#define V_TRUE 0xFFFF
#define V_FALSE 0x0000
#define RETURN_ERROR(code) retVal = code; goto Cleanup;
#define CHECK(exp, code) if(FAILED(exp)) { RETURN_ERROR(code) }

extern "C" __declspec(dllexport) unsigned int Enable(const char* appPath);  /*
	-> RETURN CODES <-
	0 - Disabled
	1 - Enabled
	2 - Internal error
	3 - No permissions
	4 - Unknown
*/

extern "C" __declspec(dllexport) unsigned int IsEnabled();  /*
	-> RETURN CODES <-
	0 - False
	1 - True
*/
