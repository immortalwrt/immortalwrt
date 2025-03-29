/* Copyright (C) 2022 ImmortalWrt.org */

'use strict';
'require form';
'require fs';
'require poll';
'require rpc';
'require uci';
'require view';

var getSystemFeatures = rpc.declare({
	object: 'luci.turboacc',
	method: 'getSystemFeatures',
	expect: { '': {} }
});

var getFastPathStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFastPathStat',
	expect: { '': {} }
});

var getFullConeStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getFullConeStat',
	expect: { '': {} }
});

var getTCPCCAStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getTCPCCAStat',
	expect: { '': {} }
});

var getMTKPPEStat = rpc.declare({
	object: 'luci.turboacc',
	method: 'getMTKPPEStat',
	expect: { '': {} }
});

function getServiceStatus() {
	return Promise.all([
		L.resolveDefault(getFastPathStat(), {}),
		L.resolveDefault(getFullConeStat(), {}),
		L.resolveDefault(getTCPCCAStat(), {})
	]);
}

function getMTKPPEStatus() {
	return Promise.all([
		L.resolveDefault(getMTKPPEStat(), {})
	]);
}

function progressbar(value, max, byte) {
	var vn = parseInt(value) || 0,
		mn = parseInt(max) || 100,
		fv = byte ? String.format('%1024.2mB', value) : value,
		fm = byte ? String.format('%1024.2mB', max) : max,
		pc = Math.floor((100 / mn) * vn);

	return E('div', {
		'class': 'cbi-progressbar',
		'title': '%s / %s (%d%%)'.format(fv, fm, pc)
	}, E('div', { 'style': 'width:%.2f%%'.format(pc) }));
}

function renderStatus(stats) {
	var spanTemp = '<em><span style="color:%s"><strong>%s</strong></span></em>';
	var renderHTML = [];
	for (var stat of stats)
		if (stat.type) {
			if (stat.type.includes(' / ')) {
				var types = stat.type.split(' / ');
				var inner = spanTemp.format('green', types[0]);
				for (var i of types.slice(1))
					inner += spanTemp.format('none', ' / ') + spanTemp.format('red', i);
				renderHTML.push(inner);
			} else
				renderHTML.push(spanTemp.format('green', stat.type));
		} else
			renderHTML.push(spanTemp.format('red', _('Disabled')));
	return renderHTML;
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('turboacc'),
			L.resolveDefault(getSystemFeatures(), {}),
			L.resolveDefault(getMTKPPEStat(), {})
		]);
	},

	render: function(data) {
		var m, s, o;
		var features = data[1];
		var ppe_stats = data[2];

		m = new form.Map('turboacc', _('TurboACC settings'),
			_('Open source flow offloading engine (fast path or hardware NAT).'));

		s = m.section(form.TypedSection);
		s.anonymous = true;
		s.render = function () {
			poll.add(function () {
				return L.resolveDefault(getServiceStatus()).then(function (res) {
					var stats = renderStatus(res);
					var tds = [ 'fastpath_state', 'fullcone_state', 'tcpcca_state' ];
					for (var i in tds) {
						var view = document.getElementById(tds[i]);
						view.innerHTML = stats[i];
					}
				});
			});

			var acc_status = E('table', { 'class': 'table', 'width': '100%', 'cellspacing': '10' }, [
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('FastPath Engine')),
					E('td', { 'id': 'fastpath_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('Full Cone NAT')),
					E('td', { 'id': 'fullcone_state' }, E('em', {}, _('Collecting data...')))
				]),
				E('tr', {}, [
					E('td', { 'width': '33%' }, _('TCP CCA')),
					E('td', { 'id': 'tcpcca_state' }, E('em', {}, _('Collecting data...')))
				])
			]);

			if (ppe_stats.hasOwnProperty('PPE_NUM')) {
				poll.add(function () {
					return L.resolveDefault(getMTKPPEStatus()).then(function (res) {
						var ppe_num = parseInt(res[0].PPE_NUM);
						for (var i=0; i<ppe_num; i++) {
							var ppe_bar = document.getElementById(`ppe${i}_entry`);
							ppe_bar.innerHTML = E('td', {},
							progressbar(res[0][`BIND_PPE${i}`], res[0][`ALL_PPE${i}`])).innerHTML;
						}
					});
				}, 3);

				var ppe_num = parseInt(ppe_stats.PPE_NUM);

				for (var i=0; i<ppe_num; i++) {
					acc_status.appendChild(E('tr', {}, [
						E('td', { 'width': '33%' }, `PPE${i} ` + _('Bind Entrys')),
						E('td', {'id': `ppe${i}_entry` },
						progressbar(ppe_stats[`BIND_PPE${i}`], ppe_stats[`ALL_PPE${i}`]))
					]));
				}
			}

			return E('fieldset', { 'class': 'cbi-section' }, [
				E('legend', {}, _('Acceleration Status')),
				acc_status
			]);
		}

		s = m.section(form.NamedSection, 'config', 'turboacc');

		o = s.option(form.ListValue, 'fastpath', _('Fastpath engine'),
			_('The offloading engine for routing/NAT.'));
		o.value('disabled', _('Disable'));
		if (features.hasFLOWOFFLOADING)
			o.value('flow_offloading', _('Flow offloading'));
		if (features.hasFASTCLASSIFIER)
			o.value('fast_classifier', _('Fast classifier'));
		if (features.hasSHORTCUTFECM)
			o.value('shortcut_fe_cm', _('SFE connection manager'));
		if (features.hasMEDIATEKHNAT)
			o.value('mediatek_hnat', _('MediaTek HNAT'));
		o.default = 'disabled';
		o.rmempty = false;
		o.onchange = function(ev, section_id, value) {
			var desc = ev.target.nextElementSibling;
			if (value === 'flow_offloading')
				desc.innerHTML = _('Software based offloading for routing/NAT.');
			else if (value === 'fast_classifier')
				desc.innerHTML = _('Fast classifier connection manager for the shortcut forwarding engine.');
			else if (value === 'shortcut_fe_cm')
				desc.innerHTML = _('Simple connection manager for the shortcut forwarding engine.');
			else if (value === 'mediatek_hnat')
				desc.innerHTML = _('MediaTek\'s open source hardware offloading engine.');
			else
				desc.innerHTML = _('The offloading engine for routing/NAT.');
		}

		o = s.option(form.Flag, 'fastpath_fo_hw', _('Hardware flow offloading'),
			_('Requires hardware NAT support. Implemented at least for mt7621.'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'flow_offloading');

		o = s.option(form.Flag, 'fastpath_fc_br', _('Bridge Acceleration'),
			_('Enable bridge acceleration (may be functional conflict with bridge-mode VPN server).'));
		o.default = o.disabled;
		o.rmempty = false;
		o.depends('fastpath', 'fast_classifier');

		if (features.hasIPV6) {
			o = s.option(form.Flag, 'fastpath_fc_ipv6', _('IPv6 acceleration'),
				_('Enable IPv6 Acceleration.'));
			o.default = o.disabled;
			o.rmempty = false;
			o.depends('fastpath', 'fast_classifier');
		}

		o = s.option(form.Flag, 'fastpath_mh_eth_hnat', _('Enable ethernet HNAT'),
			_('Enable hardware offloading for wired connections.'));
		o.default = o.enabled;
		o.rmempty = false;
		o.depends('fastpath', 'mediatek_hnat');

		o = s.option(form.Flag, 'fastpath_mh_eth_hnat_v6', _('Enable ethernet IPv6 HNAT'),
			_('Enable hardware offloading for wired IPv6 connections.'));
		o.default = o.enabled;
		o.rmempty = false;
		o.depends('fastpath_mh_eth_hnat', '1');
		
		o = s.option(form.Value, 'fastpath_mh_eth_hnat_ap', _('Enable AP Mode'),
			_('Fill in ip to enable AP Mode(reboot needed)'));
		o.optional = true;
		o.depends('fastpath_mh_eth_hnat', '1');
		
		o = s.option(form.Value, 'fastpath_mh_eth_hnat_bind_rate', _('HNAT bind rate threshold (pps)'),
			_('The smaller the threshold, the easier it is for the connection to be accelerated.'));
		o.optional = true;
		o.datatype = 'range(1,30)';
		o.placeholder = 30;
		o.depends('fastpath_mh_eth_hnat', '1');

		o = s.option(form.ListValue, 'fastpath_mh_eth_hnat_ppenum', _('Number of HNAT PPE'),
			_('Apply this setting after reboot.'));
		o.rmempty = false;
		o.value(1);
		o.value(2);
		o.default = 2;
		o.depends('fastpath_mh_eth_hnat', '1');

		o = s.option(form.ListValue, 'fullcone', _('Full cone NAT'),
			_('Full cone NAT (NAT1) can improve gaming performance effectively.'));
		o.value('0', _('Disable'))
		o.value('2', _('Boardcom_FULLCONE_NAT'));
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.ListValue, 'tcpcca', _('TCP CCA'),
			_('TCP congestion control algorithm.'));
		for (var i of features.hasTCPCCA.split(' ').sort())
			o.value(i);
		o.default = 'cubic';
		o.rmempty = false;
        
		return m.render();
	}
});
