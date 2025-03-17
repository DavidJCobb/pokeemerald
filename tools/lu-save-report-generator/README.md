
This repo uses a custom compiler plug-in to generate code to optimize the game's save file format. The plug-in outputs an XML file describing the generated format.

This folder contains Lua-language post-build scripts which parse that XML file and generate a human-readable Markdown file at [`reports/savedata.md`](../../reports/savedata.md), which lists statistics regarding the generated save file format's space usage and savings.