/*************************************************************************
 *
 * Copyright (C) 2018-2020 Ruilin Peng (Nick) <pymumu@gmail.com>.
 *
 * smartdns is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * smartdns is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


'use strict';
'require fs';
'require uci';
'require form';
'require rpc';

var conf = 'smartdns';
var callServiceList = rpc.declare({
	object: 'service',
	method: 'list',
	params: ['name'],
	expect: { '': {} }
});

function getPidOfSmartdns() {
	return L.resolveDefault(callServiceList(conf), {})
		.then(function (res) {
			var isrunning = false;
			try {
				isrunning = res[conf]['instances']['smartdns']['running'];
			} catch (e) { }
			return isrunning;
		});
}

function getIPTablesRedirect() {
	return fs.exec('/usr/sbin/iptables', ['-t', 'nat', '-nL', 'PREROUTING']).then(function (res) {
		if (res.code === 0) {
			return res.stdout.trim();
		} else {
			return "";
		}
	});
}

function getIP6TablesRedirect() {
	return fs.exec('/usr/sbin/ip6tables', ['-t', 'nat', '-nL', 'PREROUTING']).then(function (res) {
		if (res.code === 0) {
			return res.stdout.trim();
		} else {
			return "";
		}
	});
}

function smartdnsServiceStatus() {
	return Promise.all([
		getPidOfSmartdns(),
		getIPTablesRedirect(),
		getIP6TablesRedirect()
	]);
}

function smartdnsRenderStatus(res) {
	var renderHTML = "";
	var isRunning = res[0];
	var ipt = res[1];
	var ip6t = res[2];

	var serverPort = uci.get_first('smartdns', 'smartdns', 'port');
	var redirectMode = uci.get_first('smartdns', 'smartdns', 'redirect');
	var ipv6Enabled = uci.get_first('smartdns', 'smartdns', 'ipv6_server');

	if (isRunning) {
		renderHTML += "<span style=\"color:green;font-weight:bold\">SmartDNS - " + _("RUNNING") + "</span>";
	} else {
		renderHTML += "<span style=\"color:red;font-weight:bold\">SmartDNS - " + _("NOT RUNNING") + "</span>";
		return renderHTML;
	}

	if (redirectMode === "dnsmasq-upstream") {
		var matchLine = "127.0.0.1#" + serverPort;
		var dnsmasqServer = uci.get_first('dhcp', 'dnsmasq', 'server') || "";

		if (dnsmasqServer.indexOf(matchLine) < 0) {
			renderHTML += "<br /><span style=\"color:red;font-weight:bold\">" + _("Dnsmasq Forwared To Smartdns Failure") + "</span>";
		}
	} else if (redirectMode === "redirect") {
		var redirectRules = (ipt || '').split(/\n/).filter(function (rule) {
			return rule.match(/REDIRECT/) && rule.match(/dpt:53/) && rule.match("ports " + serverPort);
		});

		if (redirectRules.length <= 0) {
			renderHTML += "<br /><span style=\"color:red;font-weight:bold\">" + _("IPV4 53 Port Redirect Failure") + "</span>";
			if (ipv6Enabled) {
				var redirectRules = (ip6t || '').split(/\n/).filter(function (rule) {
					return rule.match(/REDIRECT/) && rule.match(/dpt:53/) && rule.match("ports " + serverPort);
				});
				if (redirectRules.length <= 0) {
					renderHTML += "<br /><span style=\"color:red;font-weight:bold\">" + _("IPV6 53 Port Redirect Failure") + "</span>";
				}
			}
		}
	}

	return renderHTML;
}

return L.view.extend({
	load: function () {
		return Promise.all([
			uci.load('smartdns'),
			uci.load('dhcp')
		]);
	},
	render: function (stats) {
		var m, s, o;

		m = new form.Map('smartdns', _('SmartDNS'));
		m.title = _("SmartDNS Server");
		m.description = _("SmartDNS is a local high-performance DNS server, supports finding fastest IP, "
			+ "supports ad filtering, and supports avoiding DNS poisoning.");

		s = m.section(form.NamedSection, '_status');
		s.anonymous = true;
		s.render = function (section_id) {
			L.Poll.add(function () {
				return L.resolveDefault(smartdnsServiceStatus()).then(function (res) {
					var view = document.getElementById("service_status");
					view.innerHTML = smartdnsRenderStatus(res);
				});
			});

			return E('div', { class: 'cbi-map' },
				E('div', { class: 'cbi-section' }, [
					E('div', { id: 'service_status' },
						_('Collecting data ...'))
				])
			);
		}

		// Basic;
		s = m.section(form.TypedSection, "smartdns", _("Settings"), _("General Settings"));
		s.anonymous = true;

		s.tab("settings", _("General Settings"));
		s.tab("seconddns", _("Second Server Settings"));
		s.tab("custom", _("Custom Settings"));

		// Eanble;
		o = s.taboption("settings", form.Flag, "enabled", _("Enable"), _("Enable or disable smartdns server"));
		o.default = o.disabled;
		o.rempty = false;

		// server name;
		o = s.taboption("settings", form.Value, "server_name", _("Server Name"), _("Smartdns server name"));
		o.default = "smartdns";
		o.datatype = "hostname";
		o.rempty = false;

		// Port;
		o = s.taboption("settings", form.Value, "port", _("Local Port"), _("Smartdns local server port"));
		o.placeholder = 6053;
		o.default = 6053;
		o.datatype = "port";
		o.rempty = false;

		// Enable TCP server;
		o = s.taboption("settings", form.Flag, "tcp_server", _("TCP Server"), _("Enable TCP DNS Server"));
		o.rmempty = false;
		o.default = o.enabled;

		// Support IPV6;
		o = s.taboption("settings", form.Flag, "ipv6_server", _("IPV6 Server"), _("Enable IPV6 DNS Server"));
		o.rmempty = false;
		o.default = o.enabled;

		// Support DualStack ip selection;
		o = s.taboption("settings", form.Flag, "dualstack_ip_selection", _("Dual-stack IP Selection"),
			_("Enable IP selection between IPV4 and IPV6"));
		o.rmempty = false;
		o.default = o.disabled;

		// Domain prefetch load ;
		o = s.taboption("settings", form.Flag, "prefetch_domain", _("Domain prefetch"),
			_("Enable domain prefetch, accelerate domain response speed."));
		o.rmempty = false;
		o.default = o.disabled;

		// Domain Serve expired
		o = s.taboption("settings", form.Flag, "serve_expired", _("Serve expired"),
			_("Attempts to serve old responses from cache with a TTL of 0 in the response without waiting for the actual resolution to finish."));
		o.rmempty = false;
		o.default = o.disabled;

		// Redirect;
		o = s.taboption("settings", form.ListValue, "redirect", _("Redirect"), _("SmartDNS redirect mode"));
		o.placeholder = "none";
		o.value("none", _("none"));
		o.value("dnsmasq-upstream", _("Run as dnsmasq upstream server"));
		o.value("redirect", _("Redirect 53 port to SmartDNS"));
		o.default = "none";
		o.rempty = false;

		// cache-size;
		o = s.taboption("settings", form.Value, "cache_size", _("Cache Size"), _("DNS domain result cache size"));
		o.rempty = true;

		// rr-ttl;
		o = s.taboption("settings", form.Value, "rr_ttl", _("Domain TTL"), _("TTL for all domain result."));
		o.rempty = true;

		// rr-ttl-min;
		o = s.taboption("settings", form.Value, "rr_ttl_min", _("Domain TTL Min"),
			_("Minimum TTL for all domain result."));
		o.rempty = true;
		o.placeholder = "300";
		o.default = 300;
		o.optional = true;

		// second dns server;
		// rr-ttl-max;
		o = s.taboption("settings", form.Value, "rr_ttl_max", _("Domain TTL Max"),
			_("Maximum TTL for all domain result."));
		o.rempty = true;

		// Eanble;
		o = s.taboption("seconddns", form.Flag, "seconddns_enabled", _("Enable"),
			_("Enable or disable second DNS server."));
		o.default = o.disabled;
		o.rempty = false;

		// Port;
		o = s.taboption("seconddns", form.Value, "seconddns_port", _("Local Port"), _("Smartdns local server port"));
		o.placeholder = 6553;
		o.default = 6553;
		o.datatype = "port";
		o.rempty = false;

		// Enable TCP server;
		o = s.taboption("seconddns", form.Flag, "seconddns_tcp_server", _("TCP Server"), _("Enable TCP DNS Server"));
		o.rmempty = false;
		o.default = o.enabled;

		// dns server group;
		o = s.taboption("seconddns", form.Value, "seconddns_server_group", _("Server Group"),
			_("Query DNS through specific dns server group, such as office, home."));
		o.rmempty = true;
		o.placeholder = "default";
		o.datatype = "hostname";
		o.rempty = true;

		o = s.taboption("seconddns", form.Flag, "seconddns_no_speed_check", _("Skip Speed Check"),
			_("Do not check speed."));
		o.rmempty = false;
		o.default = o.disabled;

		// skip address rules;
		o = s.taboption("seconddns", form.Flag, "seconddns_no_rule_addr", _("Skip Address Rules"),
			_("Skip address rules."));
		o.rmempty = false;
		o.default = o.disabled;

		// skip name server rules;
		o = s.taboption("seconddns", form.Flag, "seconddns_no_rule_nameserver", _("Skip Nameserver Rule"),
			_("Skip nameserver rules."));
		o.rmempty = false;
		o.default = o.disabled;

		// skip ipset rules;
		o = s.taboption("seconddns", form.Flag, "seconddns_no_rule_ipset", _("Skip Ipset Rule"),
			_("Skip ipset rules."));
		o.rmempty = false;
		o.default = o.disabled;

		// skip soa address rule;
		o = s.taboption("seconddns", form.Flag, "seconddns_no_rule_soa", _("Skip SOA Address Rule"),
			_("Skip SOA address rules."));
		o.rmempty = false;
		o.default = o.disabled;

		o = s.taboption("seconddns", form.Flag, "seconddns_no_dualstack_selection", _("Skip Dualstack Selection"),
			_("Skip Dualstack Selection."));
		o.rmempty = false;
		o.default = o.disabled;

		// skip cache;
		o = s.taboption("seconddns", form.Flag, "seconddns_no_cache", _("Skip Cache"), _("Skip Cache."));
		o.rmempty = false;
		o.default = o.disabled;

		// Force AAAA SOA
		o = s.taboption("seconddns", form.Flag, "force_aaaa_soa", _("Force AAAA SOA"), _("Force AAAA SOA."));
		o.rmempty = false;
		o.default = o.disabled;

		// custom settings;
		o = s.taboption("custom", form.TextValue, "custom_conf",
			"", _("smartdns custom settings"));

		o.rows = 20;
		o.cfgvalue = function (section_id) {
			return fs.trimmed('/etc/smartdns/custom.conf');
		};
		o.write = function (section_id, formvalue) {
			return fs.write('/etc/smartdns/custom.conf', formvalue.trim().replace(/\r\n/g, '\n') + '\n');
		};

		o = s.taboption("custom", form.Flag, "coredump", _("Generate Coredump"),
			_("Generate Coredump file when smartdns crash, coredump file is located at /tmp/smartdns.xxx.core."));
		o.rmempty = false;
		o.default = o.disabled;
		// Upstream servers;
		s = m.section(form.GridSection, "server", _("Upstream Servers"),
			_("Upstream Servers, support UDP, TCP protocol. Please configure multiple DNS servers, "
				+ "including multiple foreign DNS servers."));
		s.anonymous = true;
		s.addremove = true;

		s.tab('general', _('General Settings'));
		s.tab('advanced', _('Advanced Settings'));

		// enable flag;
		o = s.taboption("general", form.Flag, "enabled", _("Enable"), _("Enable"));
		o.rmempty = false;
		o.default = o.enabled;
		o.editable = true;

		// name;
		o = s.taboption("general", form.Value, "name", _("DNS Server Name"), _("DNS Server Name"));

		// IP address;
		o = s.taboption("general", form.Value, "ip", _("ip"), _("DNS Server ip"));
		o.datatype = "or(ipaddr, string)";
		o.rmempty = false;

		// port;
		o = s.taboption("general", form.Value, "port", _("port"), _("DNS Server port"));
		o.placeholder = "default";
		o.datatype = "port";
		o.rempty = true;
		o.depends("type", "udp");
		o.depends("type", "tcp");
		o.depends("type", "tls");

		// type;
		o = s.taboption("general", form.ListValue, "type", _("type"), _("DNS Server type"));
		o.placeholder = "udp";
		o.value("udp", _("udp"));
		o.value("tcp", _("tcp"));
		o.value("tls", _("tls"));
		o.value("https", _("https"));
		o.default = "udp";
		o.rempty = false;

		// Advanced Options
		// server group
		o = s.taboption("advanced", form.Value, "server_group", _("Server Group"), _("DNS Server group belongs to, "
			+ "used with nameserver, such as office, home."))
		o.rmempty = true
		o.placeholder = "default"
		o.datatype = "hostname"
		o.rempty = true
		o.modalonly = true;

		// blacklist_ip
		o = s.taboption("advanced", form.Flag, "blacklist_ip", _("IP Blacklist Filtering"),
			_("Filtering IP with blacklist"))
		o.rmempty = false
		o.default = o.disabled
		o.modalonly = true;

		// TLS host verify
		o = s.taboption("advanced", form.Value, "tls_host_verify", _("TLS Hostname Verify"),
			_("Set TLS hostname to verify."))
		o.default = ""
		o.datatype = "string"
		o.rempty = true
		o.modalonly = true;
		o.depends("type", "tls")
		o.depends("type", "https")

		// certificate verify
		o = s.taboption("advanced", form.Flag, "no_check_certificate", _("No check certificate"),
			_("Do not check certificate."))
		o.rmempty = false
		o.default = o.disabled
		o.modalonly = true;
		o.depends("type", "tls")
		o.depends("type", "https")

		// SNI host name
		o = s.taboption("advanced", form.Value, "host_name", _("TLS SNI name"),
			_("Sets the server name indication for query."))
		o.default = ""
		o.datatype = "hostname"
		o.rempty = true
		o.modalonly = true;
		o.depends("type", "tls")
		o.depends("type", "https")

		// http host
		o = s.taboption("advanced", form.Value, "http_host", _("HTTP Host"),
			_("Set the HTTP host used for the query. Use this parameter when the host of the URL address is an IP address."))
		o.default = ""
		o.datatype = "hostname"
		o.rempty = true
		o.modalonly = true;
		o.depends("type", "https")

		// SPKI pin
		o = s.taboption("advanced", form.Value, "spki_pin", _("TLS SPKI Pinning"),
			_("Used to verify the validity of the TLS server, The value is Base64 encoded SPKI fingerprint, "
				+ "leaving blank to indicate that the validity of TLS is not verified."))
		o.default = ""
		o.datatype = "string"
		o.rempty = true
		o.modalonly = true;
		o.depends("type", "tls")
		o.depends("type", "https")

		// other args
		o = s.taboption("advanced", form.Value, "addition_arg", _("Additional Server Args"),
			_("Additional Args for upstream dns servers"))
		o.default = ""
		o.rempty = true
		o.modalonly = true;

		// Doman addresss;
		s = m.section(form.TypedSection, "smartdns", _("Advanced Settings"), _("Advanced Settings"));
		s.anonymous = true;

		s.tab("domain-address", _("Domain Address"), _("Set Specific domain ip address."));
		s.tab("blackip-list", _("IP Blacklist"), _("Set Specific ip blacklist."));

		o = s.taboption("domain-address", form.TextValue, "address_conf",
			"",
			_("Specify an IP address to return for any host in the given domains, Queries in the domains are never "
				+ "forwarded and always replied to with the specified IP address which may be IPv4 or IPv6."));
		o.rows = 20;
		o.cfgvalue = function (section_id) {
			return fs.trimmed('/etc/smartdns/address.conf');
		};
		o.write = function (section_id, formvalue) {
			return fs.write('/etc/smartdns/address.conf', formvalue.trim().replace(/\r\n/g, '\n') + '\n');
		};

		// IP Blacklist;
		// blacklist;
		o = s.taboption("blackip-list", form.TextValue, "blackip_ip_conf",
			"", _("Configure IP blacklists that will be filtered from the results of specific DNS server."));
		o.rows = 20;
		o.cfgvalue = function (section_id) {
			return fs.trimmed('/etc/smartdns/blacklist-ip.conf');
		};
		o.write = function (section_id, formvalue) {
			return fs.write('/etc/smartdns/blacklist-ip.conf', formvalue.trim().replace(/\r\n/g, '\n') + '\n');
		};

		// Doman addresss;
		s = m.section(form.TypedSection, "smartdns", _("Technical Support"),
			_("If you like this software, please buy me a cup of coffee."));
		s.anonymous = true;

		o = s.option(form.Button, "web");
		o.title = _("SmartDNS official website");
		o.inputtitle = _("open website");
		o.inputstyle = "apply";
		o.onclick = function () {
			window.open("https://pymumu.github.io/smartdns", '_blank');
		};

		o = s.option(form.Button, "Donate");
		o.title = _("Donate to smartdns");
		o.inputtitle = _("Donate");
		o.inputstyle = "apply";
		o.onclick = function () {
			window.open("https://pymumu.github.io/smartdns/#donate", '_blank');
		};

		return m.render();
	}
});
