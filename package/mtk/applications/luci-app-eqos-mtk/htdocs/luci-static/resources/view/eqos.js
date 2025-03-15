'use strict';
'require form';
'require network';
'require uci';
'require view';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('eqos'),
			network.getHostHints()
		]);
	},

	render: function(data) {
		var m, s, o;

		m = new form.Map('eqos', _('EQoS'),
 			_('Network speed control service.(Compatiable with Mediatek HNAT)'));
 			
		s = m.section(form.NamedSection, 'config', 'eqos');

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = o.disabled;
		o.rmempty = false;
		
		o = s.option(form.Value, 'interface', _('Load balance'),
			_('Please set a different gateway hop for each network interface before filling in the network interface name. Fill in PPPOE-WAN for dialing and eth1 for DHCP. Leaving it blank to disable it.Please use commas as interface name separators. Example: pppoe-wan,eth1'));
		o.rmempty = true;
		
		o = s.option(form.Flag, 'ipv6enabled', _('IPV6Enable'));
		o.default = o.disabled;
		o.rmempty = false;
		
		o = s.option(form.Flag, 'smarthqos', _('SMART_HWQOS'),
		_('Enabling fair queue will automatically perform hardware offloading for every local host'));
		o.default = o.disabled;
		o.rmempty = false;
		
		o = s.option(form.Value, 'download', _('Download speed (Mbit/s)'),
			_('Total download bandwidth.'));
		o.datatype = 'and(uinteger,min(1))';
		o.rmempty = false;

		o = s.option(form.Value, 'upload', _('Upload speed (Mbit/s)'),
			_('Total upload bandwidth.'));
		o.datatype = 'and(uinteger,min(1))';
		o.rmempty = false;
		


		s = m.section(form.TableSection, 'device', _('Speed limit and route choose based on IP address(Auto use hardware QOS)'));
		s.addremove = true;
		s.anonymous = true;
		s.sortable = true;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.default = o.enabled;
		
		o = s.option(form.Value, 'ip', _('IPV4 address'));
		o.datatype = 'ip4addr';
		for (var i of Object.entries(data[1]?.hosts))
			for (var v in i[1].ipaddrs)
				if (i[1].ipaddrs[v]) {
					var ip_addr = i[1].ipaddrs[v], ip_host = i[1].name;
					o.value(ip_addr, ip_host ? String.format('%s (%s)', ip_host, ip_addr) : ip_addr)
				}
		o.rmempty = true;
		
		var hosts = data[1]?.hosts;
		o = s.option(form.Value, 'mac', _('IPV6 host'));
		o.datatype = 'macaddr';
		Object.keys(hosts).forEach(function(mac) {
			var hint = hosts[mac].name || L.toArray(hosts[mac].ipaddrs || hosts[mac].ipv4)[0];
			o.value(mac, hint ? '%s (%s)'.format(mac, hint) : mac);
		});
		o.rmempty = true;

		o = s.option(form.Value, 'download', _('Download speed (kbit/s)'));
		o.datatype = 'and(uinteger,min(0))';
		o.rmempty = false;

		o = s.option(form.Value, 'upload', _('Upload speed (kbit/s)'));
		o.datatype = 'and(uinteger,min(0))';
		o.rmempty = false;

		o = s.option(form.Value, 'comment', _('Comment'));
		o.rmempty = true;
		
		o = s.option(form.Value, 'interfacename', _('InterfaceName(start from 0)'));
		o.datatype = 'and(uinteger,min(0))';
		o.rmempty = true;

		return m.render();
	}
});
