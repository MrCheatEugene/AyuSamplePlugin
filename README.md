# AyuSamplePlugin
An example of an Ayugram Desktop plugin for AyuGram Desktop Plugin Engine
[Read the docs for more info](https://github.com/MrCheatEugene/AyuGramDesktop-PLEngine/blob/dev/README-pluginEngine.md)

# Useful info
- DllMain should return for plugin to Load
- pluginInfo must return a valid ptr to AyuPlugin structure
- internalLoop is always ran in a loop, so there's no need to use a while loop by default :/
- Plugins always run in each separate thread
- doFilterHistoryItem can be used in pair with AyuPlugin->sharedFiltersEnabled to enable shadow-filtering HistoryItems

# A side note
To build, you need to re-define my AyuGram build directory (edit project & solution files for that purpose)
Sorry for that, hope it'll be fixed soon
