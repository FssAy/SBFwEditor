// Version: 0.1

#include "pch.h"
#include "sbfe.h"
#include <iostream>

int retVal = 4;


// Instantiate INetFwPolicy2 
// (https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/security/windowsfirewall/getfirewallsettings/GetFirewallSettings.cpp)
HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)ppNetFwPolicy2);

    if (FAILED(hr))
    {
        goto Cleanup;
    }

    Cleanup:
    return hr;
}

// unsafe! Fix it
bool RuleExists(INetFwRules* pFwRules, BSTR ruleName, INetFwRule** pFwRule) {
    if (pFwRules == NULL) {
        
    }

    HRESULT result = pFwRules->Item(ruleName, pFwRule);
    return result == S_OK;
}

HRESULT CoCreateRule(LPVOID* pFwRule) {
    return CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        pFwRule
    );
}

unsigned int IsEnabled() {
    VARIANT_BOOL enabled = V_TRUE;
    INetFwPolicy2* pFwPolicy2 = NULL;
    INetFwRules* pFwRules = NULL;
    INetFwRule* pFwRule = NULL;

    BSTR ruleName = SysAllocString(L"Steam Blocker [@SB]");

    CHECK(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), 2);
    CHECK(WFCOMInitialize(&pFwPolicy2), 2);
    CHECK(pFwPolicy2->get_Rules(&pFwRules), 2);

    retVal = 0;
    if (RuleExists(pFwRules, ruleName, &pFwRule)) {
        pFwRule->get_Enabled(&enabled);
        if (enabled) {
            retVal = 1;
        }
    }

    Cleanup:
    SysFreeString(ruleName);

    if (pFwPolicy2 != NULL) pFwPolicy2->Release();
    if (pFwRules != NULL) pFwRules->Release();
    if (pFwRule != NULL) pFwRule->Release();
    CoUninitialize();

    return retVal;
}

unsigned int Enable(const char* appPath) {
    HRESULT result = S_OK;
    HRESULT resultCommit = S_OK;
    INetFwRule* pFwRule = NULL;
    INetFwRules* pFwRules = NULL;
    INetFwPolicy2* pFwPolicy2 = NULL;
    VARIANT_BOOL enabled = V_TRUE;
    
    int size = strlen(appPath);
    TCHAR unicode_string[300];
    size_t length = 0;
    mbstowcs_s(&length, unicode_string, appPath, static_cast<size_t>(size) + 1);

    BSTR ruleName = SysAllocString(L"Steam Blocker [@SB]");
    BSTR ruleAppName = SysAllocString(unicode_string);
    BSTR ruleDsc = SysAllocString(L"Rule automatically managed by Steam Blocker app.");
    BSTR ruleInterfaces = SysAllocString(NET_FW_RULE_INTERFACE_ALL);

    CHECK(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED), 2);
    CHECK(WFCOMInitialize(&pFwPolicy2), 2);
    CHECK(pFwPolicy2->get_Rules(&pFwRules), 2);

    if (RuleExists(pFwRules, ruleName, &pFwRule)) {
        if (pFwRule == NULL) {
            RETURN_ERROR(2)
        }

        pFwRule->get_Enabled(&enabled);
        if (enabled) {
            enabled = V_FALSE;
        }
        else {
            enabled = V_TRUE;
        }

        CHECK(pFwRules->Remove(ruleName), 3);
    }
    else {
        CHECK(CoCreateRule((LPVOID*)&pFwRule), 3);
    }

    pFwRule->put_Name(ruleName);
    pFwRule->put_ApplicationName(ruleAppName);
    pFwRule->put_Description(ruleDsc);
    pFwRule->put_Action(NET_FW_ACTION_BLOCK);
    pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
    pFwRule->put_InterfaceTypes(ruleInterfaces);
    pFwRule->put_Enabled(enabled);

    result = pFwRules->Add(pFwRule);
    if (FAILED(result) && result != E_UNEXPECTED)
    {
        if (result == E_ACCESSDENIED) {
            retVal = 3;
        }
        else {
            retVal = 2;
        }
    }
    else {
        if (enabled) {
            retVal = 1;
        }
        else {
            retVal = 0;
        }
    }

    Cleanup:
    SysFreeString(ruleName);
    SysFreeString(ruleAppName);
    SysFreeString(ruleDsc);
    SysFreeString(ruleInterfaces);

    if (pFwPolicy2 != NULL) pFwPolicy2->Release();
    if (pFwRules != NULL) pFwRules->Release();
    if (pFwRule != NULL) pFwRule->Release();
    if (SUCCEEDED(resultCommit)) CoUninitialize();

    return retVal;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
