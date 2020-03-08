#!/bin/bash /etc/rc.common
. /lib/functions.sh

GROUP_FILE="/tmp/yaml_group.yaml"
GAME_RULE_FILE="/tmp/yaml_game_rule_group.yaml"
CLASH_CONFIG="/etc/clash/config.yaml"
game_rules=$(uci get clash.config.g_rules 2>/dev/null)

if [ "${game_rules}" -eq 1 ];then

get_rule_file()
{
   if [ -z "$1" ]; then
      return
   fi

   GAME_RULE_FILE_NAME=$(grep -F $1 /usr/share/clash/rules/rules.list |awk -F ',' '{print $3}' 2>/dev/null)

   if [ -z "$GAME_RULE_FILE_NAME" ]; then
      GAME_RULE_FILE_NAME=$(grep -F $1 /usr/share/clash/rules/rules.list |awk -F ',' '{print $2}' 2>/dev/null)
   fi

   GAME_RULE_PATH="/usr/share/clash/rules/g_rules/$GAME_RULE_FILE_NAME"
   sed '/^#/d' $GAME_RULE_PATH 2>/dev/null |sed '/^ *$/d' |awk '{print "- IP-CIDR,"$0}' |awk -v tag="$2" '{print $0","'tag'""}' >> $GAME_RULE_FILE 2>/dev/null
   set_rule_file=1
}


yml_game_rule_get()
{
   local section="$1"
   config_get "group" "$section" "group" ""

   if [ -f $GAME_RULE_FILE ];then
	rm -rf $GAME_RULE_FILE 2>/dev/null
   fi

   if [ -z "$group" ]; then
      return
   fi

   config_list_foreach "$section" "rule_name" get_rule_file "$group"
}



config_load "clash"
config_foreach yml_game_rule_get "game"

if [ -f $GAME_RULE_FILE ];then

sed -i -e "\$a#*******GAME-RULE-END**********#" $GAME_RULE_FILE 2>/dev/null
sed -i '/#*******GAME-RULE-START**********#/,/#*******GAME-RULE-END**********#/d' "$CLASH_CONFIG" 2>/dev/null

if [ ! -z "$(grep "^ \{0,\}- GEOIP" "/etc/clash/config.yaml")" ]; then
   sed -i '1,/^ \{0,\}- GEOIP,/{/^ \{0,\}- GEOIP,/s/^ \{0,\}- GEOIP,/#*******GAME-RULE-START**********#\n&/}' "$CLASH_CONFIG" 2>/dev/null
elif [ ! -z "$(grep "^ \{0,\}- MATCH," "/etc/clash/config.yaml")" ]; then
   sed -i '1,/^ \{0,\}- MATCH,/{/^ \{0,\}- MATCH,/s/^ \{0,\}- MATCH,/#*******GAME-RULE-START**********#\n&/}' "$CLASH_CONFIG" 2>/dev/null
else
   echo "#*******GAME RULE START**********#" >> "$CLASH_CONFIG" 2>/dev/null
fi

sed -i '/GAME-RULE-START/r/tmp/yaml_game_rule_group.yaml' "$CLASH_CONFIG" 2>/dev/null
fi

else
sed -i '/#*******GAME-RULE-START**********#/,/#*******GAME-RULE-END**********#/d' "$CLASH_CONFIG" 2>/dev/null
fi


