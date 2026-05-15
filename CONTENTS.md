# Contents

See README.md / CONTENTS.md in sub-directories for more info where available.

## AfxCppCli

This is old C++/CLI .NET code that provides the following:

- GoldSrc related DemoTools, including GUI.
- GoldSrc related Sky Manager, inclduing GUI.
- An interface to the native shared/AfxColorLut.* classes

## AfxHookGoldSrc

This is the C++ hook DLL for GoldSrc (Half-Life based) games.

Currently only supports Win32.

## AfxHookSource

This is the C++ hook DLL for a few Source 1 games (mostly Counter-Strike: Global Offensive).

Most code is only for Win32, however for Team Fortress 2 there's Win64 code.

## AfxHookSource2

C++ Hook DLL for Counter-Strike 2, also statically links in a Rust library, mostly for mirv-script features.

Currently only supports Win64.

## deps

This folder orgnaizes sub git repositories and projects pulled with CMake that we depend on.

## doc

Text file notes, mostly taken for giving threads of though when having to update the hooks.

## hlae

The main GUI project / launcher GUI written in C# .NET.

Released in as Win32 HLAE.exe binary.

## installer

Code for the Windows installer HLAE_Setup.exe.

## lib

Folder for Rust libraries.

## misc

Miscelaneous things, most notably currently the mirv-script things are in here.

## resources

Miscelaneous resources for the HLAE install. Most notably the changelog lives here.

## ShaderBuilder

C# .NET Project that builds / compiles the shaders combinations and organizes them into shader combo files (.acs) needed by HLAE release.

## ShaderDiassembler

Tool for dissambling compiled shaders (in format as loaded by DirectX).

## shaders

Source shader code for ShaderBuilder

## shared

Mostly C++ code shared by the various hooks.
