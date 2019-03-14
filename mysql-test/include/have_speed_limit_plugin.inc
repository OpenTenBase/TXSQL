#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip Requires dynamic loading
}

#
# Check if the variable SPEED_LIMIT_MASTER_PLUGIN is set
#
if (!$SPEED_LIMIT_MASTER_PLUGIN)
{
  skip Need speed limit plugins;
}

#
# Check if --plugin-dir was setup for semisync
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$SPEED_LIMIT_PLUGIN_OPT/'`) {
  --skip SPEED_LIMIT plugin requires that --plugin-dir is set to the speed limit plugin dir (either the .opt file does not contain \$SPEED_LIMIT_PLUGIN_OPT or another plugin is in use)
}
