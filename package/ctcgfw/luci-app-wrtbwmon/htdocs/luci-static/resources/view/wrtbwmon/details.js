'use strict';
'require fs';
'require rpc';
'require uci';
'require ui';
'require validation';

var cachedData = [];
var luciConfig = '/etc/luci-wrtbwmon.conf';
var hostNameFile = '/etc/wrtbwmon.user';

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

function $(tid) {
	return document.getElementById(tid);
}

function clickToResetDatabase(settings) {
	if (confirm(_('This will delete the database file. Are you sure?'))) {
		getPath().then(function(res) {
			var db = settings.protocol == 'ipv4' ? res : renameFile(res, '6');
			fs.exec('/bin/rm', [db]).then(function() {
				updateData($('traffic'), $('updated'), $('updating'), settings, true);
			});
		})
	}
}

function clickToSaveConfig(keylist, cstrs) {
	var data = {};

	for(var i = 0; i < keylist.length; i++) {
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

function clickToSelectInterval(settings, ev) {
	if (ev.target.value > 0) {
		settings.interval = parseInt(ev.target.value);
		if (!L.Request.poll.active()) L.Request.poll.start();
	}
	else {
		L.Request.poll.stop();
		setUpdateMessage($('updating'), -1);
	}
}

function clickToSelectProtocol(settings, ev) {
	settings.protocol = ev.target.value;
	updateData($('traffic'), $('updated'), $('updating'), settings, true);
}

function clickToShowMore(settings, ev) {
	var table = $('traffic');
	var t = table.querySelector('.tr.table-totals');
	settings.showMore = ev.target.checked

	if (t && t.firstElementChild)
		t.firstElementChild.textContent = _('TOTAL') + (settings.showMore ? '' : ': ' + (table.childElementCount - 2));

	table.querySelectorAll('.showMore').forEach(function(e) {
		e.classList.toggle('hide');
	});

	if (!settings.showMore && table.querySelector('.th.sorted').classList.contains('hide')) {
		table.querySelector('[id="thTotal"]').click();
	}
}

function createOption(args, val) {
	var cstr = args[0], title = args[1], desc = args.slice(-1), widget, frame;
	val = val != null ?  val : args[2];
	widget = args.length == 5 ? new cstr(val, args[3]) : new cstr(val, args[3], args[4]);

	frame = E('div', {'class': 'cbi-value'}, [
		E('label', {'class': 'cbi-value-title'}, title),
		E('div', {'class': 'cbi-value-field'}, E('div', {}, widget.render()))
	]);

	if (desc && desc != '')
		L.dom.append(frame.lastChild, E('div', { 'class': 'cbi-value-description' }, desc));

	return [widget, frame];
}

function displayTable(tb, settings) {
	var elm, elmID, col, sortedBy, flag, IPVer;
	var thID = ['thClient', 'thMAC', 'thDownload', 'thUpload', 'thTotalDown', 'thTotalUp', 'thTotal', 'thFirstSeen', 'thLastSeen', ''];

	elm = tb.querySelector('.th.sorted');
	elmID = elm ? elm.id : 'thTotal';
	sortedBy = elm && elm.classList.contains('ascent') ? 'asc' : 'desc';

	col = thID.indexOf(elmID);
	IPVer = col == 0 ? settings.protocol : null;
	flag = sortedBy == 'desc' ? 1 : -1;

	cachedData[0].sort(sortTable.bind(this, col, IPVer, flag));

	//console.time('show');
	updateTable(tb, cachedData, '<em>%s</em>'.format(_('Collecting data...')), settings);
	//console.timeEnd('show');
	progressbar('downstream', cachedData[1][0], settings.downstream, settings.useBits, settings.useMultiple);
	progressbar('upstream', cachedData[1][1], settings.upstream, settings.useBits, settings.useMultiple);
}

function formatBandWidth(bdw, useBits) {
	return bdw * Math.pow(1000, 2) / (useBits ? 1 : 8);
}

function formatSize(size, useBits, useMultiple) {
	var res = String.format('%%%s.2m%s'.format(useMultiple, (useBits ? 'bit' : 'B')), useBits ? size * 8 : size);
	return useMultiple == '1024' ? res.replace(/([KMGTPEZ])/, '$&i') : res;
}

function formatSpeed(speed, useBits, useMultiple) {
	return formatSize(speed, useBits, useMultiple) + '/s';
}

function formatDate(d) {
	var Y = d.getFullYear(), M = d.getMonth() + 1, D = d.getDate();
	var hh = d.getHours(), mm = d.getMinutes(), ss = d.getSeconds();
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

function getPath() {
	return uci.load('wrtbwmon').then(function() {
		var res = uci.get_first('wrtbwmon', 'wrtbwmon', 'path') || '/tmp/usage.db';
		uci.unload('wrtbwmon');
		return res;
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
			[ui.Select, _('Default Protocol'), 'ipv4', {'ipv4': _('ipv4'), 'ipv6': _('ipv6')}, {}, ''],
			[ui.Select, _('Default Refresh Interval'), '5', {'-1': _('Disabled'), '2': _('2 seconds'), '5': _('5 seconds'), '10': _('10 seconds'), '30': _('30 seconds')}, {sort: ['-1', '2', '5', '10', '30']}, ''],
			[ui.Checkbox, _('Default More Columns'), false, {value_enabled: true, value_disabled: false}, ''],
			[ui.Checkbox, _('Show Zeros'), true, {value_enabled: true, value_disabled: false}, ''],
			[ui.Checkbox, _('Transfer Speed in Bits'), false, {value_enabled: true, value_disabled: false}, ''],
			[ui.Select, _('Multiple of Unit'), '1000', {'1000': _('SI - 1000'), '1024': _('IEC - 1024')}, {}, ''],
			[ui.Checkbox, _('Use DSL Bandwidth'), false, {value_enabled: true, value_disabled: false}, ''],
			[ui.Textfield, _('Upstream Bandwidth'), '100', {datatype: 'ufloat'}, 'Mbps'],
			[ui.Textfield, _('Downstream Bandwidth'), '100', {datatype: 'ufloat'}, 'Mbps'],
			[ui.DynamicList, _('Hide MAC Addresses'), [], '', {datatype: 'macaddr'}, '']
		]; // [constructor, lable, default_value(, all_values), options, description]

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

function parseDatabase(values, hosts, showZero, hideMACs) {
	var valArr = [], totals = [0, 0, 0, 0, 0], valToRows, row;

	valToRows = values.replace(/(^\s*)|(\s*$)/g, '').split(/\r?\n|\r/g);
	valToRows.shift();

	for (var i = 0; i < valToRows.length; i++) {
		row = valToRows[i].split(',');
		if ((!showZero && row[7] == 0) || hideMACs.indexOf(row[0]) >= 0) continue;

		for (var j = 0; j < totals.length; j++) {
			totals[j] += parseInt(row[3 + j]);
		}

		row = Array.prototype.concat(row.slice(0, 2).reverse(), row.slice(3), row.slice(0, 1));
		if (row[1] in hosts && hosts[row[1]] != '-') {
			row[9] = hosts[row[1]];
		}
		valArr.push(row);
	}

	return [valArr, totals];
}

function parseDefaultSettings(file) {
	var keylist = ['protocol', 'interval', 'showMore', 'showZero', 'useBits', 'useMultiple', 'useDSL', 'upstream', 'downstream', 'hideMACs'];
	var valuelist = ['ipv4', '5', false, true, false, '1000', false, '100', '100', []];

	return fs.read(file).then(function(json) {
		var settings;
		try {
			settings = JSON.parse(json);
		}
		catch(err) {
			settings = {};
		}

		for (var i = 0; i < keylist.length; i++) {
			if(!(keylist[i] in settings))
				settings[keylist[i]] = valuelist[i];
		}

		if (settings.useDSL) {
			return getDSLBandwidth().then(function(dsl) {
				settings.upstream = dsl.upstream || settings.upstream;
				settings.downstream = dsl.downstream || settings.downstream;
				return settings;
			});
		}
		else {
			return settings;
		}
	});
}

function progressbar(query, v, m, useBits, useMultiple) {
	var pg = $(query),
	    vn = v || 0,
	    mn = formatBandWidth(m, useBits) || 100,
	    fv = formatSpeed(v, useBits, useMultiple),
	    pc = '%.2f'.format((100 / mn) * vn),
	    wt = Math.floor(pc > 100 ? 100 : pc),
	    bgc = (pc >= 95 ? 'red' : (pc >= 80 ? 'darkorange' : (pc >= 60 ? 'yellow' : 'lime'))),
	    tc = (pc >= 80 ? 'white' : '#404040');
	if (pg) {
		pg.firstElementChild.style.width = wt + '%';
		pg.firstElementChild.style.background = bgc;
		pg.style.color = tc;
		pg.setAttribute('title', '%s (%f%%)'.format(fv, pc));
	}
}

function registerTableEventHandlers(settings, table) {
	var indicators = $('xhr_poll_status') || $('indicators').getElementsByTagName('span')[0];
	indicators.addEventListener('click', function() {
		$('selectInterval').value = L.Request.poll.active() ? settings.interval : -1;
	});

	table.querySelectorAll('.th').forEach(function(e) {
		if (e) {
			e.addEventListener('click', function (ev) {
				setSortedColumn(ev.target);
				displayTable(table, settings);
			});
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
		return fs.read(hostNameFile).then(function(rawStr) {
			var hostNames = [], arr = rawStr.split(/\r?\n|\r/g), row;
			for (var i = 0; i < arr.length; i++) {
				row = arr[i].split(',');
				if (row.length == 2 && row[0])
					hostNames.push({ macaddr: row[0], hostname: row[1] });
			}
			return hostNames;
		})
	}).catch(function() { return []; });
}

function resolveHostNameByMACAddr() {
	return Promise.all([
		resolveCustomizedHostName(),
		callLuciDHCPLeases()
	]).then(function(res) {
		var leaseNames, macaddr, hostNames = {};
		leaseNames = [
			res[0],
			Array.isArray(res[1].dhcp_leases) ? res[1].dhcp_leases : [],
			Array.isArray(res[1].dhcp6_leases) ? res[1].dhcp6_leases : []
		];
		for (var i = 0; i < leaseNames.length; i++) {
			for (var j = 0; j < leaseNames[i].length; j++) {
				if (leaseNames[i][j].macaddr) {
					macaddr = leaseNames[i][j].macaddr.toLowerCase();
					if (!(macaddr in hostNames) || hostNames[macaddr] == '-') {
						hostNames[macaddr] = leaseNames[i][j].hostname || '-';
					}
				}
			}
		}
		return hostNames;
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
	e.innerHTML = sec < 0 ? '' : ' ' + _('Updating again in %s second(s).').format('<b>' + sec + '</b>');
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

	if(Array.isArray(a) && Array.isArray(b)) {
		for(var i = 0; i < a.length; i++) {
			if (a[i] != b[i]) {
				return (b[i] - a[i]) * flag;
			}
		}
		return 0;
	}

	return a == b ? 0 : (a < b ? 1 : -1) * flag;
}

function updateData(table, updated, updating, settings, once) {
	if (!(L.Poll.tick % settings.interval) || once) {
		getPath().then(function(res) {
			var params = settings.protocol == 'ipv4' ? '-4' : '-6';
			fs.exec('/usr/sbin/wrtbwmon', [params, '-f', res]);
			return params == '-4' ? res : renameFile(res, '6');
		}).then(function(data) {
			Promise.all([
				fs.exec('/bin/cat', [ data ]),
				resolveHostNameByMACAddr()
			]).then(function(res) {
				//console.time('start');
				cachedData = parseDatabase(res[0].stdout || '', res[1], settings.showZero, settings.hideMACs);
				displayTable(table, settings);
				updated.innerHTML = _('Last updated at %s.').format(formatDate(new Date(document.lastModified)));
				//console.timeEnd('start');
			});
		});
	}
	updatePerSec(updating, settings.interval);
}

function updatePerSec(e, interval) {
	var tick = L.Poll.tick;
	var sec = tick % interval ? interval - tick % interval : 0;

	setUpdateMessage(e, sec);
	if(sec == 0) {
		setTimeout(setUpdateMessage.bind(this, e, interval), 100);
	}
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
				newNode = document.createElement('div');
				childTD = document.createElement('div');
				for (var j = 0; j < tbTitle.children.length; j++) {
					childTD.className = 'td' + ('178'.indexOf(j) != -1 ? ' showMore' + (settings.showMore ? '' : ' hide') : '');
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
	if(formData.length == 0) {
		newNode = document.createElement('div');
		newNode.className = 'tr placeholder';
		childTD = document.createElement('div');
		childTD.className = 'td';
		childTD.innerHTML = placeholder;
		newNode.appendChild(childTD);
	}
	else{
		newNode = fragment.firstChild.cloneNode(true);
		newNode.className = 'tr table-totals';

		newNode.children[0].textContent = _('TOTAL') + (settings.showMore ? '' : ': ' + formData.length);
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

return L.view.extend({
	load: function() {
		return Promise.all([
			parseDefaultSettings(luciConfig),
			loadCss(L.resource('view/wrtbwmon/wrtbwmon.css'))
		]);
	},

	render: function(data) {
		var settings = data[0];
		var node = E('div', { 'class': 'cbi-map' }, [
			E('h2', {}, _('Usage - Details')),
			E('div', { 'class': 'cbi-section' }, [
				E('div', { 'id': 'control_panel' }, [
					E('div', {}, [
						E('label', {}, _('Protocol:')),
						E('select', {
							'id': 'selectProtocol',
							'change': clickToSelectProtocol.bind(this, settings)
							}, [
							E('option', { 'value': 'ipv4' }, 'ipv4'),
							E('option', { 'value': 'ipv6' }, 'ipv6')
						])
					]),
					E('div', {}, [
						E('label', { 'for': 'showMore' }, _('Show More Columns:')),
						E('input', {
							'id': 'showMore',
							'type': 'checkbox',
							'click': clickToShowMore.bind(this, settings)
						}),
					]),
					E('div', {}, [
						E('button', {
							'class': 'btn cbi-button cbi-button-reset important',
							'id': 'resetDatabase',
							'click': clickToResetDatabase.bind(this, settings)
						}, _('Reset Database')),
						' ',
						E('button', {
							'class': 'btn cbi-button cbi-button-neutral',
							'click': handleConfig
						}, _('Configure Options'))
					])
				]),
				E('div', {}, [
					E('div', {}, [
						E('div', { 'id': 'updated' }),
						E('div', { 'id': 'updating' })
					]),
					E('div', {}, [
						E('label', { 'for': 'selectInterval' }, _('Auto update every:')),
						E('select', {
							'id': 'selectInterval',
							'change': clickToSelectInterval.bind(this, settings)
							}, [
							E('option', { 'value': '-1' }, _('Disabled')),
							E('option', { 'value': '2' }, _('2 seconds')),
							E('option', { 'value': '5' }, _('5 seconds')),
							E('option', { 'value': '10' }, _('10 seconds')),
							E('option', { 'value': '30' }, _('30 seconds'))
						])
					])
				]),
				E('div', { 'id': 'progressbar_panel', 'class': 'table' }, [
					E('div', { 'class': 'tr' }, [
						E('div', { 'class': 'td' }, E('div', {}, _('Downstream:'))),
						E('div', { 'class': 'td' }, E('div', {
							'id': 'downstream',
							'class': 'cbi-progressbar',
							'title': '-'
							}, E('div')
						))
					]),
					E('div', { 'class': 'tr' }, [
						E('div', { 'class': 'td' }, E('div', {}, _('Upstream:'))),
						E('div', { 'class': 'td' }, E('div', {
							'id': 'upstream',
							'class': 'cbi-progressbar',
							'title': '-'
							}, E('div')
						))
					]),
				]),
				E('div', { 'class': 'table', 'id': 'traffic' }, [
					E('div', { 'class': 'tr table-titles' }, [
						E('div', { 'class': 'th', 'id': 'thClient' }, _('Clients')),
						E('div', { 'class': 'th showMore hide', 'id': 'thMAC' }, _('MAC')),
						E('div', { 'class': 'th', 'id': 'thDownload' }, _('Download')),
						E('div', { 'class': 'th', 'id': 'thUpload' }, _('Upload')),
						E('div', { 'class': 'th', 'id': 'thTotalDown' }, _('Total Down')),
						E('div', { 'class': 'th', 'id': 'thTotalUp' }, _('Total Up')),
						E('div', { 'class': 'th sorted', 'id': 'thTotal' }, _('Total')),
						E('div', { 'class': 'th showMore hide', 'id': 'thFirstSeen' }, _('First Seen')),
						E('div', { 'class': 'th showMore hide', 'id': 'thLastSeen' }, _('Last Seen'))
					]),
					E('div', {'class': 'tr placeholder'}, [
						E('div', { 'class': 'td' }, E('em', {}, _('Collecting data...')))
					])
				])
			])
		]);

		return Promise.all([
			node.querySelector('[id="traffic"]'),
			node.querySelector('[id="updated"]'),
			node.querySelector('[id="updating"]'),
			node.querySelector('[id="selectInterval"]').value = settings.interval,
			node.querySelector('[id="selectProtocol"]').value = settings.protocol,
			node.querySelector('[id="showMore"]').checked = settings.showMore,
			node.querySelectorAll('.showMore').forEach(function(e) { settings.showMore ? e.classList.remove('hide') : e.classList.add('hide'); })
		])
		.then(function(data) {
			L.Poll.add(updateData.bind(this, data[0], data[1], data[2], settings, false), 1);
			return data[0];
		})
		.then(registerTableEventHandlers.bind(this, settings))
		.then(function() { return node; });
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
