#!/bin/bash /etc/rc.common
. /lib/functions.sh


CUSTOM_RULE_FILE="/tmp/ipadd.conf"
CLASH_CONFIG="/etc/clash/config.yaml"
append=$(uci get clash.config.append_rules 2>/dev/null)
if [ "${append}" -eq 1 ];then


if [ -f $CUSTOM_RULE_FILE ];then
	rm -rf $CUSTOM_RULE_FILE 2>/dev/null
fi
	   
	   
ipadd()
{


	   local section="$1"
	   config_get "pgroup" "$section" "pgroup" ""
	   config_get "ipaaddr" "$section" "ipaaddr" ""
	   config_get "type" "$section" "type" ""
	   echo "- $type,$ipaaddr,$pgroup">>/tmp/ipadd.conf
}

	
 config_load clash
 config_foreach ipadd "addtype"


if [ -f $CUSTOM_RULE_FILE ];then

sed -i -e "\$a#*******CUSTOM-RULE-END**********#" $CUSTOM_RULE_FILE 2>/dev/null
sed -i '/#*******CUSTOM-RULE-START**********#/,/#*******CUSTOM-RULE-END**********#/d' "$CLASH_CONFIG" 2>/dev/null

if [ ! -z "$(grep "^ \{0,\}- GEOIP" "/etc/clash/config.yaml")" ]; then
   sed -i '1,/^ \{0,\}- GEOIP,/{/^ \{0,\}- GEOIP,/s/^ \{0,\}- GEOIP,/#*******CUSTOM-RULE-START**********#\n&/}' "$CLASH_CONFIG" 2>/dev/null
elif [ ! -z "$(grep "^ \{0,\}- MATCH," "/etc/clash/config.yaml")" ]; then
   sed -i '1,/^ \{0,\}- MATCH,/{/^ \{0,\}- MATCH,/s/^ \{0,\}- MATCH,/#*******CUSTOM-RULE-START**********#\n&/}' "$CLASH_CONFIG" 2>/dev/null
else
   echo "#*******CUSTOM RULE START**********#" >> "$CLASH_CONFIG" 2>/dev/null
fi

sed -i '/CUSTOM-RULE-START/r/tmp/ipadd.conf' "$CLASH_CONFIG" 2>/dev/null
fi

else
sed -i '/#*******CUSTOM-RULE-START**********#/,/#*******CUSTOM-RULE-END**********#/d' "$CLASH_CONFIG" 2>/dev/null
fi
