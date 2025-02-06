PLUGINS_DIR := plugins
PLUGIN_NAMES := lu_bitpack # space-separated list

PLUGINDIRS := $(PLUGIN_NAMES:%=$(PLUGIN_DIR)/%)

# Tool making doesnt require a pokeemerald dependency scan.
RULES_NO_SCAN += plugins check-plugins clean-plugins $(PLUGINDIRS)
.PHONY: $(RULES_NO_SCAN)

plugins: $(PLUGINDIRS)

$(PLUGINDIRS):
	@$(MAKE) -C $@

clean-plugins:
	@$(foreach plugindir,$(PLUGINDIRS),$(MAKE) clean -C $(plugindir);)
