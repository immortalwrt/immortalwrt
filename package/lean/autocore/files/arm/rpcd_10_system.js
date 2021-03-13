'use strict';
'require baseclass';
'require fs';
'require rpc';

var callSystemBoard = rpc.declare({
	object: 'system',
	method: 'board'
});

var callSystemInfo = rpc.declare({
	object: 'system',
	method: 'info'
});

var callCPUBench = rpc.declare({
	object: 'luci',
	method: 'getCPUBench'
});

var callCPUInfo = rpc.declare({
	object: 'luci',
	method: 'getCPUInfo'
});

var callCPUUsage = rpc.declare({
	object: 'luci',
	method: 'getCPUUsage'
});

return baseclass.extend({
	title: _('System'),

	load: function() {
		return Promise.all([
			L.resolveDefault(callSystemBoard(), {}),
			L.resolveDefault(callSystemInfo(), {}),
			L.resolveDefault(callCPUBench(), {}),
			L.resolveDefault(callCPUInfo(), {}),
			L.resolveDefault(callCPUUsage(), {}),
			fs.lines('/usr/lib/lua/luci/version.lua')
		]);
	},

	render: function(data) {
		var boardinfo   = data[0],
		    systeminfo  = data[1],
		    cpubench    = data[2],
		    cpuinfo     = data[3],
		    cpuusage    = data[4],
		    luciversion = data[5];

		luciversion = luciversion.filter(function(l) {
			return l.match(/^\s*(luciname|luciversion)\s*=/);
		}).map(function(l) {
			return l.replace(/^\s*\w+\s*=\s*['"]([^'"]+)['"].*$/, '$1');
		}).join(' ');

		var datestr = null;

		if (systeminfo.localtime) {
			var date = new Date(systeminfo.localtime * 1000);

			datestr = '%04d-%02d-%02d %02d:%02d:%02d'.format(
				date.getUTCFullYear(),
				date.getUTCMonth() + 1,
				date.getUTCDate(),
				date.getUTCHours(),
				date.getUTCMinutes(),
				date.getUTCSeconds()
			);
		}

		var fields = [
			_('Hostname'),         boardinfo.hostname,
			_('Model'),            boardinfo.model + cpubench.cpubench,
			_('Architecture'),     cpuinfo.cpuinfo,
			_('Firmware Version'), (L.isObject(boardinfo.release) ? boardinfo.release.description + ' / ' : '') + (luciversion || ''),
			_('Kernel Version'),   boardinfo.kernel,
			_('Local Time'),       datestr,
			_('Uptime'),           systeminfo.uptime ? '%t'.format(systeminfo.uptime) : null,
			_('Load Average'),     Array.isArray(systeminfo.load) ? '%.2f, %.2f, %.2f'.format(
				systeminfo.load[0] / 65535.0,
				systeminfo.load[1] / 65535.0,
				systeminfo.load[2] / 65535.0
			) : null,
			_('CPU usage (%)'),    cpuusage.cpuusage
		];

		var table = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
			]));
		}

		return table;
	}
});
