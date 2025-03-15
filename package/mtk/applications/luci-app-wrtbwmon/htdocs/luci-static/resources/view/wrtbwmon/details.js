'use strict';
'require dom';
'require fs';
'require poll';
'require rpc';
'require ui';
'require validation';
'require view';

var cachedData = [];
var luciConfig = '/etc/luci-wrtbwmon.conf';
var hostNameFile = '/etc/wrtbwmon.user';
var columns = {
	thClient: _('Clients'),
	thMAC: _('MAC'),
	thDownload: _('Download'),
	thUpload: _('Upload'),
	thTotalDown: _('Total Down'),
	thTotalUp: _('Total Up'),
	thTotal: _('Total'),
	thFirstSeen: _('First Seen'),
	thLastSeen: _('Last Seen')
};

var callLuciDHCPLeases = rpc.declare({
	object: 'luci-rpc',
	method: 'getDHCPLeases',
	expect: { '': {} }
});

var callLuciDSLStatus = rpc.declare({
	object: 'luci-rpc',
	method: 'getDSLStatus',
	expect: { '': {} }
});

var callGetDatabaseRaw = rpc.declare({
	object: 'luci.wrtbwmon',
	method: 'get_db_raw',
	params: [ 'protocol' ]
});

var callGetDatabasePath = rpc.declare({
	object: 'luci.wrtbwmon',
	method: 'get_db_path',
	params: [ 'protocol' ]
});

var callRemoveDatabase = rpc.declare({
	object: 'luci.wrtbwmon',
	method: 'remove_db',
	params: [ 'protocol' ]
});

function $(tid) {
	return document.getElementById(tid);
}

function clickToResetDatabase(settings, table, updated, updating, ev) {
	if (confirm(_('This will delete the database file. Are you sure?'))) {
		return callRemoveDatabase(settings.protocol)
		.then(function() {
			updateData(settings, table, updated, updating, true);
		});
	}
}

function clickToSaveConfig(keylist, cstrs) {
	var data = {};

	for (var i = 0; i < keylist.length; i++) {
		data[keylist[i]] = cstrs[keylist[i]].getValue();
	}

	ui.showModal(_('Configuration'), [
		E('p', { 'class': 'spinning' }, _('Saving configuration data...'))
	]);

	return fs.write(luciConfig, JSON.stringify(data, undefined, '\t') + '\n')
	.catch(function(err) {
		ui.addNotification(null, E('p', {}, [ _('Unable to save %s: %s').format(luciConfig, err) ]));
	})
	.then(ui.hideModal)
	.then(function() { document.location.reload(); });
}

function clickToSelectInterval(settings, updating, ev) {
	if (ev.target.value > 0) {
		settings.interval = parseInt(ev.target.value);
		if (!poll.active()) poll.start();
	}
	else {
		poll.stop();
		setUpdateMessage(updating, -1);
	}
}

function clickToSelectProtocol(settings, table, updated, updating, ev) {
	settings.protocol = ev.target.value;
	updateData(settings, table, updated, updating, true);
}

function createOption(args, val) {
	var cstr = args[0], title = args[1], desc = args.slice(-1), widget, frame;
	widget = args.length == 4 ? new cstr(val, args[2]) : new cstr(val, args[2], args[3]);

	frame = E('div', {'class': 'cbi-value'}, [
		E('label', {'class': 'cbi-value-title'}, title),
		E('div', {'class': 'cbi-value-field'}, E('div', {}, widget.render()))
	]);

	if (desc && desc != '')
		dom.append(frame.lastChild, E('div', { 'class': 'cbi-value-description' }, desc));

	return [widget, frame];
}

function displayTable(tb, settings) {
	var elm, elmID, col, sortedBy, flag, IPVer;

	elm = tb.querySelector('.th.sorted');
	elmID = elm ? elm.id : 'thTotal';
	sortedBy = elm && elm.classList.contains('ascent') ? 'asc' : 'desc';

	col = Object.keys(columns).indexOf(elmID);
	IPVer = col == 0 ? settings.protocol : null;
	flag = sortedBy == 'desc' ? 1 : -1;

	cachedData[0].sort(sortTable.bind(this, col, IPVer, flag));

	//console.time('show');
	updateTable(tb, cachedData, '<em>%s</em>'.format(_('Collecting data...')), settings);
	//console.timeEnd('show');
	progressbar('downstream', cachedData[1][0], settings.downstream, settings.useBits, settings.useMultiple);
	progressbar('upstream', cachedData[1][1], settings.upstream, settings.useBits, settings.useMultiple);
}

function formatSize(size, useBits, useMultiple) {
	// String.format automatically adds the i for KiB if the multiple is 1024
	return String.format('%%%s.2m%s'.format(useMultiple, (useBits ? 'bit' : 'B')), useBits ? size * 8 : size);
}

function formatSpeed(speed, useBits, useMultiple) {
	return formatSize(speed, useBits, useMultiple) + '/s';
}

function formatDate(d) {
	var Y = d.getFullYear(), M = d.getMonth() + 1, D = d.getDate(),
	    hh = d.getHours(), mm = d.getMinutes(), ss = d.getSeconds();
	return '%04d/%02d/%02d %02d:%02d:%02d'.format(Y, M, D, hh, mm, ss);
}

function getDSLBandwidth() {
	return callLuciDSLStatus().then(function(res) {
		return {
			upstream : res.max_data_rate_up || null,
			downstream : res.max_data_rate_down || null
		};
	});
}

function handleConfig(ev) {
	ui.showModal(_('Configuration'), [
			E('p', { 'class': 'spinning' }, _('Loading configuration data...'))
	]);

	parseDefaultSettings(luciConfig)
	.then(function(settings) {
		var arglist, keylist = Object.keys(settings), res, cstrs = {}, node = [], body;

		arglist = [
			[ui.Select, _('Default Protocol'), {'ipv4': _('ipv4'), 'ipv6': _('ipv6')}, {}, ''],
			[ui.Select, _('Default Refresh Interval'), {'-1': _('Disabled'), '3': _('3 seconds'), '5': _('5 seconds'), '10': _('10 seconds'), '30': _('30 seconds')}, {sort: ['-1', '3', '5', '10', '30']}, ''],
			[ui.Dropdown, _('Default Columns'), columns, {multiple: true, sort: false, custom_placeholder: '', dropdown_items: 3}, ''],
			[ui.Checkbox, _('Show Zeros'), {value_enabled: true, value_disabled: false}, ''],
			[ui.Checkbox, _('Transfer Speed in Bits'), {value_enabled: true, value_disabled: false}, ''],
			[ui.Select, _('Multiple of Unit'), {'1000': _('SI - 1000'), '1024': _('IEC - 1024')}, {}, ''],
			[ui.Checkbox, _('Use DSL Bandwidth'), {value_enabled: true, value_disabled: false}, ''],
			[ui.Textfield, _('Upstream Bandwidth'), {datatype: 'ufloat'}, 'Mbps'],
			[ui.Textfield, _('Downstream Bandwidth'), {datatype: 'ufloat'}, 'Mbps'],
			[ui.DynamicList, _('Hide MAC Addresses'), '', {datatype: 'macaddr'}, '']
		]; // [constructor, label(, all_choices), options, description]

		for (var i = 0; i < keylist.length; i++) {
			res = createOption(arglist[i], settings[keylist[i]]);
			cstrs[keylist[i]] = res[0];
			node.push(res[1]);
		}

		body = [
			E('p', {}, _('Configure the default values for luci-app-wrtbwmon.')),
			E('div', {}, node),
			E('div', { 'class': 'right' }, [
				E('div', {
					'class': 'btn cbi-button-neutral',
					'click': ui.hideModal
				}, _('Cancel')),
				' ',
				E('div', {
					'class': 'btn cbi-button-positive',
					'click': clickToSaveConfig.bind(this, keylist, cstrs),
					'disabled': (L.hasViewPermission ? !L.hasViewPermission() : null) || null
				}, _('Save'))
			])
		];
		ui.showModal(_('Configuration'), body);
	})
}

function loadCss(path) {
	var head = document.head || document.getElementsByTagName('head')[0];
	var link = E('link', {
		'rel': 'stylesheet',
		'href': path,
		'type': 'text/css'
	});

	head.appendChild(link);
}

function parseDatabase(raw, hosts, showZero, hideMACs) {
	var values = [],
	    totals = [0, 0, 0, 0, 0],
	    rows = raw.trim().split(/\r?\n|\r/g),
	    rowIndex = [1, 0, 3, 4, 5, 6, 7, 8, 9, 0];

	rows.shift();

	for (var i = 0; i < rows.length; i++) {
		var row = rows[i].split(',');
		if ((!showZero && row[7] == 0) || hideMACs.indexOf(row[0]) >= 0) continue;

		for (var j = 0; j < totals.length; j++) {
			totals[j] += parseInt(row[3 + j]);
		}

		var newRow = rowIndex.map(function(i) { return row[i] });
		if (newRow[1].toLowerCase() in hosts) {
			newRow[9] = hosts[newRow[1].toLowerCase()];
		}
		values.push(newRow);
	}

	return [values, totals];
}

function parseDefaultSettings(file) {
	var defaultColumns = ['thClient', 'thDownload', 'thUpload', 'thTotalDown', 'thTotalUp', 'thTotal'],
	    keylist = ['protocol', 'interval', 'showColumns', 'showZero', 'useBits', 'useMultiple', 'useDSL', 'upstream', 'downstream', 'hideMACs'],
	    valuelist = ['ipv4', '5', defaultColumns, true, false, '1000', false, '100', '100', []];

	return fs.read_direct(file, 'json').then(function(oldSettings) {
		var settings = {};
		for (var i = 0; i < keylist.length; i++) {
			if (!(keylist[i] in oldSettings))
				settings[keylist[i]] = valuelist[i];
			else
				settings[keylist[i]] = oldSettings[keylist[i]];
		}

		if (settings.useDSL) {
			return getDSLBandwidth().then(function(dsl) {
				for (var s in dsl)
					settings[s] = dsl[s];
				return settings;
			});
		}
		else {
			return settings;
		}
	})
	.catch(function() { return {} });
}

function progressbar(query, v, m, useBits, useMultiple) {
	// v = B/s, m = Mb/s
	var pg = $(query),
	    vn = (v * 8) || 0,
	    mn = (m || 100) * Math.pow(1000, 2),
	    fv = formatSpeed(v, useBits, useMultiple),
	    pc = '%.2f'.format((100 / mn) * vn),
	    wt = Math.floor(pc > 100 ? 100 : pc),
	    bgc = (pc >= 95 ? 'red' : (pc >= 80 ? 'darkorange' : (pc >= 60 ? 'yellow' : 'lime')));
	if (pg) {
		pg.firstElementChild.style.width = wt + '%';
		pg.firstElementChild.style.background = bgc;
		pg.setAttribute('title', '%s (%f%%)'.format(fv, pc));
	}
}

function setupThisDOM(settings, table) {
	document.addEventListener('poll-stop', function() {
		$('selectInterval').value = -1;
	});

	document.addEventListener('poll-start', function() {
		$('selectInterval').value = settings.interval;
	});

	table.querySelectorAll('.th').forEach(function(e) {
		if (e) {
			e.addEventListener('click', function (ev) {
				setSortedColumn(ev.target);
				displayTable(table, settings);
			});

			if (settings.showColumns.indexOf(e.id) >= 0)
				e.classList.remove('hide');
			else
				e.classList.add('hide');

		}
	});
}

function renameFile(str, tag) {
	var n = str.lastIndexOf('/'), fn = n > -1 ? str.slice(n + 1) : str, dir = n > -1 ? str.slice(0, n + 1) : '';
	var n = fn.lastIndexOf('.'), bn = n > -1 ? fn.slice(0, n) : fn;
	var n = fn.lastIndexOf('.'), en = n > -1 ? fn.slice(n + 1) : '';
	return dir + bn + '.' + tag + (en ? '.' + en : '');
}

function resolveCustomizedHostName() {
	return fs.stat(hostNameFile).then(function() {
		return fs.read_direct(hostNameFile).then(function(raw) {
			var arr = raw.trim().split(/\r?\n/), hosts = {}, row;
			for (var i = 0; i < arr.length; i++) {
				row = arr[i].split(',');
				if (row.length == 2 && row[0])
					hosts[row[0].toLowerCase()] = row[1];
			}
			return hosts;
		})
	})
	.catch(function() { return []; });
}

function resolveHostNameByMACAddr() {
	return Promise.all([
		resolveCustomizedHostName(),
		callLuciDHCPLeases()
	]).then(function(res) {
		var hosts = res[0];
		for (var key in res[1]) {
			var leases = Array.isArray(res[1][key]) ? res[1][key] : [];
			for (var i = 0; i < leases.length; i++) {
				if(leases[i].macaddr) {
					var macaddr = leases[i].macaddr.toLowerCase();
					if (!(macaddr in hosts) && Boolean(leases[i].hostname))
						hosts[macaddr] = leases[i].hostname;
				}
			}
		}
		return hosts;
	});
}

function setSortedColumn(sorting) {
	var sorted = document.querySelector('.th.sorted') || $('thTotal');

	if (sorting.isSameNode(sorted)) {
		sorting.classList.toggle('ascent');
	}
	else {
		sorting.classList.add('sorted');
		sorted.classList.remove('sorted', 'ascent');
	}
}

function setUpdateMessage(e, sec) {
	e.innerHTML = sec < 0 ? '' : _('Updating again in %s second(s).').format('<b>' + sec + '</b>');
}

function sortTable(col, IPVer, flag, x, y) {
	var byCol = x[col] == y[col] ? 1 : col;
	var a = x[byCol], b = y[byCol];

	if (!IPVer || byCol != 0) {
		if (!(a.match(/\D/g) || b.match(/\D/g)))
			a = parseInt(a), b = parseInt(b);
	}
	else {
		IPVer == 'ipv4'
		? (a = validation.parseIPv4(a) || [0, 0, 0, 0], b = validation.parseIPv4(b) || [0, 0, 0, 0])
		: (a = validation.parseIPv6(a) || [0, 0, 0, 0, 0, 0, 0, 0], b = validation.parseIPv6(b) || [0, 0, 0, 0, 0, 0, 0, 0]);
	}

	if (Array.isArray(a) && Array.isArray(b)) {
		for (var i = 0; i < a.length; i++) {
			if (a[i] != b[i]) {
				return (b[i] - a[i]) * flag;
			}
		}
		return 0;
	}

	return a == b ? 0 : (a < b ? 1 : -1) * flag;
}

function updateData(settings, table, updated, updating, once) {
	var tick = poll.tick,
	    interval = settings.interval,
	    sec = (interval - tick % interval) % interval;
	if (!sec || once) {
		callGetDatabasePath()
		.then(function(res) {
			var params = settings.protocol == 'ipv4' ? '-4' : '-6';
			return fs.exec_direct('/usr/sbin/wrtbwmon', [params, '-f', res.file_4])
		})
		.then(function() {
			return Promise.all([
				callGetDatabaseRaw(settings.protocol),
				resolveHostNameByMACAddr()
			]);
		})
		.then(function(res) {
			//console.time('start');
			cachedData = parseDatabase(res[0].data || '', res[1], settings.showZero, settings.hideMACs);
			displayTable(table, settings);
			updated.textContent = _('Last updated at %s.').format(formatDate(new Date(document.lastModified)));
			//console.timeEnd('start');
		});
	}

	setUpdateMessage(updating, sec);
	if (!sec)
		setTimeout(setUpdateMessage.bind(this, updating, interval), 100);
}

function updateTable(tb, values, placeholder, settings) {
	var fragment = document.createDocumentFragment(), nodeLen = tb.childElementCount - 2;
	var formData = values[0], tbTitle = tb.firstElementChild, newNode, childTD;

	// Update the table data.
	for (var i = 0; i < formData.length; i++) {
		if (i < nodeLen) {
			newNode = tbTitle.nextElementSibling;
		}
		else {
			if (nodeLen > 0) {
				newNode = fragment.firstChild.cloneNode(true);
			}
			else {
				newNode = document.createElement('tr');
				childTD = document.createElement('td');
				for (var j = 0; j < tbTitle.children.length; j++) {
					childTD.className = 'td' + (settings.showColumns.indexOf(tbTitle.children[j].id) >= 0 ? '' : ' hide');
					childTD.setAttribute('data-title', tbTitle.children[j].textContent);
					newNode.appendChild(childTD.cloneNode(true));
				}
			}
			newNode.className = 'tr cbi-rowstyle-%d'.format(i % 2 ? 2 : 1);
		}

		childTD = newNode.firstElementChild;
		childTD.title = formData[i].slice(-1);
		for (var j = 0; j < tbTitle.childElementCount; j++, childTD = childTD.nextElementSibling) {
			switch (j) {
				case 2:
				case 3:
					childTD.textContent = formatSpeed(formData[i][j], settings.useBits, settings.useMultiple);
					break;
				case 4:
				case 5:
				case 6:
					childTD.textContent = formatSize(formData[i][j], settings.useBits, settings.useMultiple);
					break;
				case 7:
				case 8:
					childTD.textContent = formatDate(new Date(formData[i][j] * 1000));
					break;
				default:
					childTD.textContent = formData[i][j];
			}
		}
		fragment.appendChild(newNode);
	}

	// Remove the table data which has been deleted from the database.
	while (tb.childElementCount > 1) {
		tb.removeChild(tbTitle.nextElementSibling);
	}

	//Append the totals or placeholder row.
	if (formData.length == 0) {
		newNode = document.createElement('tr');
		newNode.className = 'tr placeholder';
		childTD = document.createElement('td');
		childTD.className = 'td';
		childTD.innerHTML = placeholder;
		newNode.appendChild(childTD);
	}
	else{
		newNode = fragment.firstChild.cloneNode(true);
		newNode.className = 'tr table-totals';

		newNode.children[0].textContent = _('TOTAL') + (settings.showColumns.indexOf('thMAC') >= 0 ? '' : ': ' + formData.length);
		newNode.children[1].textContent = formData.length + ' ' + _('Clients');

		for (var j = 0; j < tbTitle.childElementCount; j++) {
			switch(j) {
				case 0:
				case 1:
					newNode.children[j].removeAttribute('title');
					newNode.children[j].style.fontWeight = 'bold';
					break;
				case 2:
				case 3:
					newNode.children[j].textContent = formatSpeed(values[1][j - 2], settings.useBits, settings.useMultiple);
					break;
				case 4:
				case 5:
				case 6:
					newNode.children[j].textContent = formatSize(values[1][j - 2], settings.useBits, settings.useMultiple);
					break;
				default:
					newNode.children[j].textContent = '';
					newNode.children[j].removeAttribute('data-title');
			}
		}
	}

	fragment.appendChild(newNode);
	tb.appendChild(fragment);
}

function initOption(options, selected) {
	var res = [], attr = {};
	for (var idx in options) {
		attr.value = idx;
		attr.selected = idx == selected ? '' : null;
		res.push(E('option', attr, options[idx]));
	}
	return res;
}

return view.extend({
	load: function() {
		return Promise.all([
			parseDefaultSettings(luciConfig),
			loadCss(L.resource('view/wrtbwmon/wrtbwmon.css'))
		]);
	},

	render: function(data) {
		var settings = data[0],
		    labelUpdated = E('label'),
		    labelUpdating = E('label'),
		    table = E('table', { 'class': 'table', 'id': 'traffic' }, [
					E('tr', { 'class': 'tr table-titles' }, [
						E('th', { 'class': 'th', 'id': 'thClient' }, _('Clients')),
						E('th', { 'class': 'th hide', 'id': 'thMAC' }, _('MAC')),
						E('th', { 'class': 'th', 'id': 'thDownload' }, _('Download')),
						E('th', { 'class': 'th', 'id': 'thUpload' }, _('Upload')),
						E('th', { 'class': 'th', 'id': 'thTotalDown' }, _('Total Down')),
						E('th', { 'class': 'th', 'id': 'thTotalUp' }, _('Total Up')),
						E('th', { 'class': 'th sorted', 'id': 'thTotal' }, _('Total')),
						E('th', { 'class': 'th hide', 'id': 'thFirstSeen' }, _('First Seen')),
						E('th', { 'class': 'th hide', 'id': 'thLastSeen' }, _('Last Seen'))
					]),
					E('tr', {'class': 'tr placeholder'}, [
						E('td', { 'class': 'td' }, E('em', {}, _('Collecting data...')))
					])
				]);

		poll.add(updateData.bind(this, settings, table, labelUpdated, labelUpdating, false), 1);
		setupThisDOM(settings, table);
		return E('div', { 'class': 'cbi-map' }, [
			E('h2', {}, _('Usage - Details')),
			E('div', { 'class': 'cbi-section' }, [
				E('div', { 'id': 'control_panel' }, [
					E('div', {}, [
						E('label', {}, _('Protocol:')),
						E('select', {
							'id': 'selectProtocol',
							'change': clickToSelectProtocol.bind(this, settings, table, labelUpdated, labelUpdating)
							}, initOption({
								'ipv4': 'ipv4',
								'ipv6': 'ipv6'
								}, settings.protocol))
					]),
					E('div', {}, [
						E('button', {
							'class': 'btn cbi-button cbi-button-reset important',
							'id': 'resetDatabase',
							'click': clickToResetDatabase.bind(this, settings, table, labelUpdated, labelUpdating)
						}, _('Reset Database')),
						' ',
						E('button', {
							'class': 'btn cbi-button cbi-button-neutral',
							'click': handleConfig
						}, _('Configure Options'))
					])
				]),
				E('div', {}, [
					E('div', {}, [ labelUpdated, labelUpdating ]),
					E('div', {}, [
						E('label', { 'for': 'selectInterval' }, _('Auto update every:')),
						E('select', {
							'id': 'selectInterval',
							'change': clickToSelectInterval.bind(this, settings, labelUpdating)
							}, initOption({
								'-1': _('Disabled'),
								'3': _('3 seconds'),
								'5': _('5 seconds'),
								'10': _('10 seconds'),
								'30': _('30 seconds')
								}, settings.interval))
					])
				]),
				E('div', { 'id': 'progressbar_panel' }, [
					E('div', {}, [
						E('label', {},  _('Downstream:')),
						E('div', {
							'id': 'downstream',
							'class': 'cbi-progressbar',
							'title': '-'
							}, E('div')
						)
					]),
					E('div', {}, [
						E('label', {}, _('Upstream:')),
						E('div', {
							'id': 'upstream',
							'class': 'cbi-progressbar',
							'title': '-'
							}, E('div')
						)
					]),
				]),
				table
			])
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
