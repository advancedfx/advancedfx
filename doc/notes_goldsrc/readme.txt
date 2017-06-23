The debug_*.txt files are traces on how to find addresses with WinDbg.
They are used in ../AfxHookGoldSrc/hl_addresses.cpp, the file
AfxHookGoldSrc uses to initiate the addresses.
Also some code in ../AfxHookGoldSrc uses info gained from the debugging
directly.

Several names and background information used refer to

a) The Half-Life SDK which can be found on
   http://github.com/ValveSoftware/halflife

b) The Quake 1 source code, since Valve built Half-Life on the QuakeWorld source
   code and several Half-Life internals and parts of the structure remained
   similar or even the same:
   http://github.com/id-Software/Quake
   
Those 2 source codes are pretty much from where we gain info about
Half-Life internals which we use to build HLAE / AfxHookGoldSrc.