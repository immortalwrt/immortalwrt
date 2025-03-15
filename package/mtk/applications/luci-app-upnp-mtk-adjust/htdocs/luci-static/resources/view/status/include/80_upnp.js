'use strict';
'require baseclass';
'require dom';
'require rpc';
'require uci';

var callUpnpGetStatus, callUpnpDeleteRule, handleDelRule;

callUpnpGetStatus = rpc.declare({
	object: 'luci.upnp',
	method: 'get_status',
	expect: {  }
});

callUpnpDeleteRule = rpc.declare({
	object: 'luci.upnp',
	method: 'delete_rule',
	params: [ 'token' ],
	expect: { result : "OK" },
});

handleDelRule = function(num, ev) {
	dom.parent(ev.currentTarget, '.tr').style.opacity = 0.5;
	ev.currentTarget.classList.add('spinning');
	ev.currentTarget.disabled = true;
	ev.currentTarget.blur();
	callUpnpDeleteRule(num);
};

return baseclass.extend({
	title: _('Active UPnP Redirects'),

	load: function() {
		return Promise.all([
			callUpnpGetStatus(),
		]);
	},

	render: function(data) {

		var table = E('table', { 'class': 'table', 'id': 'upnp_status_table' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				E('th', { 'class': 'th' }, _('Protocol')),
				E('th', { 'class': 'th' }, _('External Port')),
				E('th', { 'class': 'th' }, _('Client Address')),
				E('th', { 'class': 'th' }, _('Host')),
				E('th', { 'class': 'th' }, _('Client Port')),
				E('th', { 'class': 'th' }, _('Description')),
				E('th', { 'class': 'th cbi-section-actions' }, '')
			])
		]);

		var rules = Array.isArray(data[0].rules) ? data[0].rules : [];

		var rows = rules.map(function(rule) {
			return [
				rule.proto,
				rule.extport,
				rule.intaddr,
				rule.host_hint || _('Unknown'),
				rule.intport,
				rule.descr,
				E('button', {
					'class': 'btn cbi-button-remove',
					'click': L.bind(handleDelRule, this, rule.num)
				}, [ _('Delete') ])
			];
		});

		cbi_update_table(table, rows, E('em', _('There are no active redirects.')));

		return table;
	}
});
