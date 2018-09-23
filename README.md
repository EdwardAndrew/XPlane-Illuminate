# This weeks release
[Download from WeTransfer](https://we.tl/t-FeMsijs7PX)


# XPlane-Illuminate
XPlane-Illuminate is an XPlane Plugin that aims to provide functionality for users to add their own colour mappings to their RGB keyboards, powered by XPlane DataRefs.

## Compatibility
Keyboards:
- At the moment, only Corsair RGB keyboards are supported, however in the future I would also like to add more brands.

Operating Systems:
- Currently supported only on Windows and tested only on Windows 10.
## Getting Started 
### From Source
You will need [Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2017) with Windows SDK.

If you're unfamiliar with generating project files from `CMakeLists.txt` I suggest you use [cmake-gui](https://cmake.org/download/).

You will need the following dependencies:
- [XPlane-11 SDK 3.0.1](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
- [Corsair CUE SDK v1.15.28](http://forum.corsair.com/v3/showthread.php?t=156813)
- [nlohmann/json](https://github.com/nlohmann/json)

Set the following key values in CMake:
- `CUESDK_PATH`
- `XPlane_11_SDK_PATH`
- `NLOHMANN/JSON PATH`

Select `Configure` and chose `Visual Studio 15 2017 Win64`. Then select `Generate`.

Navigate to the folder where you built the solution file to, and open the solution file with Visual Studio.

Select from the top Menu `Build` -> `Build Solution`.

Copy the generated `plugins` folder to your XPlane installation `Resources/` folder.

## Configuration
I appreciate the configuration may be a little tedious at the moment, and I'd love to hear feedback for improving it. I would also like to make a nice widget for configuring the plugin directly from XPlane.


When X-Plane is launched, Illuminate will look inside it's folder at `Resources/plugins/XPlane-Illuminate` for file named `illuminate.json`. This file is needed and contains all the information for the plugin.

[Here is an example configuration file.](https://gist.github.com/EdwardAndrew/f3f50df11b2dfad78b51f5055931c09f)

This file must be valid json, so I recommend using a [JSON validator](https://jsonformatter.curiousconcept.com) just to make sure it's okay before attempting to load it once you've finished editing it.

As the plugin is still in development, this file is likely to change from time to time.

Currently there are 3 main parts to the configuration file:
- colors 
- conditions
- keys

### Colors
This is an array.

Colors should be written as objects with the following format:
```
{
  "name": text,
  "r": number,
  "g": number,
  "b": number
}
```
A color with the name `background` is expected, this is used as a background color for the rest of the keyboard, any other colors names you like can be added here and used later in the file.

### Conditions
This is is an array.

A conditon will have one of the following syntax depending on it's DataRef Type.
If the dataRef type is an Array it will need to know which index to use the value from:
```
{
 "name": text,
 "dataRef": text,
 "index": integer,
 "match": "exactly" or "less_than" or "greater_than",
 "value": number
}
```
or if the dataRef is not an array we do not need the index:
```
{
  "name": text,
  "dataRef": text,
  "match": "exactly" or "less_than" or "greater_than",
  "value": number
}
```
You can find out more about XPlane DataRefs including which ones are Arrays [here](http://www.xsquawkbox.net/xpsdk/docs/DataRefs.html)
### Keys
This is an Array

Keys currently use the following format:
```
{
   "conditions": [
    text     // Condition name you gave the condition in the Condition's array
   ],
   "key": text or integer, // eg, "w" or 93
   "color": text // Color name you specifed in the Colors  section, eg "red"
}
```
You can enter either the single character of the key you wish to use and the plugin will attempt to locate the correct key, or use a number to more explicity state the key.

[Here is the full list of CorsairKeyCodes which this file uses for the time being](https://gist.github.com/EdwardAndrew/28ea357712846e7e2075a8cda8e16513)
