
These libraries exist for use by various post-build scripts written in Lua. Post-build scripts have their own folders within the repo's `tools` folder, and include:

* **`lu-save-js-indexer`:** Generates data files for use by the included save editor (which itself is located in `usertools/lu-save-js`).

* **`lu-save-report-generator`:** Generates human-readable (Markdown) reports on the savegame format, listing the overall space savings as well as space usage in a variety of categories.
