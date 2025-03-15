#!/bin/sh

ACTION=$2
MARK_FILE="/tmp/dhcp_mac_mark_mapping"  # 存储MAC和MARK映射关系的文件
LEASE_FILE="/tmp/dhcp.leases"           # DHCP leases 文件
MAX_MARK=31   # 限制mark值在1到31之间

# 定义哈希函数，将MAC地址转化为一个数值，限制在1到31之间
hash_mac() {
    MAC=$1
    # 将MAC地址中的冒号去掉，转化为十六进制数值，取模限制在1到31之间
    MAC_HEX=$(echo "$MAC" | sed 's/://g')
    echo $(( 0x$MAC_HEX % MAX_MARK + 1 ))
}

# 从文件中加载当前的MAC-MARK映射
load_mapping() {
    if [ ! -f "$MARK_FILE" ]; then
        touch "$MARK_FILE"
    fi
    cat "$MARK_FILE"
}

# 检查mark是否被占用
is_mark_in_use() {
    MARK=$1
    grep -q " $MARK$" "$MARK_FILE"
    return $?
}

# 分配一个可用的mark
allocate_mark() {
    MAC=$1
    MARK=$(hash_mac $MAC)
    retries=0  # 记录重试次数
    max_retries=$((MAX_MARK - 1))  # 允许的最大重试次数

    # 循环检查mark是否被占用，直到找到可用的mark
    while is_mark_in_use $MARK; do
        MARK=$((MARK + 1))
        retries=$((retries + 1))
        if [ "$MARK" -gt $MAX_MARK ]; then
            MARK=1
        fi
        # 如果重试次数达到最大值，退出循环，表示无法找到可用的MARK
        if [ "$retries" -ge "$max_retries" ]; then
            MARK=1
            break
        fi
    done
    # 返回可用的MARK
    echo $MARK
}

# 保存MAC和MARK的映射
save_mapping() {
    MAC=$1
    MARK=$2
    echo "$MAC $MARK" >> "$MARK_FILE"
}

# 删除MAC对应的MARK映射
delete_mapping() {
    MAC=$1
    sed -i "/^$MAC /d" "$MARK_FILE"
}

# 处理现有的DHCP记录，确保已有设备保留其MARK
process_existing_leases() {
    while read -r line; do
        IP=$(echo "$line" | awk '{print $3}')
        MAC=$(echo "$line" | awk '{print $2}')
        EXISTING_MARK=$(grep "^$MAC " "$MARK_FILE" | awk '{print $2}')
        if [ -z "$EXISTING_MARK" ]; then
            # 如果没有记录，分配新的MARK
            MARK_VALUE=$(allocate_mark $MAC)
            save_mapping $MAC $MARK_VALUE
        else
            # 使用现有的MARK
            MARK_VALUE=$EXISTING_MARK
        fi
	    idpair=$((MARK_VALUE+32))
        # 添加iptables规则，基于MAC和IP地址给这个设备打上MARK
	iptables -t mangle -D eqos -s $IP -j DSCP --set-dscp ${MARK_VALUE}
	iptables -t mangle -D eqos -d $IP -j DSCP --set-dscp ${idpair}
	ip6tables -t mangle -D eqos -m mac --mac-source $MAC -j MARK --set-mark ${MARK_VALUE}
	ebtables -t nat -D eqos -p ipv6 -d $MAC -j mark --mark-set ${idpair}
	iptables -t mangle -A eqos -s $IP -j DSCP --set-dscp ${MARK_VALUE}
	iptables -t mangle -A eqos -d $IP -j DSCP --set-dscp ${idpair}
	ip6tables -t mangle -A eqos -m mac --mac-source $MAC -j MARK --set-mark ${MARK_VALUE}
	ebtables -t nat -A eqos -A ipv6 -d $MAC -j mark --mark-set ${idpair}
    done < "$LEASE_FILE"
}

if [ "$ACTION" = "init" ]; then
    rm /tmp/dhcp_mac_mark_mapping
    load_mapping
    iptables -t mangle -F eqos
    ip6tables -t mangle -F eqos
    ebtables -t nat  -F eqos
    process_existing_leases
fi

