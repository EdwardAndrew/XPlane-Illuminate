# XPlane-Illuminate
XPlane-Illuminate is an XPlane Plugin that aims to provide functionality for users to add their own colour mappings to their RGB keyboards, powered by [XPlane DataRefs]().

## Compatibility
Keyboards:
- At the moment, only Corsair RGB keyboards are supported, however in the future I would also like to add more keyboards.

Operating Systems:
- Currently supported only on Windows and tested only on Windows 10.
## Getting Started 
### From Source
You will need [Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2017) with Windows SDK.

If you're unfamiliar with generating project files from `CMakeLists.txt` I suggest you use [cmake-gui](https://cmake.org/download/).

You will need the following dependencies:
- [XPlane-11 SDK 3.0.1](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
- [Corsair CUE SDK v1.15.28](http://forum.corsair.com/v3/showthread.php?t=156813)

Set the following key values in CMake:
- `CUESDK_PATH`
- `XPlane_11_SDK_PATH`

Select `Configure` and chose `Visual Studio 15 2017 Win64`. Then select `Generate`.

Navigate to the folder where you built the solution file to, and open the solution file with Visual Studio.

Select from the top Menu `Build` -> `Build Solution`.

Copy the generated `plugins` folder to your XPlane installation `Resources/` folder.