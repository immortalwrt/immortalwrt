#!/bin/sh
#
# establish the chains that miniupnpd will update dynamically
#
# 'add' doesn't raise an error if the object already exists. 'create' does.
#

. "$(dirname "$0")/miniupnpd_functions.sh"

$NFT --check list table inet $TABLE > /dev/null 2>&1
if [ $? -eq "0" ]
then
echo "Table $TABLE already exists"
exit 0
fi

echo "Creating nftables structure"

cat > /tmp/miniupnpd.nft <<EOF
table inet $TABLE {
    chain forward {
        type filter hook forward priority 0;
        policy drop;

        # miniupnpd
        jump $CHAIN

        # Add other rules here
    }

    # miniupnpd
    chain $CHAIN {
    }

EOF

if [ "$TABLE" != "$NAT_TABLE" ]
then
cat >> /tmp/miniupnpd.nft <<EOF
}

table inet $NAT_TABLE {
EOF
fi

cat >> /tmp/miniupnpd.nft <<EOF
    chain prerouting {
        type nat hook prerouting priority -100;
        policy accept;

        # miniupnpd
        jump $PREROUTING_CHAIN

        # Add other rules here
    }

    chain postrouting {
        type nat hook postrouting priority 100;
        policy accept;

        # miniupnpd
        jump $POSTROUTING_CHAIN

        # Add other rules here
    }

    chain $PREROUTING_CHAIN {
    }

    chain $POSTROUTING_CHAIN {
    }
}
EOF

$NFT -f /tmp/miniupnpd.nft
