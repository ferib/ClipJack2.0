# ClipJack 2.0
New ClipJack written in C++, based on the old Assembly one (C# was used to inject).

# What is this?
This tools is a PoC to show you how third-party software can inject itself into a process and take control over the Clipboard in order to manipulate bitcoin addresses.

# How does it work?
The DLL gets injected in a target process (x64 only), the DLL will hook the GetClipboardData() from user32.DLL in order to fuck with the clipboard.

# GetClipboardData
!(https://github.com/ferib/ClipJack2.0/blob/master/images/hookinfo.png?raw=true)

