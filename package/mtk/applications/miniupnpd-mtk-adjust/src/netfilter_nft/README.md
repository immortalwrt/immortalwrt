Miniupnpd nftables support by Tomofumi Hayashi (s1061123@gmail.com).

## Supported Features
- IPv4 NAT/Filter add/del.

## How to build miniupnpd with nftables:
Run 'configure' command with '--firewall=nftables',

`./configure --firewall=nftables && make`

## How to Run
Please run 'netfilter_nft/scripts/nft_init.sh' to add miniupnpd chain.

`sudo ./netfilter_nft/scripts/nft_init.sh`

## FAQ
I will add this section when I get question.
Comments and Questions are welcome ;)

### Custom Chains
NFTables is very flexible but it comes with some restrictions because of that. If there is a second filter chain than all packets that were passed before with the miniupnpd chain will be reevaluated. This also means that if the chain is a drop chain you loose the packets. In that case you really want to use a custom chain and jump to it in your filter chain. miniupnpd should save all accept rules in that custom chain.
For NAT it is the same, a second chain will also evaluate the packets again and therefore it is possible that a second SNAT or DNAT is performed.

The following is used in miniupnpd for a table setup but it can be customized:

    table inet filter {
        chain forward {
            type filter hook forward priority 0;
            policy drop;

            # miniupnpd
            jump miniupnpd

            # Add other rules here
        }

        # miniupnpd
        chain miniupnpd {
        }

        chain prerouting {
            type nat hook prerouting priority -100;
            policy accept;

            # miniupnpd
            jump prerouting_miniupnpd

            # Add other rules here
        }

        chain postrouting {
            type nat hook postrouting priority 100;
            policy accept;

            # miniupnpd
            jump postrouting_miniupnpd

            # Add other rules here
        }

        chain prerouting_miniupnpd {
        }

        chain postrouting_miniupnpd {
        }
    }

and the following config settings can be used to change the tables and chains :

    upnp_table_name=filter
    upnp_nat_table_name=filter
    upnp_forward_chain=miniupnpd
    upnp_nat_chain=prerouting_miniupnpd
    upnp_nat_postrouting_chain=postrouting_miniupnpd

If you need to use the old ipv4 NAT family style set the flag upnp_nftables_family_split to yes.
Default is to use INET family which combines IPv4 and IPv6.
