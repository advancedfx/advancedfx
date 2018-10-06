#pragma once

// Description: Counter-Strike 1.6 cross-hair fix related.


/// <summary>Whether to prevent cross-hair cool-down for the current pass (true) or not (false).</summary>
extern bool g_Cstrike_CrossHair_Block;

/// <returns>Current setting for the cross-hair fix.</returns>
double Cstrike_CrossHair_Fps_get();

/// <summary>Enables the CrossHair fix (0.0 < value) or disables it (value &lt;= 0)</summary>
void Cstrike_CrossHair_Fps_set(double value);

/// <summary>Installs the cstrike cross-hair fix hook.</summary>
bool Hook_Cstrike_CrossHair_Fix();