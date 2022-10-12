call mtr.add_suppression("Plugin \'MEMORY\'");
Pattern "\[ERROR\] \[[^]]*\] \[[^]]*\] unknown variable 'performance-schema-enabled=maybe'" not found
# Server start with invalid startup option value 'performance-schema-enabled=maybe' : pass
Pattern "\[ERROR\] \[[^]]*\] \[[^]]*\] unknown variable 'performance-schema-max_=12'" not found
# Server start with ambigous startup option 'performance-schema-max_=12' : pass
Pattern "\[ERROR\] \[[^]]*\] \[[^]]*\] unknown option '--performance-schema-unknown_99'" not found
# Server start with invalid startup option '--performance-schema-unknown_99' : pass
Pattern "\[ERROR\] \[[^]]*\] \[[^]]*\] Failed to set datadir to \'.*bad_option_h_param" not found
# Server start with invalid startup option value '--datadir=bad_option_h_param' : pass
